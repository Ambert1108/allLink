#include "presenter.h"

#include <iostream>
#include <cctype>
#include <math.h>
#include <conio.h>

#include "api/video/i420_buffer.h"
#include "defaults.h"
#include "rtc_base/arraysize.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "third_party/libyuv/include/libyuv/convert_argb.h"
#include "libyuv.h"

namespace {
  const char kConnecting[] = "Connecting... ";
  const char kNoVideoStreams[] = "(no video streams either way)";
  const char kNoIncomingStream[] = "(no incoming video)";

  void clearConsole() {
    // 获取控制台句柄
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // 获取控制台屏幕缓冲区的大小
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);

    // 计算缓冲区的大小
    DWORD bufferSize = csbi.dwSize.X * csbi.dwSize.Y;

    // 填充缓冲区
    DWORD charsWritten;
    FillConsoleOutputCharacter(hConsole, ' ', bufferSize, { 0, 0 }, &charsWritten);

    // 重置光标到左上角
    SetConsoleCursorPosition(hConsole, { 0, 0 });
  }

  void printFixMsg() { std::cout << "Welcome use 1v1 media client (input 'q' exit)\n\tpower by WebRTC\n\n"; }

  bool isNumber(const std::string& str) {
    return !str.empty() && std::all_of(str.begin(), str.end(), [](char c) {
      return std::isdigit(static_cast<unsigned char>(c));
      });
  }

}  // namespace

namespace wt {
const wchar_t MainWnd::kClassName[] = L"WebRTC_MainWnd";

  MainWnd::MainWnd(const char* server, int port)
    : ui_(CONNECT_TO_SERVER),
    callback_(NULL),
    server_(server) {
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%i", port);
    port_ = buffer;
  }

  MainWnd::~MainWnd() {
    RTC_DCHECK(!IsWindow());
  }

  bool MainWnd::Create() {
    createWindow();
    SwitchToConnectUI();
    return true;
  }

  bool MainWnd::Destroy() {
    BOOL ret = FALSE;
    if (wnd) delete wnd;
    if (localSrc) delete localSrc;
    if (remoteSrc) delete remoteSrc;

    return ret != FALSE;
  }

  void MainWnd::RegisterObserver(MainWndCallback* callback) {
    callback_ = callback;
  }

  bool MainWnd::IsWindow() {
    return wnd && wnd->isOpen();
  }

  bool MainWnd::PreTranslateMessage() {
    if (ui_ == CONNECT_TO_SERVER) {
      std::string input;
      D_LOG("input ip(use enter skip):");
      std::cout << "input ip(use enter skip):";
      std::getline(std::cin, input); 
      if (isExist(input)) return false;
      if (!input.empty()) server_ = input;
      std::cout << server_ << std::endl;
      D_LOG("input port(use enter skip):");
      std::cout << "input port(use enter skip):";
      std::getline(std::cin, input);
      if (isExist(input)) return false;
      if (!input.empty()) port_ = input;
      std::cout << port_ << std::endl;
      OnDefaultAction();
    }
    else {
      OnDefaultAction();
    }

    return true;
  }

  bool MainWnd::PreTranslateMessage(Message msg) {
    /*处理自定义类型的消息*/
    D_LOG("pre translate msg");
    callback_->UIThreadCallback(msg.id, msg.data);
    return true;
  }

  bool MainWnd::isExist(const std::string& msg) {
    // 检查用户是否输入 'q' 来退出
    if (msg == "q" || msg == "Q") {
      std::cout << "you want to exit? type(y/n)" << std::endl;
      char inputChar;
      std::cin >> inputChar;
      if (inputChar == 'y' || inputChar == 'Y') {
        if (ui_ == STREAMING) {
          callback_->DisconnectFromCurrentPeer();
        }
        else if(ui_ == LIST_PEERS) {
          callback_->DisconnectFromServer();
        }
        return true;
      }
    }
    return false;
  }

  void MainWnd::SwitchToConnectUI() {
    clearConsole();
    printFixMsg();
    D_LOG("input ip:port or use default({}:{})to start", server_, port_);
    std::cout << "input ip:port or use default(" << server_ << ":" << port_ << ")to start" << std::endl;
    ui_ = CONNECT_TO_SERVER;
  }

  void MainWnd::SwitchToPeerList(const Peers& peers) {
    if (ui_ == STREAMING) {
      //被动关闭
      //清空窗口画面并隐藏
      wnd->clear();
      if (wndVisible) {
        wndVisible = false;
        wnd->setVisible(false);
      }
    }

    clearConsole();
    printFixMsg();
    Peers::const_iterator i = peers.begin();
    D_LOG("Current Peer List: ");
    std::cout << "Current Peer List: " << std::endl;
    for (; i != peers.end(); ++i) {
      D_LOG("{}:{}", i->first, i->second);
      std::cout << i->first << ": " << i->second << std::endl;
    }
    ui_ = LIST_PEERS;
  }

