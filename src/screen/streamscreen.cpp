#include "screen/screensink.h"
#include "api/video/i420_buffer.h"
#include "rtc_base/arraysize.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "third_party/libyuv/include/libyuv/convert_argb.h"
#include "libyuv.h"

namespace alllink {
	StreamScreen::StreamScreen(sf::VideoMode mode, const sf::String& title, sf::Image icon, sf::Uint32 style)
		: BaseScreen(mode, title, icon, style, sf::ContextSettings()) {
		wr = static_cast<float>(mode.width) / 1920;
		hr = static_cast<float>(mode.height) / 1080;
    wndPosition = this->getPosition();
		this->icon_ = icon;
		this->setFramerateLimit(60);
		this->setVisible(false);
	}

	StreamScreen::~StreamScreen() {}

	bool StreamScreen::OnEnter() {
		// 设置界面可见
		this->setVisible(true);
    this->setPosition(wndPosition);
		isActive = true;
		return true;
	}

	bool StreamScreen::OnExit() {
		// 设置界面不可见
		this->setVisible(false);
		isActive = false;
		return true;
	}

	int StreamScreen::init() {
    remoteSrc = new sf::Texture();
    localSrc = new sf::Texture();
    remoteVideo.init(1920, 1080, 0, 0);
    localVideo.init(640, 480, 1180, 590);
    this->setSize(sf::Vector2u(1280, 720));
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
        break;
      case sf::Event::Closed:
        // 通知视觉控制器会议画面被关闭
        hi::PostMsg({ msgTo(MessageType::MEETING_END), nullptr });
      }
    }
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

  // ??远端流收到视频帧和本地捕捉到视频帧都会调用此函数
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
        if (this->getSize().x > 200 && this->getSize().y > 200) {
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