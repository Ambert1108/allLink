#include "screen/screensink.h"
#include "api/video/i420_buffer.h"
#include "rtc_base/arraysize.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "third_party/libyuv/include/libyuv/convert_argb.h"
#include "libyuv.h"
#include "libyuv/scale.h"
#include "libyuv/scale_argb.h"

namespace alllink {
  float scaleRatio = 0.0f;

  static inline std::string parseTime(int64_t timestamp) {
    timestamp *= 0.001;
    int64_t hour = timestamp / 3600;
    int64_t min = timestamp / 60 - hour * (int64_t)60;
    int64_t sec = timestamp - hour * (int64_t)3600 - min * (int64_t)60;
    std::string s = std::to_string(hour) + ":" + std::to_string(min) + ":" + std::to_string(sec) + "";
    return s;
  }

	StreamScreen::StreamScreen(sf::VideoMode mode, const sf::String& title, sf::Image icon, sf::Uint32 style)
		: BaseScreen(mode, title, icon, style, sf::ContextSettings()) {
		wr = static_cast<float>(mode.width) / 1920;
		hr = static_cast<float>(mode.height) / 1080;
    wndPosition = this->getPosition();
		this->icon_ = icon;
		this->setFramerateLimit(60);
		this->setVisible(false);
    auto maxSize = sf::Texture::getMaximumSize();
    scaleRatio = 2048.0 / maxSize;
    I_LOG("device driver support maximum is {}x{}, ratio is {}", maxSize, maxSize, scaleRatio);
	}

	StreamScreen::~StreamScreen() {}

	bool StreamScreen::OnEnter() {
		// 设置界面可见
		this->setVisible(true);
    this->setPosition(wndPosition);
    timePoint = seeker::time::currentTime();
		isActive = true;
		return true;
	}

	bool StreamScreen::OnExit() {
		// 设置界面不可见
		this->setVisible(false);
		isActive = false;
    camState = false;
    micState = false;
    shareState = false;
    timePoint = 0;
		return true;
	}

