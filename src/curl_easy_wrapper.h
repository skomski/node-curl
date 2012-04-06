// Copyright 2012 Karl Skomski MIT

#ifndef SRC_CURL_EASY_WRAPPER_H_
#define SRC_CURL_EASY_WRAPPER_H_

#include "curl/curl.h"
#include "node.h"
#include "helpers.h"
#include <map>

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
      static v8::Handle<v8::Value> Pause(const v8::Arguments& args);
      static v8::Handle<v8::Value> Resume(const v8::Arguments& args);

      size_t OnData(char* data, size_t size);
      static size_t WriteFunction(
          char* ptr, size_t size, size_t nmemb, void *userdata);

      CURL* const easy_handle_;
  };
}

#endif  // SRC_CURL_EASY_WRAPPER_H_
