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
    std::vector<curl_slist*>::const_iterator i;
    for(i = option_lists_.begin(); i != option_lists_.end(); ++i) {
      assert(*i != NULL);
      curl_slist_free_all(*i);
    }
  }

  void CurlEasyWrapper::Initialize(Handle<Object> target) {
    Local<FunctionTemplate> wrapper_template = FunctionTemplate::New(New);
    wrapper_template->SetClassName(String::NewSymbol("CurlEasyWrapper"));
    wrapper_template->InstanceTemplate()->SetInternalFieldCount(1);

    wrapper_template->PrototypeTemplate()->Set(String::NewSymbol("_setNumberOption"),
        FunctionTemplate::New(SetNumberOption_)->GetFunction());

    wrapper_template->PrototypeTemplate()->Set(String::NewSymbol("_setStringOption"),
        FunctionTemplate::New(SetStringOption_)->GetFunction());

    wrapper_template->PrototypeTemplate()->Set(String::NewSymbol("_setListOption"),
        FunctionTemplate::New(SetListOption_)->GetFunction());

    wrapper_template->PrototypeTemplate()->Set(String::NewSymbol("resume"),
        FunctionTemplate::New(Resume)->GetFunction());

    wrapper_template->PrototypeTemplate()->Set(String::NewSymbol("pause"),
        FunctionTemplate::New(Pause)->GetFunction());

    wrapper_template->PrototypeTemplate()->Set(String::NewSymbol("close"),
        FunctionTemplate::New(Close)->GetFunction());

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

  Handle<Value> CurlEasyWrapper::SetListOption_(const Arguments& args) {
    HandleScope scope;
    CurlEasyWrapper* wrapper = ObjectWrap::Unwrap<CurlEasyWrapper>(args.This());

    if (args.Length() < 2) {
      helpers::ThrowError("Need 2 arguments");
      return scope.Close(Undefined());
    }

    if (!args[0]->IsNumber() || !args[1]->IsArray()) {
      helpers::ThrowError("Need number[0] and array[1]");
      return scope.Close(Undefined());
    }

    const CURLoption option = (CURLoption) args[0]->Int32Value();
    curl_slist *list = NULL;

    Local<Array> entries = Local<Array>::Cast(args[1]);
    int entries_length = entries->Length();

    for (int i = 0; i < entries_length; i++) {
      String::Utf8Value entry(entries->Get(i));
      list = curl_slist_append(list, *entry);
    }

    const CURLcode status = curl_easy_setopt(
        wrapper->easy_handle_, option, list);

    if (status != CURLE_OK) {
      curl_slist_free_all(list);
      helpers::EmitCurlError(wrapper->handle_, status);
    }

    wrapper->option_lists_.push_back(list);

    return scope.Close(Undefined());
  }

  Handle<Value> CurlEasyWrapper::SetNumberOption_(const Arguments& args) {
    HandleScope scope;
    CurlEasyWrapper* wrapper = ObjectWrap::Unwrap<CurlEasyWrapper>(args.This());

    if (args.Length() < 2) {
      helpers::ThrowError("Need 2 arguments");
      return scope.Close(Undefined());
    }

    if (!args[0]->IsNumber() || !args[1]->IsNumber()) {
      helpers::ThrowError("Need number[0] and number[1]");
      return scope.Close(Undefined());
    }

    const CURLoption option = (CURLoption) args[0]->Int32Value();

    const CURLcode status = curl_easy_setopt(
        wrapper->easy_handle_, option, args[1]->Int32Value());

    if (status != CURLE_OK) {
      helpers::EmitCurlError(wrapper->handle_, status);
    }

    return scope.Close(Undefined());
  }

  Handle<Value> CurlEasyWrapper::SetStringOption_(const Arguments& args) {
    HandleScope scope;
    CurlEasyWrapper* wrapper = ObjectWrap::Unwrap<CurlEasyWrapper>(args.This());

    if (args.Length() < 2) {
      helpers::ThrowError("Need 2 arguments");
      return scope.Close(Undefined());
    }

    if (!args[0]->IsNumber() || !args[1]->IsString()) {
      helpers::ThrowError("Need number[0] and string[1]");
      return scope.Close(Undefined());
    }

    const CURLoption option = (CURLoption) args[0]->Int32Value();

    const CURLcode status = curl_easy_setopt(
        wrapper->easy_handle_, option, *String::Utf8Value(args[1]));

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

  Handle<Value> CurlEasyWrapper::Close(const Arguments& args) {
    HandleScope scope;
    CurlEasyWrapper* wrapper = ObjectWrap::Unwrap<CurlEasyWrapper>(args.This());

    return scope.Close(Undefined());
  }
}
