#include "main_wnd.h"

#include <math.h>

#include "api/video/i420_buffer.h"
#include "defaults.h"
#include "rtc_base/arraysize.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "third_party/libyuv/include/libyuv/convert_argb.h"
#include <random>

ATOM MainWnd::wnd_class_ = 0;
const wchar_t MainWnd::kClassName[] = L"WebRTC_MainWnd";

namespace {

const char kConnecting[] = "Connecting... ";
const char kNoVideoStreams[] = "(no video streams either way)";
const char kNoIncomingStream[] = "(no incoming video)";

void CalculateWindowSizeForText(HWND wnd,
                                const wchar_t* text,
                                size_t* width,
                                size_t* height) {
  HDC dc = ::GetDC(wnd);
  RECT text_rc = {0};
  ::DrawTextW(dc, text, -1, &text_rc, DT_CALCRECT | DT_SINGLELINE);
  ::ReleaseDC(wnd, dc);
  RECT client, window;
  ::GetClientRect(wnd, &client);
  ::GetWindowRect(wnd, &window);

  *width = text_rc.right - text_rc.left;
  *width += (window.right - window.left) - (client.right - client.left);
  *height = text_rc.bottom - text_rc.top;
  *height += (window.bottom - window.top) - (client.bottom - client.top);
}

HFONT GetDefaultFont() {
  static HFONT font = reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
  return font;
}

std::string GetWindowText(HWND wnd) {
  char text[MAX_PATH] = {0};
  //::GetWindowTextA(wnd, &text[0], ARRAYSIZE(text));//控制台获取
  return text;
}

void AddListBoxItem(HWND listbox, const std::string& str, LPARAM item_data) {
  //LRESULT index = ::SendMessageA(listbox, LB_ADDSTRING, 0,
  //                               reinterpret_cast<LPARAM>(str.c_str()));
  //::SendMessageA(listbox, LB_SETITEMDATA, index, item_data);
}

}  // namespace

MainWnd::MainWnd(const char* server,
                 int port,
                 bool auto_connect,
                 bool auto_call)
    : ui_(CONNECT_TO_SERVER),
      destroyed_(false),
      nested_msg_(NULL),
      callback_(NULL),
      server_(server),
      auto_connect_(auto_connect),
      auto_call_(auto_call) {
  char buffer[10];
  snprintf(buffer, sizeof(buffer), "%i", port);
  port_ = buffer;
}

MainWnd::~MainWnd() {
  //RTC_DCHECK(!IsWindow());

}

bool MainWnd::Create() {
    I_LOG("init sdl");

    std::string name = std::to_string(rand());
    E_LOG("name {}", name);
    window = SDL_CreateWindow(name.c_str(),  // 窗口标题
        SDL_WINDOWPOS_UNDEFINED,  // 窗口x坐标
        SDL_WINDOWPOS_UNDEFINED,  // 窗口y坐标
        win_width, win_height, // 窗口宽高
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE); // 窗口属性
    if (!window)  // 如果创建失败
    {
        E_LOG("window create fail");
    }
    I_LOG("init sdl end");
  ui_thread_id_ = ::GetCurrentThreadId();// 获取当前线程ID并存储，这将用于后续的UI操作。
  SwitchToConnectUI();//
  return true;// 如果窗口句柄不为NULL，说明窗口创建成功，返回true；否则返回false。
}

bool MainWnd::Destroy() {
  BOOL ret = FALSE;
  //if (IsWindow()) {
  //  ret = ::DestroyWindow(wnd_);//销毁w32窗口
  //}

  return ret != FALSE;
}

void MainWnd::RegisterObserver(MainWndCallback* callback) {
  callback_ = callback;
}

bool MainWnd::IsWindow() {
    return true;
}