  void MainWnd::SwitchToStreamingUI(bool isRecv) {
    clearConsole();
    printFixMsg();
    D_LOG("switch to streaming ui");
    D_LOG("Calling ...");
    if (isRecv) std::cout << "Receive Call, use ENTER accept";
    else std::cout << "Calling ..." << std::endl;
    ui_ = STREAMING;
    D_LOG("switch streaming finish");
  }

  void MainWnd::StartLocalRenderer(webrtc::VideoTrackInterface* local_video) {
    local_renderer_.reset(new VideoRenderer(std::bind(&MainWnd::OnPaint, this), 1, 1, local_video));
    D_LOG("start local render");
  }

  void MainWnd::StopLocalRenderer() {
    local_renderer_.reset();
  }

  void MainWnd::StartRemoteRenderer(webrtc::VideoTrackInterface* remote_video) {
    remote_renderer_.reset(new VideoRenderer(std::bind(&MainWnd::OnPaint, this), 1, 1, remote_video));
    D_LOG("start remote render");
  }

  void MainWnd::StopRemoteRenderer() {
    remote_renderer_.reset();
  }

  void MainWnd::QueueUIThreadCallback(int msg_id, void* data) {
    hi::PostMsg(Message{ msg_id, data });
  }

  void MainWnd::OnPaint() {
    //获取本地和远端的视频画面
    VideoRenderer* local_renderer = local_renderer_.get();
    VideoRenderer* remote_renderer = remote_renderer_.get();
    if (ui_ == STREAMING && remote_renderer && local_renderer) {
      AutoLock<VideoRenderer> local_lock(local_renderer);
      AutoLock<VideoRenderer> remote_lock(remote_renderer);
      const BITMAPINFO& bmi = remote_renderer->bmi();
      const uint8_t* image = remote_renderer->image();

      if (image != NULL) {
        remoteImageList.Push(ImageData(bmi, image));
        if (wndWidth > 200 && wndHeight > 200) {
          const BITMAPINFO& lbmi = local_renderer->bmi();
          const uint8_t* limage = local_renderer->image();
          if (isMirror.load()) {
            ImageData data;
            data.bmi = lbmi;
            data.image.reset(new uint8_t[lbmi.bmiHeader.biSizeImage]);
            libyuv::ARGBMirror(limage, lbmi.bmiHeader.biWidth * lbmi.bmiHeader.biBitCount / 8,
              data.image.get(), lbmi.bmiHeader.biWidth * lbmi.bmiHeader.biBitCount / 8,
              lbmi.bmiHeader.biWidth, std::abs(lbmi.bmiHeader.biHeight));
            localImageList.Push(data);
          }
          else localImageList.Push(ImageData(lbmi, limage));
        }
      }
      else {
        //TODO：没有收到远端画面，渲染文字
        // Connecting...
        // 若没有本地流：(no video streams either way)
        // 有本地流：(no incoming video)
      }
    }
    else {
      //if(ui_ != STREAMING) std::cout << "ui status failed" << std::endl;
      //if(!remote_renderer) std::cout << "remote_renderer failed" << std::endl;
      //if(!local_renderer) std::cout << "local_renderer failed" << std::endl;
      //TODO：不处于渲染状态，显示黑屏
    }
  }

  void MainWnd::OnDestroyed() {
    PostQuitMessage(0);
  }

