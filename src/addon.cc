// Copyright 2012 Karl Skomski MIT

#include "curl_wrapper.h"

extern "C" void init(v8::Handle<v8::Object> target) {
  nodecurl::CurlWrapper::Initialize(target);
}
