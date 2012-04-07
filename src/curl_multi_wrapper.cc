// Copyright 2012 Karl Skomski MIT

#include "curl_multi_wrapper.h"

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

  CurlMultiWrapper::CurlMultiWrapper() :
      multi_handle_(curl_multi_init()), num_easy_handles_(0) {
    if (multi_handle_ == NULL) {
      ThrowException(String::New("MultiHandleInit failed!"));
      return;
    }

    curl_multi_setopt(multi_handle_, CURLMOPT_SOCKETFUNCTION, SocketFunction);
    curl_multi_setopt(multi_handle_, CURLMOPT_SOCKETDATA, this);

    curl_multi_setopt(multi_handle_, CURLMOPT_TIMERFUNCTION, TimerFunction);
    curl_multi_setopt(multi_handle_, CURLMOPT_TIMERDATA, this);

    ev_init(&timeout_timer_, CurlMultiWrapper::TimerEventFunction);
    timeout_timer_.data = this;
  }

  CurlMultiWrapper::~CurlMultiWrapper() {
    curl_multi_cleanup(multi_handle_);
  }

  void CurlMultiWrapper::Initialize(Handle<Object> target) {
    if (curl_global_init(CURL_GLOBAL_ALL) != 0) {
      helpers::ThrowError("CurlGlobalInit failed");
      return;
    }

    Local<FunctionTemplate> wrapper_template = FunctionTemplate::New(New);
    wrapper_template->SetClassName(String::NewSymbol("CurlMultiWrapper"));
    wrapper_template->InstanceTemplate()->SetInternalFieldCount(1);

    wrapper_template->PrototypeTemplate()->Set(String::NewSymbol("execute"),
        FunctionTemplate::New(Execute)->GetFunction());

    wrapper_template->PrototypeTemplate()->Set(String::NewSymbol("close"),
        FunctionTemplate::New(Close)->GetFunction());

    Persistent<Function> constructor = Persistent<Function>::New(
        wrapper_template->GetFunction());
    target->Set(String::NewSymbol("CurlMultiWrapper"), constructor);
  }

  Handle<Value> CurlMultiWrapper::New(const Arguments& args) {
    HandleScope scope;

    CurlMultiWrapper* wrapper = new CurlMultiWrapper();
    wrapper->Wrap(args.This());

    return scope.Close(args.This());
  }

  bool CurlMultiWrapper::ProcessEvents(int fd, int action) {
    int running_handles = 0;

    const CURLMcode status = curl_multi_socket_action(
        multi_handle_, fd, action, &running_handles);

    if (status != CURLM_OK) {
      helpers::EmitCurlMultiError(this->handle_, status);
      return false;
    }

    if (running_handles == 0) ev_timer_stop(EV_DEFAULT_UC_ &timeout_timer_);

    int messages_in_queue = 0;
    CURLMsg *message;

    while (
        (message = curl_multi_info_read(multi_handle_, &messages_in_queue))) {
      if (message->msg == CURLMSG_DONE) {
        char* handle;
        curl_easy_getinfo(message->easy_handle, CURLINFO_PRIVATE, &handle);

        if (message->data.result != CURLE_OK) {
          helpers::EmitCurlError(*reinterpret_cast<Handle<Object>*>(handle),
              message->data.result);
        }
        helpers::Emit(*reinterpret_cast<Handle<Object>*>(handle), "end",
            Undefined());

        curl_multi_remove_handle(multi_handle_, message->easy_handle);

        this->num_easy_handles_ -= 1;
        if (running_handles == 0 && num_easy_handles_ == 0) {
          ev_unref(EV_DEFAULT_UC);
        }
      }
    }

    assert(messages_in_queue == 0);

    return status == CURLM_OK;
  }

  void CurlMultiWrapper::TimerEventFunction(EV_P_ ev_timer* timer, int events) {
    CurlMultiWrapper* wrapper = static_cast<CurlMultiWrapper*>(timer->data);
    ev_timer_stop(EV_A_ timer);
    wrapper->ProcessEvents(CURL_SOCKET_TIMEOUT, 0);
  }

  int CurlMultiWrapper::TimerFunction(CURLM* , int timeout, void* userp) {
    CurlMultiWrapper* wrapper = static_cast<CurlMultiWrapper*>(userp);

    if (timeout == 0) {
      wrapper->ProcessEvents(CURL_SOCKET_TIMEOUT, 0);
      return CURLM_OK;
    }

    if (timeout == -1) {
      timeout = 5000;
    }

    wrapper->timeout_timer_.repeat = timeout / 1000.;
    ev_timer_again(EV_DEFAULT_UC_ &wrapper->timeout_timer_);

    return CURLM_OK;
  }

  int CurlMultiWrapper::SocketFunction(CURLM* /*handle*/, curl_socket_t sockfd,
      int events, void* userp, void* /*socketp*/) {
    CurlMultiWrapper* wrapper = static_cast<CurlMultiWrapper*>(userp);

    int lib_events =
      (events & CURL_POLL_IN ?  EV_READ  : 0) |
      (events & CURL_POLL_OUT ? EV_WRITE : 0);

    SockFDs::iterator it = wrapper->socket_fds_.find(sockfd);
    if (it == wrapper->socket_fds_.end()) {
      if (lib_events) {
        // create I/O watcher and add it to the list
        ev_io *watcher = &wrapper->socket_fds_.insert(
            SockFDs::value_type(sockfd, ev_io())).first->second;

        watcher->data = wrapper;
        ev_io_init(watcher, IOEventFunction, sockfd, lib_events);
        ev_io_start(EV_DEFAULT_UC_ watcher);
      } else {
        assert(0 && "CURL_POLL_NONE or CURL_POLL_REMOVE for bad socket");
      }
    } else {
      ev_io *watcher = &it->second;
      if (lib_events) {
        ev_io_stop(EV_DEFAULT_UC_ watcher);
        ev_io_set(watcher, sockfd, lib_events);
        ev_io_start(EV_DEFAULT_UC_ watcher);
      } else {
        ev_io_stop(EV_DEFAULT_UC_ watcher);
        wrapper->socket_fds_.erase(it);
      }
    }

    return CURLM_OK;
  }

  void CurlMultiWrapper::IOEventFunction(EV_P_ ev_io* watcher, int events) {
    CurlMultiWrapper* wrapper = static_cast<CurlMultiWrapper*>(watcher->data);
    int action =
        (events & EV_READ ? CURL_CSELECT_IN : 0) |
        (events & EV_WRITE ? CURL_CSELECT_OUT : 0);
    wrapper->ProcessEvents(watcher->fd, action);
  }

  Handle<Value> CurlMultiWrapper::Execute(const Arguments& args) {
    HandleScope scope;
    CurlMultiWrapper* wrapper = ObjectWrap::Unwrap<CurlMultiWrapper>(
        args.This());
    CurlEasyWrapper* easy_wrapper = ObjectWrap::Unwrap<CurlEasyWrapper>(
        args[0]->ToObject());

    CURLMcode status = curl_multi_add_handle(
        wrapper->multi_handle_, easy_wrapper->getHandle());

    if (status != CURLM_OK) {
      helpers::EmitCurlMultiError(wrapper->handle_, status);
      return scope.Close(Undefined());
    }

    wrapper->num_easy_handles_ += 1;

    if (wrapper->num_easy_handles_ == 1) ev_ref(EV_DEFAULT_UC);

    return scope.Close(Undefined());
  }

  Handle<Value> CurlMultiWrapper::Close(const Arguments& args) {
    HandleScope scope;
    CurlMultiWrapper* wrapper = ObjectWrap::Unwrap<CurlMultiWrapper>(
        args.This());

    curl_multi_cleanup(wrapper->multi_handle_);

    return scope.Close(Undefined());
  }
}