	int StreamScreen::init() {
    // 初始化本地及远端画面纹理
    remoteSrc = new sf::Texture();
    localSrc = new sf::Texture();

    // 初始化本地及远端精灵尺寸，其中远端精灵尺寸需要适配不同分辨率屏幕
    remoteVideo.init(1920 * wr, 1080 * hr, 0, 0);
    localVideo.init(640, 480, 1180, 590);


    closeMic = std::make_unique<VariableStateGraphicModule>();
    openMic = std::make_unique<VariableStateGraphicModule>();
    closeCam = std::make_unique<VariableStateGraphicModule>();
    openCam = std::make_unique<VariableStateGraphicModule>();
    closeShare = std::make_unique<VariableStateGraphicModule>();
    openShare = std::make_unique<VariableStateGraphicModule>();

    closeMic->init(80 * wr, 50 * hr, 20 * wr, 1020 * hr);
    closeMic->setTexture(closeMicFile);
    closeMic->setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
    closeMic->setImageSize(40 * wr, 40 * wr);
    closeMic->setImageColor(sf::Color::Black);

    openMic->init(80 * wr, 50 * hr, 20 * wr, 1020 * hr);
    openMic->setTexture(openMicFile);
    openMic->setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
    openMic->setImageSize(40 * wr, 40 * wr);
    openMic->setImageColor(sf::Color(74, 224, 84));

    closeCam->init(80 * wr, 50 * hr, 140 * wr, 1020 * hr);
    closeCam->setTexture(closeCamFile);
    closeCam->setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
    closeCam->setImageSize(40 * wr, 40 * wr);
    closeCam->setImageColor(sf::Color::Black);

    openCam->init(80 * wr, 50 * hr, 140 * wr, 1020 * hr);
    openCam->setTexture(openCamFile);
    openCam->setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
    openCam->setImageSize(40 * wr, 40 * wr);
    openCam->setImageColor(sf::Color(117, 188, 255));

    closeShare->init(80 * wr, 50 * hr, 280 * wr, 1020 * hr);
    closeShare->setTexture(closeShareFile);
    closeShare->setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
    closeShare->setImageSize(40 * wr, 40 * wr);
    closeShare->setImageColor(sf::Color::Black);

    openShare->init(80 * wr, 50 * hr, 280 * wr, 1020 * hr);
    openShare->setTexture(openShareFile);
    openShare->setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
    openShare->setImageSize(40 * wr, 40 * wr);
    openShare->setImageColor(sf::Color(242, 80, 125));

    meetingTime = std::make_unique<HorizonGraphicTextsModule>(false);
    meetingTime->init(240 * wr, 16 * hr, 6 * wr, 12 * hr);
    meetingTime->setSource(18 * hr, msyhFile, meetingTimeFile);
    meetingTime->setColor(sf::Color(255, 255, 255, 0), sf::Color(235, 235, 235, 200), sf::Color(225, 225, 225, 200));
    meetingTime->setText(L"会议时长 0:0:0", sf::Color(0, 0, 0));
    meetingTime->setImageSize(24 * wr, 24 * wr);
    meetingTime->setImageColor(sf::Color(117, 188, 255));

    leaveMeeting = std::make_unique<TextFillRectangle>(sf::Color::Red, 3 * wr);
    leaveMeeting->init(190 * wr, 50 * hr, 1686 * wr, 1015 * hr, 8.0 * wr);
    leaveMeeting->setColor(sf::Color::White, sf::Color::Red, sf::Color(191, 23, 23));
    leaveMeeting->setText(msyhFile, L"离开会议", sf::Color::Black, sf::Color::White);

    meetingDescribe = std::make_unique<BaseText>();
    meetingDescribe->init(msyhFile);
    meetingDescribe->setCharacterSize(18 * hr);
    meetingDescribe->setString(L"会议号 unknown");
    meetingDescribe->setFillColor(sf::Color::Black);
    meetingDescribe->setPosition(((1920 - meetingDescribe->getGlobalBounds().width) / 2) * wr, 11 * hr);

    bottom.setPosition(0 * wr, 1000 * hr);
    bottom.setSize(sf::Vector2f(1920 * wr, 80 * hr));
    bottom.setFillColor(sf::Color(255, 255, 255));

    top.setPosition(0, 0);
    top.setSize(sf::Vector2f(1920 * wr, 40 * hr));
    top.setFillColor(sf::Color(255, 255, 255));

    // 设置窗口大小为等比例720p
    this->setSize(sf::Vector2u(1280 * wr, 720 * hr));

    // 获取屏幕的分辨率
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    unsigned int screenWidth = desktop.width;
    unsigned int screenHeight = desktop.height;

    // 计算窗口初始化位置并进行设置
    int posX = (screenWidth - this->getSize().x) / 2;
    int posY = (screenHeight - this->getSize().y) / 2;
    wndPosition = sf::Vector2i(posX, posY);
    this->setPosition(wndPosition);

		return 0;
	}

