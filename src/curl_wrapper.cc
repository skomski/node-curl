// Copyright 2012 Karl Skomski MIT

#include "curl_wrapper.h"
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

  CurlWrapper::CurlWrapper() : multi_handle_(curl_multi_init()), num_easy_handles_(0) {
    if (multi_handle_ == NULL) {
      ThrowException(String::New("MultiHandleInit failed!"));
      return;
    }

    curl_multi_setopt(multi_handle_, CURLMOPT_SOCKETFUNCTION, SocketFunction);
    curl_multi_setopt(multi_handle_, CURLMOPT_SOCKETDATA, this);

    curl_multi_setopt(multi_handle_, CURLMOPT_TIMERFUNCTION, TimerFunction);
    curl_multi_setopt(multi_handle_, CURLMOPT_TIMERDATA, this);

    ev_init(&timeout_timer_, TimerEventFunction);
    timeout_timer_.data = this;
  }

  CurlWrapper::~CurlWrapper() {}

  void CurlWrapper::Initialize(Handle<Object> target) {
    if (curl_global_init(CURL_GLOBAL_ALL) != 0) {
      helpers::ThrowError("CurlGlobalInit failed");
      return;
    }

    Local<FunctionTemplate> wrapper_template = FunctionTemplate::New(New);
    wrapper_template->SetClassName(String::NewSymbol("CurlWrapper"));
    wrapper_template->InstanceTemplate()->SetInternalFieldCount(1);

    wrapper_template->PrototypeTemplate()->Set(String::NewSymbol("execute"),
        FunctionTemplate::New(Execute)->GetFunction());

    wrapper_template->PrototypeTemplate()->Set(String::NewSymbol("setOption"),
        FunctionTemplate::New(SetOption)->GetFunction());

    wrapper_template->PrototypeTemplate()->Set(String::NewSymbol("close"),
        FunctionTemplate::New(Close)->GetFunction());

    Persistent<Function> constructor = Persistent<Function>::New(
        wrapper_template->GetFunction());
    target->Set(String::NewSymbol("CurlWrapper"), constructor);
  }

  Handle<Value> CurlWrapper::New(const Arguments& args) {
    HandleScope scope;

    CurlWrapper* wrapper = new CurlWrapper();
    wrapper->Wrap(args.This());

    return scope.Close(args.This());
  }

  size_t CurlWrapper::WriteFunction(char *ptr, size_t size, size_t nmemb,
      void *userdata) {
    fprintf(stderr, "write - size=%ld\n", size);
    CurlWrapper *wrapper = (CurlWrapper*)userdata;
    return wrapper->OnData(ptr, size * nmemb);
  }

  bool CurlWrapper::ProcessEvents(int fd, int action) {
    int running_handles = 0;

    CURLMcode status = curl_multi_socket_action(
        multi_handle_, fd, action, &running_handles);

    fprintf(stderr, "fd=%d action=%d status=%d\n", fd, action, status);

    if (status != CURLM_OK) {
      helpers::EmitCurlMultiError(this->handle_, status);
      return false;
    }

    if (running_handles == 0) {
      fprintf(stderr, "running_handles=%d\n", running_handles);
      ev_timer_stop(ev_default_loop(0), &timeout_timer_);
      //this->Unref();
    }

    int messages_in_queue = 0;
    CURLMsg *message;

    while ((message = curl_multi_info_read(multi_handle_, &messages_in_queue))) {
      if (message->msg == CURLMSG_DONE) {
        curl_multi_remove_handle(multi_handle_, message->easy_handle);
        curl_easy_cleanup(message->easy_handle);
      }
    }

    assert(messages_in_queue == 0);

    return status == CURLM_OK;
  }

  void CurlWrapper::TimerEventFunction(EV_P_ ev_timer* timer, int events) {
    CurlWrapper* wrapper = static_cast<CurlWrapper*>(timer->data);
    wrapper->ProcessEvents(CURL_SOCKET_TIMEOUT, 0);
  }

  int CurlWrapper::TimerFunction(CURLM* /*handle*/, long timeout, void* userp) {
    CurlWrapper* wrapper = static_cast<CurlWrapper*>(userp);

    ev_timer_stop(ev_default_loop(0), &wrapper->timeout_timer_);

    if (timeout == -1) {
      wrapper->ProcessEvents(CURL_SOCKET_TIMEOUT, 0);
      return CURLM_OK;
    }

    if (timeout == 0) {
      timeout = 5000;
    }

    fprintf(stderr, "timeout=%ld\n", timeout);

    ev_timer_set(&wrapper->timeout_timer_, timeout / 1000, 0.);
    ev_timer_start(ev_default_loop(0), &wrapper->timeout_timer_);

    return CURLM_OK;
  }

  int CurlWrapper::SocketFunction(CURLM* /*handle*/, curl_socket_t sockfd, int events,
      void* userp, void* /*socketp*/) {
    CurlWrapper* wrapper = static_cast<CurlWrapper*>(userp);

    fprintf(stderr, "sockfd=%d events=%d\n", sockfd, events);

    int lib_events =
      (events & CURL_POLL_IN ?  EV_READ  : 0) |
      (events & CURL_POLL_OUT ? EV_WRITE : 0) ;

    SockFDs::iterator it = wrapper->socket_fds_.find(sockfd);
    if (it == wrapper->socket_fds_.end()) {
      if (events) {
        fprintf(stderr, "create\n");
        // create I/O watcher and add it to the list
        ev_io& watcher = wrapper->socket_fds_.insert(
            SockFDs::value_type(sockfd, ev_io())).first->second;

        ev_io_init(&watcher, IOEventFunction, sockfd, lib_events);

        ev_io_start(ev_default_loop(0), &watcher);
        watcher.data = wrapper;
      } else {
        assert(0 && "CURL_POLL_NONE or CURL_POLL_REMOVE for bad socket");
      }
    } else {
      ev_io& watcher = it->second;
      if (events) {
        // update the event flags
        fprintf(stderr, "update\n");
        ev_io_set(&watcher, sockfd, events);
      }
      else {
        // disarm and dispose fd watcher
        fprintf(stderr, "disarm\n");
        ev_io_stop(ev_default_loop(0), &watcher);
        wrapper->socket_fds_.erase(it);
      }
    }

    return CURLM_OK;
  }

  void CurlWrapper::IOEventFunction(EV_P_ ev_io* watcher, int events) {
    CurlWrapper* wrapper = static_cast<CurlWrapper*>(watcher->data);
    int action =
        (events & EV_READ ? CURL_CSELECT_IN : 0) |
        (events & EV_WRITE ? CURL_CSELECT_OUT : 0);
    wrapper->ProcessEvents(watcher->fd, action);
  }

  size_t CurlWrapper::OnData(char *data, size_t size) {
    assert(size > 0);

    node::Buffer *buffer = node::Buffer::New(data, size);
    helpers::Emit(this->handle_, "data", buffer->handle_);
    return size;
  }

  Handle<Value> CurlWrapper::Execute(const Arguments& args) {
    HandleScope scope;
    CurlWrapper* wrapper = ObjectWrap::Unwrap<CurlWrapper>(args.This());

    CURL *easy_handle = curl_easy_init();
    if(easy_handle == NULL) {
      helpers::EmitError(wrapper->handle_, "CurlEasyInit failed");
      return scope.Close(Undefined());
    }

    curl_easy_setopt(easy_handle, CURLOPT_URL, "http://google.com");
    curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, WriteFunction);
    curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, wrapper);

    CURLMcode status = curl_multi_add_handle(wrapper->multi_handle_, easy_handle);
    if (status != CURLM_OK) {
      helpers::EmitCurlMultiError(wrapper->handle_, status);
      return scope.Close(Undefined());
    }

    wrapper->num_easy_handles_ += 1;

    if (wrapper->num_easy_handles_ == 1) {
      wrapper->Ref();
    }

    return scope.Close(Undefined());
  }

  Handle<Value> CurlWrapper::Close(const Arguments& args) {
    HandleScope scope;
    CurlWrapper* wrapper = ObjectWrap::Unwrap<CurlWrapper>(args.This());

    curl_multi_cleanup(wrapper->multi_handle_);

    return scope.Close(Undefined());
  }

  Handle<Value> CurlWrapper::SetOption(const Arguments& args) {
    HandleScope scope;

    return scope.Close(Undefined());
  }
}
