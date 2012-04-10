// Copyright 2012 Karl Skomski MIT

#ifndef SRC_HELPERS_H_
#define SRC_HELPERS_H_

#include "node.h"
#include "curl/curl.h"

namespace helpers {
  void ThrowError(const char* message);
  void ThrowCurlError(CURLcode code);
  void ThrowCurlMultiError(CURLMcode code);

  void Emit(
       v8::Handle<v8::Object> source,
       const char* event,
       v8::Handle<v8::Value> object);

  void EmitError(
      v8::Handle<v8::Object> source,
      const char* error_message);

  void EmitCurlError(
      v8::Handle<v8::Object> source,
      CURLcode code);

  void EmitCurlMultiError(
      v8::Handle<v8::Object> source,
      CURLMcode code);

  void ProcessCallback(
      const v8::Handle<v8::Object>& object,
      const char* method,
      int argc,
      v8::Handle<v8::Value> argv[]);

  void ProcessCallback(
      const v8::Handle<v8::Object>& object,
      const v8::Handle<v8::Function>& callback,
      int argc,
      v8::Handle<v8::Value> argv[]);
}

#endif  // SRC_HELPERS_H_
