// Copyright 2012 Karl Skomski MIT

#ifndef SRC_CURL_WRAPPER_H_
#define SRC_CURL_WRAPPER_H_

#include "curl/curl.h"
#include "node.h"
#include "helpers.h"
#include <map>

namespace nodecurl {
  class CurlWrapper : public node::ObjectWrap {
    public:
      static void Initialize(v8::Handle<v8::Object> target);
    private:
      typedef std::map<curl_socket_t, ev_io> SockFDs;

      CurlWrapper();
      ~CurlWrapper();

      static v8::Handle<v8::Value> New(const v8::Arguments& args);

      static v8::Handle<v8::Value> Execute(const v8::Arguments& args);
      static v8::Handle<v8::Value> Close(const v8::Arguments& args);
      static v8::Handle<v8::Value> SetOption(const v8::Arguments& args);

      static void ExecuteWork(uv_work_t *job);
      static void ExecuteDone(uv_work_t *job);

      static int TimerFunction(CURLM* mh, long timeout, void* userp);
      static int SocketFunction(CURLM* mh, curl_socket_t sockfd,
          int events, void* userp, void* socketp);
      static void IOEventFunction(EV_P_ ev_io* watcher, int events);
      static void TimerEventFunction(EV_P_ ev_timer* timer, int events);

      bool ProcessEvents(int fd, int action);

      size_t OnData(char* data, size_t size);
      static size_t WriteFunction(
          char* ptr, size_t size, size_t nmemb, void *userdata);


      CURLM* const multi_handle_;
      unsigned int num_easy_handles_;
      SockFDs socket_fds_;
      ev_timer timeout_timer_;
  };
}

#endif  // SRC_CURL_WRAPPER_H_