  void MainWnd::OnDefaultAction() {
    if (!callback_)
      return;
    if (ui_ == CONNECT_TO_SERVER) {
      std::string server(server_);
      std::string port_str(port_);
      int port = port_str.length() ? atoi(port_str.c_str()) : 0;
      D_LOG("start login");
      std::cout << "start login" << std::endl;
      callback_->StartLogin(server, port);
    }
    else if (ui_ == LIST_PEERS) {
      std::string id;
      D_LOG("please input peer id：");
      std::cout << "please input peer id：";
      std::getline(std::cin, id);
      if (!isNumber(id)) {
        isExist(id);
        return;
      }
      int peerId_ = std::atoi(id.c_str());
      D_LOG("is connecting {} ...", peerId_);
      std::cout << "is connecting " << peerId_ << "..." << std::endl;
      callback_->ConnectToPeer(peerId_);
    }
    else {
      D_LOG("wnd stream render loop");
      if (wnd->isOpen()) {
        if (!wndVisible) {
          wndVisible = true;
          wnd->setVisible(true);
        }
        wnd->clear(sf::Color(243, 243, 243));
        if (wnd->pollEvent(event)) {
          switch (event.type) {
          case sf::Event::KeyPressed:
            if (event.key.code == sf::Keyboard::Escape) {
              if (callback_) callback_->DisconnectFromCurrentPeer();
            }
            else if (event.key.code == sf::Keyboard::M) {
              isMirror.store(!isMirror);
            }
            break;
          case sf::Event::Closed:
            if (callback_) callback_->DisconnectFromCurrentPeer();
          }
        }

        ImageData remoteData, localData;
        if (remoteImageList.WaitPopFlex(remoteData)) {
          int remoteHeight = abs(remoteData.bmi.bmiHeader.biHeight);
          int remoteWidth = remoteData.bmi.bmiHeader.biWidth;
          bool reset = false;
          if (remoteSrc->getSize().x != remoteWidth
            || remoteSrc->getSize().y != remoteHeight) {
            D_LOG("remote size is {}:{}, raw size is {}:{}", remoteSrc->getSize().x, remoteSrc->getSize().y,
              remoteWidth, remoteHeight);
            remoteSrc->create(remoteWidth, remoteHeight);
            reset = true;
          }
          remoteSrc->update(remoteData.image.get());
          remoteVideo.setTexture(*remoteSrc, reset);
          int remoteX = (wndWidth - remoteWidth) / 2;
          int remoteY = (wndHeight - remoteHeight) / 2;
          remoteVideo.setPosition(sf::Vector2f(remoteX, remoteY));
          wnd->draw(remoteVideo);
        }
        if (localImageList.TryPopFlex(localData)) {
          int rawWidth = localData.bmi.bmiHeader.biWidth;
          int rawHeight = abs(localData.bmi.bmiHeader.biHeight);
          int localWidth = rawWidth / 4;
          int localHeight = rawHeight / 4;

          bool reset = false;
          if (localSrc->getSize().x != rawWidth || localSrc->getSize().y != rawHeight) {
            D_LOG("local size is {}:{}, raw size is {}:{}", localSrc->getSize().x, localSrc->getSize().y,
              rawWidth, rawHeight);
            localSrc->create(rawWidth, rawHeight);
            reset = true;
          }
          localSrc->update(localData.image.get());
          localVideo.setTexture(*localSrc, reset);
          localVideo.setScale(0.25f, 0.25f);
          int lcoalX = wndWidth - localWidth - 10;
          int lcoalY = wndHeight - localHeight - 10;
          localVideo.setPosition(sf::Vector2f(lcoalX, lcoalY));
          wnd->draw(localVideo);
        }

        wnd->display();
      }
    }
  }

  void MainWnd::createWindow() {
    wnd = new sf::RenderWindow(
      sf::VideoMode(sf::VideoMode::getDesktopMode().width / 2, 
      sf::VideoMode::getDesktopMode().height / 2), 
      "client", 
      sf::Style::Close);
    wnd->setVerticalSyncEnabled(false);
    wnd->setFramerateLimit(60);
    wnd->setVisible(false);
    wndWidth = wnd->getSize().x;
    wndHeight = wnd->getSize().y;

    localSrc = new sf::Texture();
    remoteSrc = new sf::Texture();
  }

  MainWnd::VideoRenderer::VideoRenderer(
    std::function<void()> callback,
    int width,
    int height,
    webrtc::VideoTrackInterface* track_to_render)
    : paint(callback), rendered_track_(track_to_render) {
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
    /*向VideoTrack订阅视频帧*/
    rendered_track_->AddOrUpdateSink(this, rtc::VideoSinkWants());
  }

  MainWnd::VideoRenderer::~VideoRenderer() {
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

      /*将获取的视频帧，转成YUV420格式*/
      rtc::scoped_refptr<webrtc::I420BufferInterface> buffer(
        video_frame.video_frame_buffer()->ToI420());
      if (video_frame.rotation() != webrtc::kVideoRotation_0) {
        buffer = webrtc::I420Buffer::Rotate(*buffer, video_frame.rotation());
      }

      SetSize(buffer->width(), buffer->height());

      uint8_t* src = new uint8_t[bmi_.bmiHeader.biSizeImage];

      RTC_DCHECK(image_.get() != NULL);
      /*将YUV420格式的图像，转成RGB格式的图像。*/
      libyuv::I420ToABGR(buffer->DataY(), buffer->StrideY(), buffer->DataU(),
        buffer->StrideU(), buffer->DataV(), buffer->StrideV(),
        src,
        bmi_.bmiHeader.biWidth * bmi_.bmiHeader.biBitCount / 8,
        buffer->width(), buffer->height());

      /*将图像镜像反转*/
      libyuv::ARGBMirror(src, bmi_.bmiHeader.biWidth * bmi_.bmiHeader.biBitCount / 8,
        image_.get(), bmi_.bmiHeader.biWidth * bmi_.bmiHeader.biBitCount / 8,
        buffer->width(), buffer->height());
      delete src;
    }

    /*触发渲染*/
    paint();
  }
}