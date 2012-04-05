// Copyright 2012 Karl Skomski MIT

#include "curl_multi_wrapper.h"
#include "curl_easy_wrapper.h"
#include <vector>

extern "C" void init(v8::Handle<v8::Object> target) {
  nodecurl::CurlEasyWrapper::Initialize(target);
  nodecurl::CurlMultiWrapper::Initialize(target);
}
