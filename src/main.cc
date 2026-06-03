/**
 * node-bsdiff - Node-API implementation
 * Refactored from NAN for Bun compatibility.
 */
#include <napi.h>

#include <climits>
#include <cstdlib>
#include <cstring>
#include <vector>

extern "C" {
#include "bsdiff/bsdiff.h"
#include "bzlib/bzlib.h"
}

namespace bsdiffNode
{
    inline bool getBufferData(const Napi::Value& arg, const uint8_t** data, size_t* length) {
        if (arg.IsBuffer()) {
            Napi::Buffer<uint8_t> buf = arg.As<Napi::Buffer<uint8_t>>();
            *data = buf.Data();
            *length = buf.Length();
            return true;
        }

        if (arg.IsTypedArray()) {
            Napi::TypedArray typedArray = arg.As<Napi::TypedArray>();
            Napi::ArrayBuffer arrayBuffer = typedArray.ArrayBuffer();
            *data = static_cast<const uint8_t*>(arrayBuffer.Data()) + typedArray.ByteOffset();
            *length = typedArray.ByteLength();
            return true;
        }

        if (arg.IsArrayBuffer()) {
            Napi::ArrayBuffer arrayBuffer = arg.As<Napi::ArrayBuffer>();
            *data = static_cast<const uint8_t*>(arrayBuffer.Data());
            *length = arrayBuffer.ByteLength();
            return true;
        }

        return false;
    }

    inline Napi::Buffer<uint8_t> bufferFromVector(Napi::Env env, std::vector<uint8_t>&& data) {
        if (data.empty()) {
            return Napi::Buffer<uint8_t>::New(env, 0);
        }

        auto* vec = new std::vector<uint8_t>(std::move(data));
        return Napi::Buffer<uint8_t>::New(
            env,
            vec->data(),
            vec->size(),
            [](Napi::Env /*env*/, uint8_t* /*data*/, std::vector<uint8_t>* vecPtr) {
                delete vecPtr;
            },
            vec
        );
    }

    inline bool hasPendingException(Napi::Env env) {
        bool pending = false;
        napi_is_exception_pending(env, &pending);
        return pending;
    }

    struct DiffStreamOpaque {
        Napi::Env env;
        Napi::Function* cb;
        std::vector<uint8_t>* output;
    };

    static int callback_write(struct bsdiff_stream* stream, const void* buffer, int size)
    {
        DiffStreamOpaque* opaque = static_cast<DiffStreamOpaque*>(stream->opaque);
        if (size < 0) {
            return -1;
        }

        const uint8_t* data = static_cast<const uint8_t*>(buffer);
        size_t length = static_cast<size_t>(size);

        if (opaque->cb != nullptr) {
            try {
                Napi::Buffer<uint8_t> output = Napi::Buffer<uint8_t>::Copy(opaque->env, data, length);
                opaque->cb->Call(opaque->env.Global(), { output });
            } catch (Napi::Error& error) {
                error.ThrowAsJavaScriptException();
                return -1;
            }
        } else if (opaque->output != nullptr && length > 0) {
            size_t offset = opaque->output->size();
            opaque->output->resize(offset + length);
            std::memcpy(opaque->output->data() + offset, data, length);
        }

        return 0;
    }

    inline void writeOutput(Napi::Env env,
                            Napi::Function* cb,
                            std::vector<uint8_t>* output,
                            const uint8_t* data,
                            size_t length) {
        if (length == 0) {
            return;
        }

        if (cb != nullptr) {
            Napi::Buffer<uint8_t> chunk = Napi::Buffer<uint8_t>::Copy(env, data, length);
            cb->Call(env.Global(), { chunk });
            return;
        }

        if (output != nullptr) {
            size_t offset = output->size();
            output->resize(offset + length);
            std::memcpy(output->data() + offset, data, length);
        }
    }

