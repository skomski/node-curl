// Copyright 2012 Karl Skomski MIT

#include "curl_easy_wrapper.h"
#include "node_buffer.h"

namespace nodecurl {
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

  CurlEasyWrapper::CurlEasyWrapper() : easy_handle_(curl_easy_init()) {
    if (easy_handle_ == NULL) {
      helpers::ThrowError("EasyHandleInit failed!");
      return;
    }

    curl_easy_setopt(easy_handle_, CURLOPT_WRITEFUNCTION, WriteFunction);
    curl_easy_setopt(easy_handle_, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(easy_handle_, CURLOPT_PRIVATE, &this->handle_);
  }

  CurlEasyWrapper::~CurlEasyWrapper() {
    curl_easy_cleanup(easy_handle_);
  }

  void CurlEasyWrapper::Initialize(Handle<Object> target) {
    Local<FunctionTemplate> wrapper_template = FunctionTemplate::New(New);
    wrapper_template->SetClassName(String::NewSymbol("CurlEasyWrapper"));
    wrapper_template->InstanceTemplate()->SetInternalFieldCount(1);

    wrapper_template->PrototypeTemplate()->Set(String::NewSymbol("_setOption"),
        FunctionTemplate::New(SetOption_)->GetFunction());

    wrapper_template->PrototypeTemplate()->Set(String::NewSymbol("resume"),
        FunctionTemplate::New(Resume)->GetFunction());

    wrapper_template->PrototypeTemplate()->Set(String::NewSymbol("pause"),
        FunctionTemplate::New(Pause)->GetFunction());

    Persistent<Function> constructor = Persistent<Function>::New(
        wrapper_template->GetFunction());
    target->Set(String::NewSymbol("CurlEasyWrapper"), constructor);
  }

  Handle<Value> CurlEasyWrapper::New(const Arguments& args) {
    HandleScope scope;

    CurlEasyWrapper* wrapper = new CurlEasyWrapper();
    wrapper->Wrap(args.This());

    return scope.Close(args.This());
  }

  size_t CurlEasyWrapper::WriteFunction(char *ptr, size_t size, size_t nmemb,
      void *userdata) {
    CurlEasyWrapper *wrapper = (CurlEasyWrapper*)userdata;
    return wrapper->OnData(ptr, size * nmemb);
  }

  size_t CurlEasyWrapper::OnData(char *data, size_t size) {
    assert(size > 0);

    node::Buffer *buffer = node::Buffer::New(data, size);
    helpers::Emit(this->handle_, "data", buffer->handle_);
    return size;
  }

  Handle<Value> CurlEasyWrapper::SetOption_(const Arguments& args) {
    HandleScope scope;
    CurlEasyWrapper* wrapper = ObjectWrap::Unwrap<CurlEasyWrapper>(args.This());

    const CURLoption option = (CURLoption) args[0]->Int32Value();

    const CURLcode status = curl_easy_setopt(wrapper->easy_handle_, option, *String::Utf8Value(args[1]));

    if (status != CURLE_OK) {
      helpers::EmitCurlError(wrapper->handle_, status);
    }

    return scope.Close(Undefined());
  }

  Handle<Value> CurlEasyWrapper::Pause(const Arguments& args) {
    HandleScope scope;
    CurlEasyWrapper* wrapper = ObjectWrap::Unwrap<CurlEasyWrapper>(args.This());

    const CURLcode status = curl_easy_pause(wrapper->easy_handle_, CURLPAUSE_ALL);

    if (status != CURLE_OK) {
      helpers::EmitCurlError(wrapper->handle_, status);
    }

    return scope.Close(Undefined());
  }

Handle<Value> CurlEasyWrapper::Resume(const Arguments& args) {
    HandleScope scope;
    CurlEasyWrapper* wrapper = ObjectWrap::Unwrap<CurlEasyWrapper>(args.This());

    const CURLcode status = curl_easy_pause(wrapper->easy_handle_, CURLPAUSE_CONT);

    if (status != CURLE_OK) {
      helpers::EmitCurlError(wrapper->handle_, status);
    }

    return scope.Close(Undefined());
  }
}