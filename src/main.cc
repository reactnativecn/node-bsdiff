/**
 * Created by tdzl2003 on 2/28/16.
 */

#include <node.h>
#include <node_buffer.h>

#include <memory.h>
#include <stdlib.h>

extern "C" {
#include "bsdiff/bsdiff.h"
#include "bzlib/bzlib.h"
}

namespace bsdiffNode
{
    using v8::FunctionCallbackInfo;
    using v8::HandleScope;
    using v8::Isolate;
    using v8::Local;
    using v8::Object;
    using v8::String;
    using v8::Value;
    using v8::Function;
    using v8::MaybeLocal;
    using v8::Null;
    using v8::Boolean;
    using v8::Exception;

    struct DiffStreamOpaque {
        Isolate* isolate;
        Local<Function> cb;
    };

    static int callback_write(struct bsdiff_stream* stream, const void* buffer, int size)
    {
      DiffStreamOpaque* opaque = (DiffStreamOpaque*)stream->opaque;

      Local<Object> returnObj = node::Buffer::Copy(opaque->isolate, (const char*)buffer, size).ToLocalChecked();

      Local<Value> argv[1] = { returnObj };
      opaque->cb->Call(Null(opaque->isolate), 1, argv);

      return 0;
    }

    void diff(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        HandleScope scope(isolate);

        if (!node::Buffer::HasInstance(args[0]) || !node::Buffer::HasInstance(args[1]) || !args[2]->IsFunction()) {
          isolate->ThrowException(Exception::TypeError(
                            String::NewFromUtf8(isolate, "Invalid arguments.")));
        }

        char*         oldData   = node::Buffer::Data(args[0]);
        size_t        oldLength = node::Buffer::Length(args[0]);

        char*         newData   = node::Buffer::Data(args[1]);
        size_t        newLength = node::Buffer::Length(args[1]);

        DiffStreamOpaque streamOpaque;

        streamOpaque.isolate = isolate;
        streamOpaque.cb = Local<Function>::Cast(args[2]);

        bsdiff_stream stream;

        stream.malloc = malloc;
        stream.free = free;
        stream.write = callback_write;
        stream.opaque = &streamOpaque;

        if (bsdiff((const uint8_t*)oldData, oldLength, (const uint8_t*)newData, newLength, &stream)) {
            isolate->ThrowException(Exception::Error(
                    String::NewFromUtf8(isolate, "Create bsdiff failed.")));
        }

//        args.GetReturnValue().Set(returnObj);
//        args.GetReturnValue().Set(String::NewFromUtf8(isolate, bufferData, String::kNormalString, bufferLength));
    }

    void compress(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        HandleScope scope(isolate);

        if (!node::Buffer::HasInstance(args[0])) {
          isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Invalid arguments.")));
        }

        char*         Data   = node::Buffer::Data(args[0]);
        size_t        Length = node::Buffer::Length(args[0]);

        Local<Function> cb = Local<Function>::Cast(args[1]);

        bz_stream stream;
        stream.bzalloc = NULL;
        stream.bzfree = NULL;

        int ret = BZ2_bzCompressInit ( &stream, 9, 0, 0 );
        if (ret != BZ_OK) {
            isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Compress error")));
        }

//        Local<Object> obj = node::Buffer::New(isolate, 4096).ToLocalChecked();

        char* bufStart = (char*)malloc(4096);

        stream.next_in = Data;
        stream.avail_in = Length;
        stream.next_out = bufStart;
        stream.avail_out = 4096;

        ret = BZ2_bzCompress ( &stream, BZ_FINISH );

        while (ret == BZ_FINISH_OK) {
            Local<Object> obj = node::Buffer::Copy(isolate, bufStart, stream.next_out - bufStart).ToLocalChecked();
            Local<Value> argv[1] = { obj };
            cb->Call(Null(isolate), 1, argv);

            stream.next_out = bufStart;
            stream.avail_out = 4096;
            ret = BZ2_bzCompress( &stream, BZ_FINISH);
        }

        if (ret != BZ_STREAM_END) {
            free(bufStart);
            isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Compress error")));
        }

        Local<Object> obj = node::Buffer::Copy(isolate, bufStart, stream.next_out - bufStart).ToLocalChecked();
        Local<Value> argv[1] = { obj };
        cb->Call(Null(isolate), 1, argv);

        BZ2_bzCompressEnd(&stream);
        free(bufStart);
    }

    void init(Local<Object> exports)
    {
        Isolate* isolate = exports->GetIsolate();
        HandleScope scope(isolate);

        NODE_SET_METHOD(exports, "diff", diff);
        NODE_SET_METHOD(exports, "compress", compress);
    }

    NODE_MODULE(bsdiff, init)

} // namespace bsdiff