bool MainWnd::PreTranslateMessage(MSG* msg) {
  bool ret = false;

  //callback_->UIThreadCallback(static_cast<int>(msg->wParam),
  //                              reinterpret_cast<void*>(msg->lParam));
  

  //if (msg->message == WM_CHAR) {
  //  if (msg->wParam == VK_TAB) {
  //    HandleTabbing();
  //    ret = true;
  //  } else if (msg->wParam == VK_RETURN) {
  //    OnDefaultAction();
  //    ret = true;
  //  } else if (msg->wParam == VK_ESCAPE) {
  //    if (callback_) {
  //      if (ui_ == STREAMING) {
  //        callback_->DisconnectFromCurrentPeer();
  //      } else {
  //        callback_->DisconnectFromServer();
  //      }
  //    }
  //  }
  //}
  // 
  if (msg->hwnd == NULL && msg->message == UI_THREAD_CALLBACK) {
      I_LOG("PreTranslateMessage(MSG * msg)");
      callback_->UIThreadCallback(static_cast<int>(msg->wParam),
          reinterpret_cast<void*>(msg->lParam));
  }

  //  ret = true;
  //}
  return ret;
}

bool MainWnd::PreTranslateMessage() {
    OnDefaultAction();
    return true;
}

void MainWnd::SwitchToConnectUI() {
  ui_ = CONNECT_TO_SERVER;
  return;
}


void MainWnd::SwitchToPeerList(const Peers& peers) {
  LayoutConnectUI(false);

  //::SendMessage(listbox_, LB_RESETCONTENT, 0, 0);

  //AddListBoxItem(listbox_, "List of currently connected peers:", -1);
  I_LOG("####connect peer info update#####");
  Peers::const_iterator i = peers.begin();
  for (; i != peers.end(); ++i)
    I_LOG("connect peers {} {}",i->first,i->second);
  I_LOG("####connect peer info done#####");

  ui_ = LIST_PEERS;

  //if (auto_call_ && peers.begin() != peers.end()) {
  //  // Get the number of items in the list
  // // LRESULT count = ::SendMessage(listbox_, LB_GETCOUNT, 0, 0);
  //  //if (count != LB_ERR) {
  //  //  // Select the last item in the list
  //  //  LRESULT selection = ::SendMessage(listbox_, LB_SETCURSEL, count - 1, 0);
  //  //  if (selection != LB_ERR)
  //  //    ::PostMessage(wnd_, WM_COMMAND,
  //  //                  MAKEWPARAM(GetDlgCtrlID(listbox_), LBN_DBLCLK),
  //  //                  reinterpret_cast<LPARAM>(listbox_));
  //  //}
  //}
}

void MainWnd::SwitchToStreamingUI() {
    I_LOG("switchToStreamingUI");
  LayoutConnectUI(false);
  LayoutPeerListUI(false);
  ui_ = STREAMING;
}

void MainWnd::MessageBox(const char* caption, const char* text, bool is_error) {
  DWORD flags = MB_OK;
  if (is_error)
    flags |= MB_ICONERROR;

 // ::MessageBoxA(handle(), text, caption, flags);
}

void MainWnd::StartLocalRenderer(webrtc::VideoTrackInterface* local_video) {
  I_LOG("StartLocalRenderer");
  local_renderer_.reset(new VideoRenderer(window,1, 1, local_video, false));
  I_LOG("XXXXXXXXXXXXXX flag = {}", local_renderer_->remoteFlag);
}

void MainWnd::StopLocalRenderer() {
  local_renderer_.reset();
}

void MainWnd::StartRemoteRenderer(webrtc::VideoTrackInterface* remote_video) {
    I_LOG("StartRemoteRenderer");
  remote_renderer_.reset(new VideoRenderer(window,1, 1, remote_video, true));
  I_LOG("XXXXXXXXXXXXXXXXXX flag = {}", remote_renderer_->remoteFlag);
}

void MainWnd::StopRemoteRenderer() {
  remote_renderer_.reset();
}