	void StreamScreen::show() {
		if (!isActive) return;
		this->clear(sf::Color(242, 242, 242));
    ImageData remoteData, localData;
    if (remoteImageList.WaitPopFlex(remoteData)) {
      int remoteHeight = abs(remoteData.bmi.bmiHeader.biHeight);
      int remoteWidth = remoteData.bmi.bmiHeader.biWidth;
      bool reset = false;
      if (remoteSrc->getSize().x != remoteWidth
        || remoteSrc->getSize().y != remoteHeight) {
        I_LOG("remote size is {}:{}, raw size is {}:{}", remoteSrc->getSize().x, remoteSrc->getSize().y,
          remoteWidth, remoteHeight);
        remoteSrc->create(remoteWidth, remoteHeight);
        int remoteX = (this->getSize().x - remoteWidth) / 2;
        int remoteY = (this->getSize().y - remoteHeight) / 2;
        //remoteVideo.setPosition(sf::Vector2f(remoteX, remoteY));
        reset = true;
      }
      remoteSrc->update(remoteData.image.get());
      remoteVideo.setVideo(*remoteSrc);
      remoteVideo.render(this);
    }
    if (localImageList.TryPopFlex(localData)) {
      int rawWidth = localData.bmi.bmiHeader.biWidth;
      int rawHeight = abs(localData.bmi.bmiHeader.biHeight);
      int localWidth = rawWidth / 4;
      int localHeight = rawHeight / 4;

      bool reset = false;
      if (localSrc->getSize().x != rawWidth || localSrc->getSize().y != rawHeight) {
        I_LOG("local size is {}:{}, raw size is {}:{}", localSrc->getSize().x, localSrc->getSize().y,
          rawWidth, rawHeight);
        localSrc->create(rawWidth, rawHeight);
        int lcoalX = this->getSize().x - localWidth - 10;
        int lcoalY = this->getSize().y - localHeight - 10;
        //localVideo.setPosition(sf::Vector2f(lcoalX, lcoalY));
        reset = true;
      }
      localSrc->update(localData.image.get());
      localVideo.setVideo(*localSrc);
      localVideo.setScale(0.25f, 0.25f);
      localVideo.render(this);
    }
    if (!isFull) {
      this->draw(top);
      this->draw(bottom);
      leaveMeeting->render(this);
      if (micState) openMic->render(this);
      else closeMic->render(this);

      if (camState) openCam->render(this);
      else closeCam->render(this);

      if (shareState) openShare->render(this);
      else closeShare->render(this);
      meetingTime->render(this);
      this->draw(*meetingDescribe.get());
    }
    this->display();
	}

