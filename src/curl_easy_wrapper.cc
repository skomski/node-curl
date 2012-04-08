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
  using v8::Number;

  CurlEasyWrapper::CurlEasyWrapper() :
      easy_handle_(curl_easy_init()), form_(NULL) {
    if (easy_handle_ == NULL) {
      helpers::ThrowError("EasyHandleInit failed!");
      return;
    }

    curl_easy_setopt(easy_handle_, CURLOPT_WRITEFUNCTION, WriteFunction);
    curl_easy_setopt(easy_handle_, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(easy_handle_, CURLOPT_PRIVATE, this);
  }

  CurlEasyWrapper::~CurlEasyWrapper() {
    curl_easy_cleanup(easy_handle_);
    if (form_ != NULL) curl_formfree(form_);
    std::vector<curl_slist*>::const_iterator i;
    for (i = option_lists_.begin(); i != option_lists_.end(); ++i) {
      assert(*i != NULL);
      curl_slist_free_all(*i);
    }
  }

  void CurlEasyWrapper::Initialize(Handle<Object> target) {
    Local<FunctionTemplate> wrapper_template = FunctionTemplate::New(New);
    wrapper_template->SetClassName(String::NewSymbol("CurlEasyWrapper"));
    wrapper_template->InstanceTemplate()->SetInternalFieldCount(1);

    wrapper_template->PrototypeTemplate()->Set(
        String::NewSymbol("_setNumberOption"),
        FunctionTemplate::New(SetNumberOption_)->GetFunction());

    wrapper_template->PrototypeTemplate()->Set(
        String::NewSymbol("_setStringOption"),
        FunctionTemplate::New(SetStringOption_)->GetFunction());

    wrapper_template->PrototypeTemplate()->Set(
        String::NewSymbol("_setListOption"),
        FunctionTemplate::New(SetListOption_)->GetFunction());

    wrapper_template->PrototypeTemplate()->Set(
        String::NewSymbol("_setFormData"),
        FunctionTemplate::New(SetFormData_)->GetFunction());

    wrapper_template->PrototypeTemplate()->Set(
        String::NewSymbol("_getStringInfo"),
        FunctionTemplate::New(GetStringInfo_)->GetFunction());

    wrapper_template->PrototypeTemplate()->Set(
        String::NewSymbol("_getDoubleInfo"),
        FunctionTemplate::New(GetDoubleInfo_)->GetFunction());

    wrapper_template->PrototypeTemplate()->Set(
        String::NewSymbol("_getIntegerInfo"),
        FunctionTemplate::New(GetIntegerInfo_)->GetFunction());

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
    CurlEasyWrapper *wrapper = static_cast<CurlEasyWrapper*>(userdata);
    return wrapper->OnData(ptr, size * nmemb);
  }

  size_t CurlEasyWrapper::OnData(char *data, size_t size) {
    HandleScope scope;

    node::Buffer *buffer = node::Buffer::New(data, size);
    helpers::Emit(
        this->handle_,
        "data",
        Local<Object>::New(buffer->handle_));

    return size;
  }

  Handle<Value> CurlEasyWrapper::SetFormData_(const Arguments& args) {
    HandleScope scope;
    CurlEasyWrapper* wrapper = ObjectWrap::Unwrap<CurlEasyWrapper>(args.This());

    if (args.Length() < 1) {
      helpers::ThrowError("Need 1 argument");
      return scope.Close(Undefined());
    }

    if (!args[0]->IsArray()) {
      helpers::ThrowError("Need array[1]");
      return scope.Close(Undefined());
    }

    if (wrapper->form_ != NULL) curl_formfree(wrapper->form_);
    wrapper->form_ = NULL;
    curl_httppost *last_post = NULL;

    Local<Array> entries = Local<Array>::Cast(args[0]);
    int entries_length = entries->Length();

    for (int i = 0; i < entries_length; i++) {
      Local<Object> entry = entries->Get(i)->ToObject();
      String::Utf8Value entry_name(entry->Get(String::NewSymbol("name")));
      String::Utf8Value entry_content(entry->Get(String::NewSymbol("content")));
      int type = entry->Get(String::NewSymbol("_type"))->Int32Value();

      const CURLFORMcode status = curl_formadd(&wrapper->form_, &last_post,
          CURLFORM_COPYNAME, *entry_name,
          (CURLformoption)type, *entry_content,
          CURLFORM_END);

      if (status != CURL_FORMADD_OK) {
        curl_formfree(wrapper->form_);
        helpers::ThrowError("FormAddError");
        return scope.Close(Undefined());
      }
    }

    const CURLcode status = curl_easy_setopt(
        wrapper->easy_handle_, CURLOPT_HTTPPOST, wrapper->form_);

    if (status != CURLE_OK) {
      curl_formfree(wrapper->form_);
      helpers::ThrowCurlError(status);
    }

    return scope.Close(Undefined());
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

    const CURLcode status = curl_easy_pause(
        wrapper->easy_handle_, CURLPAUSE_ALL);

    if (status != CURLE_OK) {
      helpers::EmitCurlError(wrapper->handle_, status);
    }

    return scope.Close(Undefined());
  }

  Handle<Value> CurlEasyWrapper::Resume(const Arguments& args) {
    HandleScope scope;
    CurlEasyWrapper* wrapper = ObjectWrap::Unwrap<CurlEasyWrapper>(args.This());

    const CURLcode status = curl_easy_pause(
        wrapper->easy_handle_, CURLPAUSE_CONT);

    if (status != CURLE_OK) {
      helpers::EmitCurlError(wrapper->handle_, status);
    }

    return scope.Close(Undefined());
  }

  template<typename InfoType, typename ReturnType>
  Handle<Value> CurlEasyWrapper::GetInfo_(const v8::Arguments &args) {
    CurlEasyWrapper* wrapper = ObjectWrap::Unwrap<CurlEasyWrapper>(args.This());

    if (args.Length() < 1) {
      helpers::ThrowError("Need 1 argument");
      return Undefined();
    }

    if (!args[0]->IsNumber()) {
      helpers::ThrowError("Need number[0]");
      return Undefined();
    }

    const CURLINFO info = (CURLINFO)args[0]->Int32Value();

    InfoType infoData;

    const CURLcode status = curl_easy_getinfo(
        wrapper->easy_handle_, info, &infoData);

    if (status != CURLE_OK) {
      helpers::ThrowCurlError(status);
      return Undefined();
    }

    return ReturnType::New(infoData);
  }

  Handle<Value> CurlEasyWrapper::GetStringInfo_(const Arguments& args) {
    HandleScope scope;
    CurlEasyWrapper* wrapper = ObjectWrap::Unwrap<CurlEasyWrapper>(args.This());
    return scope.Close(wrapper->GetInfo_<char*, v8::String>(args));
  }

  Handle<Value> CurlEasyWrapper::GetDoubleInfo_(const Arguments& args) {
    HandleScope scope;
    CurlEasyWrapper* wrapper = ObjectWrap::Unwrap<CurlEasyWrapper>(args.This());
    return scope.Close(wrapper->GetInfo_<double, v8::Number>(args));
  }

  Handle<Value> CurlEasyWrapper::GetIntegerInfo_(const Arguments& args) {
    HandleScope scope;
    CurlEasyWrapper* wrapper = ObjectWrap::Unwrap<CurlEasyWrapper>(args.This());
    return scope.Close(wrapper->GetInfo_<int32_t, v8::Integer>(args));
  }

  Handle<Value> CurlEasyWrapper::Close(const Arguments& args) {
    HandleScope scope;
    return scope.Close(Undefined());
  }
}