void MainWnd::QueueUIThreadCallback(int msg_id, void* data) {
  I_LOG("QueueUIThreadCallback");
  ::PostThreadMessage(ui_thread_id_, UI_THREAD_CALLBACK,
                      static_cast<WPARAM>(msg_id),
                      reinterpret_cast<LPARAM>(data));
}

void MainWnd::OnPaint() {
    I_LOG("OnPaint");
}

void MainWnd::OnDestroyed() {
  PostQuitMessage(0);
}

void MainWnd::OnDefaultAction() {
    if (!callback_)
        return;
    if (ui_ == CONNECT_TO_SERVER) {
        I_LOG("please cin serverIp serverPort");
        //std::cin >> server_ >> port_;
        callback_->StartLogin(server_, std::atoi(port_.c_str()));
        I_LOG("loginSuccess {} {}", server_, port_);
    }
    //I_LOG("please cin peerID to connect peer");
    //int name;
    //std::cin >> name;
    //callback_->ConnectToPeer(name);

}

bool MainWnd::OnMessage(UINT msg, WPARAM wp, LPARAM lp, LRESULT* result) {
  switch (msg) {
    case WM_ERASEBKGND:
      *result = TRUE;
      return true;

    //case WM_PAINT:
    //  OnPaint();//视频流的帧渲染及图形界面渲染
    //  return true;

    //case WM_SETFOCUS://编辑框聚焦定位
    //  if (ui_ == CONNECT_TO_SERVER) {
    //    SetFocus(edit1_);
    //  } else if (ui_ == LIST_PEERS) {
    //    SetFocus(listbox_);
    //  }
    //  return true;

    //case WM_SIZE://用来放大和缩小窗口
    //  if (ui_ == CONNECT_TO_SERVER) {
    //    LayoutConnectUI(true);
    //  } else if (ui_ == LIST_PEERS) {
    //    LayoutPeerListUI(true);
    //  }
    //  break;

    //case WM_CTLCOLORSTATIC:
    //  *result = reinterpret_cast<LRESULT>(GetSysColorBrush(COLOR_WINDOW));
    //  return true;

    //case WM_COMMAND://按键点击消息处理
    //  if (button_ == reinterpret_cast<HWND>(lp)) {
    //    if (BN_CLICKED == HIWORD(wp))
    //      OnDefaultAction();
    //  } else if (listbox_ == reinterpret_cast<HWND>(lp)) {
    //    if (LBN_DBLCLK == HIWORD(wp)) {
    //      OnDefaultAction();
    //    }
    //  }
    //  return true;

    case WM_CLOSE:
      if (callback_)
        callback_->Close();
      break;
  }
  return false;
}

// static
LRESULT CALLBACK MainWnd::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
  MainWnd* me =
      reinterpret_cast<MainWnd*>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
  if (!me && WM_CREATE == msg) {
    CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lp);
    me = reinterpret_cast<MainWnd*>(cs->lpCreateParams);
   // me->wnd_ = hwnd;
    ::SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(me));
  }

  LRESULT result = 0;
  if (me) {
    void* prev_nested_msg = me->nested_msg_;
    me->nested_msg_ = &msg;

    bool handled = me->OnMessage(msg, wp, lp, &result);
    if (WM_NCDESTROY == msg) {
      me->destroyed_ = true;
    } else if (!handled) {
      result = ::DefWindowProc(hwnd, msg, wp, lp);
    }

    if (me->destroyed_ && prev_nested_msg == NULL) {
      me->OnDestroyed();
     // me->wnd_ = NULL;
      me->destroyed_ = false;
    }

    me->nested_msg_ = prev_nested_msg;
  } else {
    result = ::DefWindowProc(hwnd, msg, wp, lp);
  }

  return result;
}

