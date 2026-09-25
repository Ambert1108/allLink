/*
 *  Copyright 2012 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef EXAMPLES_PEERCONNECTION_CLIENT_MAIN_WND_H_
#define EXAMPLES_PEERCONNECTION_CLIENT_MAIN_WND_H_

#include <map>
#include <memory>
#include <string>

#include "api/media_stream_interface.h"
#include "api/video/video_frame.h"
#include "peer_connection_client.h"
#include "media/base/media_channel.h"
#include "media/base/video_common.h"
#if defined(WEBRTC_WIN)
#include "rtc_base/win32.h"
#endif  // WEBRTC_WIN
#include "seeker/logger.h"
#include<seeker/loggerApi.h>
#include <opencv2/opencv.hpp>
#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"  // 引入SDL库


class MainWndCallback {
 public:
  virtual void StartLogin(const std::string& server, int port) = 0;
  virtual void DisconnectFromServer() = 0;
  virtual void ConnectToPeer(int peer_id) = 0;
  virtual void DisconnectFromCurrentPeer() = 0;
  virtual void UIThreadCallback(int msg_id, void* data) = 0;
  virtual void Close() = 0;

 protected:
  virtual ~MainWndCallback() {}
};

// Pure virtual interface for the main window.
class MainWindow {
 public:
  virtual ~MainWindow() {}

  enum UI {
    CONNECT_TO_SERVER,
    LIST_PEERS,
    STREAMING,
  };

  virtual void RegisterObserver(MainWndCallback* callback) = 0;

  virtual bool IsWindow() = 0;
  virtual void MessageBox(const char* caption,
                          const char* text,
                          bool is_error) = 0;

  virtual UI current_ui() = 0;

  virtual void SwitchToConnectUI() = 0;
  virtual void SwitchToPeerList(const Peers& peers) = 0;
  virtual void SwitchToStreamingUI() = 0;

  virtual void StartLocalRenderer(webrtc::VideoTrackInterface* local_video) = 0;
  virtual void StopLocalRenderer() = 0;
  virtual void StartRemoteRenderer(
      webrtc::VideoTrackInterface* remote_video) = 0;
  virtual void StopRemoteRenderer() = 0;

  virtual void QueueUIThreadCallback(int msg_id, void* data) = 0;
};

#ifdef WIN32

class MainWnd : public MainWindow {
 public:
  static const wchar_t kClassName[];

  enum WindowMessages {
    UI_THREAD_CALLBACK = WM_APP + 1,
  };

  MainWnd(const char* server, int port, bool auto_connect, bool auto_call);
  ~MainWnd();

  bool Create();
  bool Destroy();
  bool PreTranslateMessage(MSG* msg);
  bool PreTranslateMessage();

  virtual void RegisterObserver(MainWndCallback* callback);
  virtual bool IsWindow();
  virtual void SwitchToConnectUI();
  virtual void SwitchToPeerList(const Peers& peers);
  virtual void SwitchToStreamingUI();
  virtual void MessageBox(const char* caption, const char* text, bool is_error);
  virtual UI current_ui() { return ui_; }

  virtual void StartLocalRenderer(webrtc::VideoTrackInterface* local_video);
  virtual void StopLocalRenderer();
  virtual void StartRemoteRenderer(webrtc::VideoTrackInterface* remote_video);
  virtual void StopRemoteRenderer();

  virtual void QueueUIThreadCallback(int msg_id, void* data);

  //HWND handle() const { return wnd_; }

  class VideoRenderer : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
   public:
    VideoRenderer(SDL_Window* window,
                    int width,
                  int height,
                  webrtc::VideoTrackInterface* track_to_render,
                  bool remote);
    virtual ~VideoRenderer();

    void Lock() { ::EnterCriticalSection(&buffer_lock_); }

    void Unlock() { ::LeaveCriticalSection(&buffer_lock_); }

    // VideoSinkInterface implementation
    void OnFrame(const webrtc::VideoFrame& frame) override;

    const BITMAPINFO& bmi() const { return bmi_; }
    const uint8_t* image() const { return image_.get(); }
    bool remoteFlag = false;
   protected:
    void SetSize(int width, int height);

    enum {
      SET_SIZE,
      RENDER_FRAME,
    };

    BITMAPINFO bmi_;
    std::unique_ptr<uint8_t[]> image_;
    CRITICAL_SECTION buffer_lock_;
    rtc::scoped_refptr<webrtc::VideoTrackInterface> rendered_track_;
    int i=0;
    int videoIndex = 0;
    SDL_Window* window = NULL;
    SDL_Rect rect;                      // 矩形区域
    SDL_Renderer* renderer = NULL;      // SDL 渲染器
    SDL_Texture* texture = NULL;        // SDL 纹理

    SDL_Renderer* renderer_remote = NULL;      // SDL 渲染器
    SDL_Texture* texture_remote = NULL;        // SDL 纹理

    SDL_Thread* timer_thread = NULL;    // 刷新线程
    uint32_t pixformat = SDL_PIXELFORMAT_IYUV;    // YUV格式
    int video_width = -1;        // 视频宽度
    int video_height = -1;      // 视频高度

  };

  // A little helper class to make sure we always to proper locking and
  // unlocking when working with VideoRenderer buffers.
  template <typename T>
  class AutoLock {
   public:
    explicit AutoLock(T* obj) : obj_(obj) { obj_->Lock(); }
    ~AutoLock() { obj_->Unlock(); }

   protected:
    T* obj_;
  };

 protected:
  enum ChildWindowID {
    EDIT_ID = 1,
    BUTTON_ID,
    LABEL1_ID,
    LABEL2_ID,
    LISTBOX_ID,
  };
  SDL_Rect rect;                      // 矩形区域
  SDL_Window* window = NULL;          // SDL 窗口
  SDL_Renderer* renderer = NULL;      // SDL 渲染器
  SDL_Texture* texture = NULL;        // SDL 纹理
  SDL_Renderer* renderer_remote = NULL;      // SDL 渲染器
  SDL_Texture* texture_remote = NULL;        // SDL 纹理
  SDL_Thread* timer_thread = NULL;    // 刷新线程
  uint32_t pixformat = SDL_PIXELFORMAT_IYUV;    // YUV格式
  // YUV 视频 的 分辨率设置
  //int video_width = 800;        // 视频宽度
  //int video_height = 800;      // 视频高度
  // SDL 播放窗口 分辨率设置
  int win_width = 640;          // 窗口宽度
  int win_height = 1000;        // 窗口高度

  void OnPaint();
  void OnDestroyed();

  void OnDefaultAction();

  bool OnMessage(UINT msg, WPARAM wp, LPARAM lp, LRESULT* result);

  static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
  static bool RegisterWindowClass();

  void CreateChildWindow(HWND* wnd,
                         ChildWindowID id,
                         const wchar_t* class_name,
                         DWORD control_style,
                         DWORD ex_style);
  void CreateChildWindows();

  void LayoutConnectUI(bool show);
  void LayoutPeerListUI(bool show);

  void HandleTabbing();

 private:
  std::unique_ptr<VideoRenderer> local_renderer_;
  std::unique_ptr<VideoRenderer> remote_renderer_;
  UI ui_;
  //HWND wnd_;
  DWORD ui_thread_id_;
  //HWND edit1_;
  //HWND edit2_;
  //HWND label1_;
  //HWND label2_;
  //HWND button_;
  //HWND listbox_;
  bool destroyed_;
  void* nested_msg_;
  MainWndCallback* callback_;
  static ATOM wnd_class_;
  std::string server_;
  std::string port_;
  bool auto_connect_;
  bool auto_call_;
};
#endif  // WIN32

#endif  // EXAMPLES_PEERCONNECTION_CLIENT_MAIN_WND_H_
