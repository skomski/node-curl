// Copyright 2012 Karl Skomski MIT

#ifndef SRC_CURL_MULTI_WRAPPER_H_
#define SRC_CURL_MULTI_WRAPPER_H_

#include <map>

#include "curl/curl.h"
#include "node.h"

#include "helpers.h"
#include "curl_easy_wrapper.h"

namespace nodecurl {
  class CurlMultiWrapper : public node::ObjectWrap {
    public:
      static void Initialize(v8::Handle<v8::Object> target);

    private:
      typedef std::map<curl_socket_t, ev_io> SockFDs;

      CurlMultiWrapper();
      ~CurlMultiWrapper();



      static v8::Handle<v8::Value> New(const v8::Arguments& args);

      static v8::Handle<v8::Value> Execute(const v8::Arguments& args);
      static v8::Handle<v8::Value> Close(const v8::Arguments& args);

      static int TimerFunction(CURLM* mh, int timeout, void* userp);
      static int SocketFunction(CURLM* mh, curl_socket_t sockfd,
          int events, void* userp, void* socketp);
      static void IOEventFunction(EV_P_ ev_io* watcher, int events);
      static void TimerEventFunction(EV_P_ ev_timer* timer, int events);

      bool ProcessEvents(int fd, int action);

      CURLM* const multi_handle_;
      unsigned int num_easy_handles_;
      SockFDs socket_fds_;
      ev_timer timeout_timer_;
  };
}

#endif  // SRC_CURL_MULTI_WRAPPER_H_