// static
bool MainWnd::RegisterWindowClass() {
  if (wnd_class_)// 如果窗口类已经注册，直接返回true
    return true;

  WNDCLASSEXW wcex = {sizeof(WNDCLASSEX)};// 初始化窗口类结构体
  wcex.style = CS_DBLCLKS;// 设置窗口样式，这里允许接收双击消息s
  wcex.hInstance = GetModuleHandle(NULL);// 获取当前进程的实例句柄
  wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
  wcex.hCursor = ::LoadCursor(NULL, IDC_ARROW);// 设置窗口光标样式
  wcex.lpfnWndProc = &WndProc;// 设置窗口消息处理函数
  wcex.lpszClassName = kClassName; // 设置窗口类名
  wnd_class_ = ::RegisterClassExW(&wcex);// 调用RegisterClassExW函数注册窗口类，注册成功会返回一个窗口类的原子类名，失败返回0
  RTC_DCHECK(wnd_class_ != 0);// 检查窗口类是否注册成功
  return wnd_class_ != 0;
}

void MainWnd::CreateChildWindow(HWND* wnd,
                                MainWnd::ChildWindowID id,
                                const wchar_t* class_name,
                                DWORD control_style,
                                DWORD ex_style) {
  if (::IsWindow(*wnd))
    return;

  // Child windows are invisible at first, and shown after being resized.
  //DWORD style = WS_CHILD | control_style;// 子窗口初始为隐藏状态，在调整大小后显示
  //*wnd = ::CreateWindowExW(ex_style, class_name, L"", style, 100, 100, 100, 100, // 创建子窗口，窗口位置和尺寸初始为100*100，实际会在后续调整
  //                         wnd_, reinterpret_cast<HMENU>(id),
  //                         GetModuleHandle(NULL), NULL);
  //RTC_DCHECK(::IsWindow(*wnd) != FALSE);
  //::SendMessage(*wnd, WM_SETFONT, reinterpret_cast<WPARAM>(GetDefaultFont()),
  //              TRUE);
}

void MainWnd::CreateChildWindows() {
  // Create the child windows in tab order.
  //CreateChildWindow(&label1_, LABEL1_ID, L"Static", ES_CENTER | ES_READONLY, 0);
  //CreateChildWindow(&edit1_, EDIT_ID, L"Edit",
  //                  ES_LEFT | ES_NOHIDESEL | WS_TABSTOP, WS_EX_CLIENTEDGE);
  //CreateChildWindow(&label2_, LABEL2_ID, L"Static", ES_CENTER | ES_READONLY, 0);
  //CreateChildWindow(&edit2_, EDIT_ID, L"Edit",
  //                  ES_LEFT | ES_NOHIDESEL | WS_TABSTOP, WS_EX_CLIENTEDGE);
  //CreateChildWindow(&button_, BUTTON_ID, L"Button", BS_CENTER | WS_TABSTOP, 0);

  //CreateChildWindow(&listbox_, LISTBOX_ID, L"ListBox",
  //                  LBS_HASSTRINGS | LBS_NOTIFY, WS_EX_CLIENTEDGE);

  //::SetWindowTextA(edit1_, server_.c_str());
  //::SetWindowTextA(edit2_, port_.c_str());
}

