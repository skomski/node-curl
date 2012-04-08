// Copyright 2012 Karl Skomski MIT

#include "helpers.h"

namespace helpers {
  using v8::Object;
  using v8::Handle;
  using v8::Local;
  using v8::Persistent;
  using v8::Value;
  using v8::HandleScope;
  using v8::FunctionTemplate;
  using v8::String;
  using v8::Array;
  using v8::Function;
  using v8::TryCatch;
  using v8::Context;
  using v8::Arguments;
  using v8::Integer;
  using v8::Exception;
  using v8::Undefined;
  using v8::External;

  void ThrowError(const char* message) {
    HandleScope scope;
    ThrowException(Exception::Error(String::New(message)));
  }

  void ThrowCurlError(CURLcode code) {
    HandleScope scope;
    ThrowError(curl_easy_strerror(code));
  }

  void ThrowCurlMultiError(CURLMcode code) {
    HandleScope scope;
    ThrowError(curl_multi_strerror(code));
  }

  void Emit(Handle<Object> source, const char *event, Handle<Value> object) {
    HandleScope scope;
    Handle<Value> argv[2] = {
      String::NewSymbol(event),
      object
    };

    MakeCallback(source, "emit", 2, argv);
  }

  void EmitError(Handle<Object> source, const char* error_message) {
    HandleScope scope;
    Emit(source, "error", Exception::Error(String::New(error_message)));
  }

  void EmitCurlError(Handle<Object> source, CURLcode code) {
    HandleScope scope;
    EmitError(source, curl_easy_strerror(code));
  }

  void EmitCurlMultiError(Handle<Object> source, CURLMcode code) {
    HandleScope scope;
    EmitError(source, curl_multi_strerror(code));
  }

  void MakeCallback(Handle<Object> object,
                    const char *method,
                    int argc,
                    Handle<Value> argv[]) {
    HandleScope scope;
    Local<Value> callback_v = object->Get(String::NewSymbol(method));
    Local<Function> callback = Local<Function>::Cast(callback_v);

    TryCatch try_catch;

    callback->Call(object, argc, argv);

    if (try_catch.HasCaught()) {
      node::FatalException(try_catch);
    }
  }
}
