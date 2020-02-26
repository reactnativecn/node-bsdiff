/**
 * Created by tdzl2003 on 2/28/16.
 */
#include <nan.h>
#include <node.h>
#include <node_buffer.h>

#include <cstdlib>

using namespace std;

extern "C" {
#include "bsdiff/bsdiff.h"
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
      // opaque->cb->Call(Nan::GetCurrentContext()->Global(), Null(opaque->isolate), 1, argv);


      Nan::MakeCallback(Nan::GetCurrentContext()->Global(), opaque->cb, 1, argv);

      return 0;
    }

    void diff(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        HandleScope scope(isolate);

        if (!node::Buffer::HasInstance(args[0]) || !node::Buffer::HasInstance(args[1]) || !args[2]->IsFunction()) {
          Nan::ThrowError("Invalid arguments.");
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
          Nan::ThrowError("Create bsdiff failed.");
        }

//        args.GetReturnValue().Set(returnObj);
//        args.GetReturnValue().Set(String::NewFromUtf8(isolate, bufferData, String::kNormalString, bufferLength));
    }
    void init(Local<Object> exports)
    {
        Isolate* isolate = exports->GetIsolate();
        HandleScope scope(isolate);

        NODE_SET_METHOD(exports, "diff", diff);
    }

    NODE_MODULE(bsdiff, init)

} // namespace bsdiff