void MainWnd::LayoutConnectUI(bool show) {
  //struct Windows {
  //  HWND wnd;
  //  const wchar_t* text;
  //  size_t width;
  //  size_t height;
  //} windows[] = {
  //    {label1_, L"Server"},  {edit1_, L"XXXyyyYYYgggXXXyyyYYYggg"},
  //    {label2_, L":"},       {edit2_, L"XyXyX"},
  //    {button_, L"Connect"},
  //};

  //if (show) {
  //  const size_t kSeparator = 5;
  //  size_t total_width = (ARRAYSIZE(windows) - 1) * kSeparator;

  //  for (size_t i = 0; i < ARRAYSIZE(windows); ++i) {
  //    CalculateWindowSizeForText(windows[i].wnd, windows[i].text,
  //                               &windows[i].width, &windows[i].height);
  //    total_width += windows[i].width;
  //  }

  //  RECT rc;
  //  ::GetClientRect(wnd_, &rc);
  //  size_t x = (rc.right / 2) - (total_width / 2);
  //  size_t y = rc.bottom / 2;
  //  for (size_t i = 0; i < ARRAYSIZE(windows); ++i) {
  //    size_t top = y - (windows[i].height / 2);
  //    ::MoveWindow(windows[i].wnd, static_cast<int>(x), static_cast<int>(top),
  //                 static_cast<int>(windows[i].width),
  //                 static_cast<int>(windows[i].height), TRUE);
  //    x += kSeparator + windows[i].width;
  //    if (windows[i].text[0] != 'X')
  //      ::SetWindowTextW(windows[i].wnd, windows[i].text);
  //    ::ShowWindow(windows[i].wnd, SW_SHOWNA);
  //  }
  //} else {
  //  for (size_t i = 0; i < ARRAYSIZE(windows); ++i) {
  //    ::ShowWindow(windows[i].wnd, SW_HIDE);
  //  }
  //}
}

void MainWnd::LayoutPeerListUI(bool show) {
  //if (show) {
  //  RECT rc;
  //  ::GetClientRect(wnd_, &rc);
  //  ::MoveWindow(listbox_, 0, 0, rc.right, rc.bottom, TRUE);
  //  ::ShowWindow(listbox_, SW_SHOWNA);
  //} else {
  //  ::ShowWindow(listbox_, SW_HIDE);
  //  InvalidateRect(wnd_, NULL, TRUE);
  //}
}

void MainWnd::HandleTabbing() {
  //bool shift = ((::GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0);
  //UINT next_cmd = shift ? GW_HWNDPREV : GW_HWNDNEXT;
  //UINT loop_around_cmd = shift ? GW_HWNDLAST : GW_HWNDFIRST;
  //HWND focus = GetFocus(), next;
  //do {
  //  next = ::GetWindow(focus, next_cmd);
  //  if (IsWindowVisible(next) &&
  //      (GetWindowLong(next, GWL_STYLE) & WS_TABSTOP)) {
  //    break;
  //  }

  //  if (!next) {
  //    next = ::GetWindow(focus, loop_around_cmd);
  //    if (IsWindowVisible(next) &&
  //        (GetWindowLong(next, GWL_STYLE) & WS_TABSTOP)) {
  //      break;
  //    }
  //  }
  //  focus = next;
  //} while (true);
  //::SetFocus(next);
}

//
// MainWnd::VideoRenderer
//

MainWnd::VideoRenderer::VideoRenderer(
    SDL_Window* window,
    int width,
    int height,
    webrtc::VideoTrackInterface* track_to_render,
    bool remote)
    :window(window),rendered_track_(track_to_render) {
    I_LOG("VideoRenderer");
  ::InitializeCriticalSection(&buffer_lock_);
  ZeroMemory(&bmi_, sizeof(bmi_));
  bmi_.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi_.bmiHeader.biPlanes = 1;
  bmi_.bmiHeader.biBitCount = 32;
  bmi_.bmiHeader.biCompression = BI_RGB;
  bmi_.bmiHeader.biWidth = width;
  bmi_.bmiHeader.biHeight = -height;
  bmi_.bmiHeader.biSizeImage =
      width * height * (bmi_.bmiHeader.biBitCount >> 3);
  if (remote) {
    I_LOG("remote = {}", remote);
    this->remoteFlag = true;
  }
  rendered_track_->AddOrUpdateSink(this, rtc::VideoSinkWants());
}

MainWnd::VideoRenderer::~VideoRenderer() {
    I_LOG("~VideoRenderer");
  rendered_track_->RemoveSink(this);
  ::DeleteCriticalSection(&buffer_lock_);
}