    Napi::Value diff(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();

        const uint8_t* oldData = nullptr;
        size_t oldLength = 0;
        const uint8_t* newData = nullptr;
        size_t newLength = 0;

        if (info.Length() < 2 ||
            !getBufferData(info[0], &oldData, &oldLength) ||
            !getBufferData(info[1], &newData, &newLength)) {
            Napi::TypeError::New(env, "Invalid arguments: expected Buffer, TypedArray, or ArrayBuffer.")
                .ThrowAsJavaScriptException();
            return env.Undefined();
        }

        Napi::Function cb;
        Napi::Function* cbPtr = nullptr;
        std::vector<uint8_t> result;

        if (info.Length() > 2 && info[2].IsFunction()) {
            cb = info[2].As<Napi::Function>();
            cbPtr = &cb;
        }

        DiffStreamOpaque streamOpaque { env, cbPtr, cbPtr == nullptr ? &result : nullptr };
        bsdiff_stream stream;
        stream.malloc = std::malloc;
        stream.free = std::free;
        stream.write = callback_write;
        stream.opaque = &streamOpaque;

        if (bsdiff(oldData, oldLength, newData, newLength, &stream)) {
            if (!hasPendingException(env)) {
                Napi::Error::New(env, "Create bsdiff failed.").ThrowAsJavaScriptException();
            }
            return env.Undefined();
        }

        if (cbPtr != nullptr) {
            return env.Undefined();
        }

        return bufferFromVector(env, std::move(result));
    }

    Napi::Value compress(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();

        const uint8_t* data = nullptr;
        size_t length = 0;
        if (info.Length() < 1 || !getBufferData(info[0], &data, &length)) {
            Napi::TypeError::New(env, "Invalid arguments: expected Buffer, TypedArray, or ArrayBuffer.")
                .ThrowAsJavaScriptException();
            return env.Undefined();
        }

        if (length > static_cast<size_t>(UINT_MAX)) {
            Napi::RangeError::New(env, "Input is too large for one-shot bzip2 compression.")
                .ThrowAsJavaScriptException();
            return env.Undefined();
        }

        Napi::Function cb;
        Napi::Function* cbPtr = nullptr;
        std::vector<uint8_t> result;

        if (info.Length() > 1 && info[1].IsFunction()) {
            cb = info[1].As<Napi::Function>();
            cbPtr = &cb;
        }

        bz_stream stream;
        std::memset(&stream, 0, sizeof(stream));

        int ret = BZ2_bzCompressInit(&stream, 9, 0, 0);
        if (ret != BZ_OK) {
            Napi::Error::New(env, "Compress error.").ThrowAsJavaScriptException();
            return env.Undefined();
        }

        stream.next_in = reinterpret_cast<char*>(const_cast<uint8_t*>(data));
        stream.avail_in = static_cast<unsigned int>(length);

        uint8_t outputBuffer[4096];

        do {
            stream.next_out = reinterpret_cast<char*>(outputBuffer);
            stream.avail_out = sizeof(outputBuffer);

            ret = BZ2_bzCompress(&stream, BZ_FINISH);
            if (ret != BZ_FINISH_OK && ret != BZ_STREAM_END) {
                BZ2_bzCompressEnd(&stream);
                Napi::Error::New(env, "Compress error.").ThrowAsJavaScriptException();
                return env.Undefined();
            }

            size_t have = sizeof(outputBuffer) - stream.avail_out;
            try {
                writeOutput(env, cbPtr, cbPtr == nullptr ? &result : nullptr, outputBuffer, have);
            } catch (Napi::Error& error) {
                BZ2_bzCompressEnd(&stream);
                error.ThrowAsJavaScriptException();
                return env.Undefined();
            }
        } while (ret != BZ_STREAM_END);

        BZ2_bzCompressEnd(&stream);

        if (cbPtr != nullptr) {
            return env.Undefined();
        }

        return bufferFromVector(env, std::move(result));
    }

    Napi::Object Init(Napi::Env env, Napi::Object exports) {
        exports.Set(Napi::String::New(env, "diff"), Napi::Function::New(env, diff));
        exports.Set(Napi::String::New(env, "compress"), Napi::Function::New(env, compress));
        return exports;
    }

    NODE_API_MODULE(bsdiff, Init)

} // namespace bsdiff