	void StreamScreen::eventProcess() {
    if (!isActive) return;
    while (this->pollEvent(event)) {
      switch (event.type) {
      case sf::Event::KeyPressed:
        if (event.key.code == sf::Keyboard::F) {
          isMirror.store(!isMirror);
        }
        else if (event.key.code == sf::Keyboard::M) {
          micState = !micState;
          hi::PostMsg({ msgTo(MessageType::SET_MIC_PHONE), micState });
        }
        else if (event.key.code == sf::Keyboard::V) {
          camState = !camState;
          hi::PostMsg({ msgTo(MessageType::SET_CAMERA), camState });
        }
        else if (event.key.code >= sf::Keyboard::Num0 && event.key.code <= sf::Keyboard::Num9) {
          int num = event.key.code - sf::Keyboard::Num0;
          if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            hi::PostMsg({ msgTo(MessageType::SWITCH_AUDIO_INPUT), num });
          }
        }
        else if (event.key.code == sf::Keyboard::Up) {
          if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) && micVolume < 100) {
            micVolume += 10;
            hi::PostMsg({ msgTo(MessageType::SWITCH_MIC_VOLUME), micVolume });
          }
        }
        else if (event.key.code == sf::Keyboard::Down) {
          if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) && micVolume > 0) {
            micVolume -= 10;
            hi::PostMsg({ msgTo(MessageType::SWITCH_MIC_VOLUME), micVolume });
          }
        }
        break;
      case sf::Event::Closed:
        // 通知视觉控制器会议画面被关闭
        hi::PostMsg({ msgTo(MessageType::MEETING_END), nullptr });
      }
      sf::Vector2i mousePosWin = sf::Mouse::getPosition(*this);
      // 检查鼠标是否在窗口内
      if (mousePosWin.x >= 0 && mousePosWin.x < this->getSize().x &&
        mousePosWin.y >= 0 && mousePosWin.y < this->getSize().y) {
        sf::Vector2f mousePosView = this->mapPixelToCoords(mousePosWin);
        if (leaveMeeting->onClick(event, mousePosView, this)) {
          I_LOG("leave meeting");
          hi::PostMsg({ msgTo(MessageType::MEETING_END), nullptr });
        }
        if (!micState) {
          if (closeMic->onClick(event, mousePosView, this)) {
            I_LOG("open microphone");
            micState = !micState;
            hi::PostMsg({ msgTo(MessageType::SET_MIC_PHONE), micState });
          }
        }
        else {
          if (openMic->onClick(event, mousePosView, this)) {
            I_LOG("close microphone");
            micState = !micState;
            hi::PostMsg({ msgTo(MessageType::SET_MIC_PHONE), micState });
          }
        }
        if (!camState) {
          if (closeCam->onClick(event, mousePosView, this)) {
            I_LOG("open camera");
            camState = !camState;
            hi::PostMsg({ msgTo(MessageType::SET_CAMERA), camState });
          }
        }
        else {
          if (openCam->onClick(event, mousePosView, this)) {
            I_LOG("close camera");
            camState = !camState;
            hi::PostMsg({ msgTo(MessageType::SET_CAMERA), camState });
          }
        }
        if (!shareState) {
          if (closeShare->onClick(event, mousePosView, this)) {
            I_LOG("open share");
            shareState = !shareState;
          }
        }
        else {
          if (openShare->onClick(event, mousePosView, this)) {
            I_LOG("close share");
            shareState = !shareState;
          }
        }
        isFull = false;
      }
      else isFull = true;
    }
    std::wstring time = L"会议时长 " +
      WstrConv.from_bytes(parseTime(seeker::time::currentTime() - timePoint));
    meetingTime->setText(time, sf::Color(0, 0, 0));
	}

  void StreamScreen::startLocalRenderer(webrtc::VideoTrackInterface* local_video) {
    local_renderer_.reset(new VideoRenderer(std::bind(&StreamScreen::OnPaint, this), 1, 1, local_video));
    I_LOG("local render reset");
  }

  void StreamScreen::stopLocalRenderer() {
    local_renderer_.reset();
    I_LOG("local render stop");
  }

  void StreamScreen::startRemoteRenderer(webrtc::VideoTrackInterface* remote_video) {
    remote_renderer_.reset(new VideoRenderer(std::bind(&StreamScreen::OnPaint, this), 1, 1, remote_video));
    I_LOG("remote render reset");
  }

  void StreamScreen::stopRemoteRenderer() {
    remote_renderer_.reset();
    I_LOG("remote render stop");
  }

  void StreamScreen::setSessionMode(int mode) {
    this->mode = mode;
  }

  void StreamScreen::setSessionId(std::string id) {
    std::wstring s = L"会议号 " + WstrConv.from_bytes(id);
    meetingDescribe->setString(s);
  }

  // 远端流收到视频帧和本地捕捉到视频帧都会调用此函数
  void StreamScreen::OnPaint() {
    //获取本地和远端的视频画面
    VideoRenderer* local_renderer = local_renderer_.get();
    VideoRenderer* remote_renderer = remote_renderer_.get();
    if (isActive && remote_renderer && local_renderer) {
      AutoLock<VideoRenderer> local_lock(local_renderer);
      AutoLock<VideoRenderer> remote_lock(remote_renderer);
      const BITMAPINFO& bmi = remote_renderer->bmi();
      const uint8_t* image = remote_renderer->image();

      if (image != NULL) {
        remoteImageList.Push(ImageData(bmi, image));
        if (this->getSize().x > 200 && this->getSize().y > 200 && mode == 0) {
          const BITMAPINFO& lbmi = local_renderer->bmi();
          const uint8_t* limage = local_renderer->image();
          if (limage == nullptr) {
            I_LOG("local image is nullptr");
            return;
          }
          else if (lbmi.bmiHeader.biSizeImage <= 0) {
            I_LOG("error, image size is {}", lbmi.bmiHeader.biSizeImage);
            return;
          }
          if (isMirror.load()) {
            ImageData data;
            data.bmi = lbmi;
            data.image.reset(new uint8_t[lbmi.bmiHeader.biSizeImage]);
            libyuv::ARGBMirror(limage, lbmi.bmiHeader.biWidth * lbmi.bmiHeader.biBitCount / 8,
              data.image.get(), lbmi.bmiHeader.biWidth * lbmi.bmiHeader.biBitCount / 8,
              lbmi.bmiHeader.biWidth, std::abs(lbmi.bmiHeader.biHeight));
            localImageList.Push(data);
          }
          else {
            localImageList.Push(ImageData(lbmi, limage));
          }
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

  StreamScreen::VideoRenderer::VideoRenderer(
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

  StreamScreen::VideoRenderer::~VideoRenderer() {
    rendered_track_->RemoveSink(this);
    ::DeleteCriticalSection(&buffer_lock_);
  }

  void StreamScreen::VideoRenderer::SetSize(int width, int height) {
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

  void StreamScreen::VideoRenderer::OnFrame(const webrtc::VideoFrame& video_frame) {
    {
      AutoLock<VideoRenderer> lock(this);

      /*将获取的视频帧，转成YUV420格式*/
      rtc::scoped_refptr<webrtc::I420BufferInterface> buffer(
        video_frame.video_frame_buffer()->ToI420());
      if (video_frame.rotation() != webrtc::kVideoRotation_0) {
        buffer = webrtc::I420Buffer::Rotate(*buffer, video_frame.rotation());
      }

      RTC_DCHECK(image_.get() != NULL);

      if (scaleRatio > 1) {
        SetSize(buffer->width() / scaleRatio, buffer->height() / scaleRatio);

        uint8_t* src = new uint8_t[bmi_.bmiHeader.biSizeImage];
        uint8_t* ydata = new uint8_t[bmi_.bmiHeader.biSizeImage];
        uint8_t* udata = new uint8_t[bmi_.bmiHeader.biSizeImage];
        uint8_t* vdata = new uint8_t[bmi_.bmiHeader.biSizeImage];

        int stride_y = buffer->width() / scaleRatio;
        int stride_u = (buffer->width() / 2.0 + 1) / scaleRatio;
        int stride_v = (buffer->width() / 2.0 + 1) / scaleRatio;

        //缩放
        libyuv::Scale(buffer->DataY(), buffer->DataU(), buffer->DataV(),
          buffer->StrideY(), buffer->StrideU(), buffer->StrideV(),
          buffer->width(), buffer->height(),
          ydata, udata, vdata, stride_y, stride_u, stride_v,
          buffer->width() / scaleRatio, buffer->height() / scaleRatio, libyuv::kFilterBilinear);

        /*将YUV420格式的图像，转成RGB格式的图像。*/
        libyuv::I420ToABGR(ydata, stride_y, udata,
          stride_u, vdata, stride_v,
          image_.get(),
          bmi_.bmiHeader.biWidth * bmi_.bmiHeader.biBitCount / 8,
          buffer->width() / scaleRatio, buffer->height() / scaleRatio);

        /*将图像镜像反转*/
        //libyuv::ARGBMirror(src, bmi_.bmiHeader.biWidth * bmi_.bmiHeader.biBitCount / 8,
        //  image_.get(), bmi_.bmiHeader.biWidth * bmi_.bmiHeader.biBitCount / 8,
        //  buffer->width() / scaleRatio, buffer->height() / scaleRatio);

        delete ydata;
        delete udata;
        delete vdata;
        delete src;
      }
      else {
        SetSize(buffer->width(), buffer->height());

        uint8_t* src = new uint8_t[bmi_.bmiHeader.biSizeImage];
        /*将YUV420格式的图像，转成RGB格式的图像。*/
        libyuv::I420ToABGR(buffer->DataY(), buffer->StrideY(), buffer->DataU(),
          buffer->StrideU(), buffer->DataV(), buffer->StrideV(),
          image_.get(),
          bmi_.bmiHeader.biWidth * bmi_.bmiHeader.biBitCount / 8,
          buffer->width(), buffer->height());

        /*将图像镜像反转*/
        //libyuv::ARGBMirror(src, bmi_.bmiHeader.biWidth * bmi_.bmiHeader.biBitCount / 8,
        //  image_.get(), bmi_.bmiHeader.biWidth * bmi_.bmiHeader.biBitCount / 8,
        //  buffer->width(), buffer->height());

        delete src;
      }
    }

    /*触发渲染*/
    paint();
  }
}