void MainWnd::VideoRenderer::SetSize(int width, int height) {
  AutoLock<VideoRenderer> lock(this);

  if (width == bmi_.bmiHeader.biWidth && height == bmi_.bmiHeader.biHeight) {
    return;
  }
  bmi_.bmiHeader.biWidth = width;
  bmi_.bmiHeader.biHeight = -height;
  bmi_.bmiHeader.biSizeImage =
      width * height * (bmi_.bmiHeader.biBitCount >> 3);
  image_.reset(new uint8_t[bmi_.bmiHeader.biSizeImage]);
}

void MainWnd::VideoRenderer::OnFrame(const webrtc::VideoFrame& video_frame) {
  {
    AutoLock<VideoRenderer> lock(this);

    rtc::scoped_refptr<webrtc::I420BufferInterface> buffer(
        video_frame.video_frame_buffer()->ToI420());
    if (video_frame.rotation() != webrtc::kVideoRotation_0) {
      buffer = webrtc::I420Buffer::Rotate(*buffer, video_frame.rotation());
    }

    SetSize(buffer->width(), buffer->height());
    D_LOG("image w {} h {}", buffer->width(), buffer->height());

    RTC_DCHECK(image_.get() != NULL);
    //libyuv::I420ToARGB(buffer->DataY(), buffer->StrideY(), buffer->DataU(),
    //                   buffer->StrideU(), buffer->DataV(), buffer->StrideV(),
    //                   image_.get(),
    //                   bmi_.bmiHeader.biWidth * bmi_.bmiHeader.biBitCount / 8,
    //                   buffer->width(), buffer->height());
    if (video_width !=buffer->width()) {
        I_LOG("init render");
        if (texture) {
            SDL_DestroyTexture(texture);
        }
        if (renderer) {
            SDL_DestroyRenderer(renderer);
        }

        if (texture_remote) {
          SDL_DestroyTexture(texture_remote);
        }
        if (renderer_remote) {
          SDL_DestroyRenderer(renderer_remote);
        }

        video_width = buffer->width();
        video_height = buffer->height();
        renderer = SDL_CreateRenderer(window, -1, 0); // 创建基于窗口的渲染器
        texture = SDL_CreateTexture(renderer, pixformat, SDL_TEXTUREACCESS_STREAMING, video_width, video_height);
        renderer_remote = SDL_CreateRenderer(window, -1, 0); // 创建基于窗口的渲染器
        texture_remote = SDL_CreateTexture(renderer_remote, pixformat, SDL_TEXTUREACCESS_STREAMING, video_width, video_height);
    }
    rect.w = video_width;
    rect.h = video_height;
    //I_LOG("rect.w = {}, rect.h = {}", rect.w, rect.h);
    //if (this->remoteFlag) {
      //rect.x = 0;
      //rect.y = video_height;
      //SDL_UpdateTexture(texture_remote, NULL, buffer->DataY(), video_width);
      ////SDL_RenderClear(renderer);
      //SDL_RenderCopy(renderer_remote, texture_remote, NULL, &rect);
      //SDL_RenderPresent(renderer_remote);
    //}
    //else{
    
    D_LOG("remoteFlag = {} {} {}", remoteFlag, this->remoteFlag, buffer->DataY()[0]);
    //if (this->remoteFlag) {
      rect.x = 0;
      rect.y = 0;
      SDL_UpdateTexture(texture, NULL, buffer->DataY(), video_width);
      SDL_RenderClear(renderer);
      SDL_RenderCopy(renderer, texture, NULL, &rect);
      SDL_RenderPresent(renderer);
    //}
    //}
    // 更新纹理数据


    ////保存rgba
    //cv::Mat mat(buffer->height(), buffer->width(), CV_8UC4);
    //mat.data = image_.get();
    //i++;
    //cv::imwrite(std::to_string(i)+".png", mat);
  }
  //InvalidateRect(wnd_, NULL, TRUE);
}
