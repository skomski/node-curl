// Copyright 2012 Karl Skomski MIT

#ifndef SRC_CURL_EASY_WRAPPER_H_
#define SRC_CURL_EASY_WRAPPER_H_

#include <vector>

#include "curl/curl.h"
#include "node.h"

#include "helpers.h"

namespace nodecurl {
  class CurlEasyWrapper : public node::ObjectWrap {
    public:
      static void Initialize(v8::Handle<v8::Object> target);
      CURL*  getHandle() const { return easy_handle_; }

    private:
      CurlEasyWrapper();
      ~CurlEasyWrapper();

      static v8::Handle<v8::Value> New(const v8::Arguments& args);

      static v8::Handle<v8::Value> SetStringOption_(const v8::Arguments& args);
      static v8::Handle<v8::Value> SetNumberOption_(const v8::Arguments& args);
      static v8::Handle<v8::Value> SetListOption_(const v8::Arguments& args);
      static v8::Handle<v8::Value> SetFormData_(const v8::Arguments& args);

      template<typename InfoType, typename ReturnType>
      static v8::Handle<v8::Value> GetInfo_(const v8::Arguments &args);
      static v8::Handle<v8::Value> GetStringInfo_(const v8::Arguments& args);
      static v8::Handle<v8::Value> GetDoubleInfo_(const v8::Arguments& args);
      static v8::Handle<v8::Value> GetIntegerInfo_(const v8::Arguments& args);

      static v8::Handle<v8::Value> Pause(const v8::Arguments& args);
      static v8::Handle<v8::Value> Resume(const v8::Arguments& args);
      static v8::Handle<v8::Value> Close(const v8::Arguments& args);

      size_t OnData(char* data, size_t size);
      static size_t WriteFunction(
          char* ptr, size_t size, size_t nmemb, void *userdata);

      CURL* const easy_handle_;
      std::vector<curl_slist*> option_lists_;
      curl_httppost *form_;
  };
}

#endif  // SRC_CURL_EASY_WRAPPER_H_
