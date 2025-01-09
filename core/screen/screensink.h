#pragma once
#include "basescreen.h"
#include "config/path.h"
#include "component/module.h"
#include "component/widget.h"
#include "message.h"

#include "seeker/common.h"

#include "api/video/video_sink_interface.h"
#include "api/media_stream_interface.h"
#include "api/video/video_frame.h"
#include "media/base/media_channel.h"
#include "media/base/video_common.h"
#if defined(WEBRTC_WIN)
#include "rtc_base/win32.h"
#endif  // WEBRTC_WIN
#include "api/async_dns_resolver.h"
#include "api/task_queue/pending_task_safety_flag.h"
#include "rtc_base/net_helpers.h"
#include "rtc_base/physical_socket_server.h"
#include "rtc_base/third_party/sigslot/sigslot.h"

namespace alllink {

	class CustomScreen : public BaseScreen {
	public:
		enum Style {
			Close = 0,       //表示窗口只有关闭
			Minisize,        //表示窗口拥有最小化和关闭
			All              //默认选项，窗口拥有最小化、最大化及关闭
		};

		CustomScreen(sf::VideoMode mode, const sf::String& title, sf::Image icon, int style);

		void checkStatus(sf::Event& event);

		virtual void needClose() { this->close(); };

	protected:
		Style style_;
		sf::RectangleShape topSide_;
		sf::VideoMode wndSize_;
		/* 最大化按钮 */
		VariableStateRectangleModule square;
		/* 最小化按钮 */
		VariableStateVertxModule horizontalLine;
		/* 关闭按钮 */
		VariableStateVertxModule cross;
		/* 窗口是否最大化 */
		bool isDesktop;
	private:
		HWND hwnd;
		sf::Vector2i screenSize;
		sf::Vector2i dragOffset;
		bool isDragging;
		int maxX = 0;
		int minX = 0;
		int maxY = 0;
		int minY = 0;
		sf::View view;
		sf::Vector2i wndPos;
	};

	/*
	* 程序的开始界面，提供登录/注销，创建/加入会议和设置功能，属于流式界面，
	* 可以切换至下一个流式界面。
	*/
	class StartScreen : public CustomScreen {
	public:
		enum class LoginType {
			OFFLINE = 0,
			ONLINE
		};

		StartScreen(sf::VideoMode mode, const sf::String& title,
			sf::Image icon, int style = CustomScreen::Style::All);

		~StartScreen();

		bool OnEnter() override;

		bool OnExit() override;

		int init() override;

		void show() override;

		void eventProcess() override;

		void OnFailed() override {}

		LoginType type() const;

		void setUseId(const std::string& id);

	private:
		std::unique_ptr<VerticalGraphicTextsModule> createMeeting;
		std::unique_ptr<VerticalGraphicTextsModule> joinMeeting;
		std::unique_ptr<HorizonGraphicTextsModule> startLogin;
		std::unique_ptr<HorizonGraphicTextsModule> isLogin;
		std::unique_ptr<VariableStateGraphicModule> setting;
		BaseText todayDate;
		BaseText useId;
		LoginType type_{ LoginType::OFFLINE };
		sf::RectangleShape taskSide;
		float wr, hr;
		sf::Vector2i wndPosition;
	};

	/*
	* 程序的登录界面，允许用户选择服务器并输入账号密码，属于独立界面，
	* 无法进行界面切换，最终目的只是获取用户输入。
	*/
	class LoginScreen : public CustomScreen {
	public:
		LoginScreen(sf::VideoMode mode, const sf::String& title,
			sf::Image icon, int style = CustomScreen::Style::All);

		~LoginScreen();

		bool OnEnter() override;

		bool OnExit() override;

		int init() override;

		void show() override;

		void eventProcess() override;

		void OnFailed() override;

	protected:
		void needClose() override { OnExit(); }

	private:
		void reset();

		std::unique_ptr<EnterDescriptionWidget> inputSeverAddrWidget;
		std::unique_ptr<EnterDescriptionWidget> inputUserIdWidget;
		std::unique_ptr<EnterDescriptionWidget> inputUserPwdWidget;
		std::unique_ptr <TextRectangle> loginButton;
		BaseText screenDescriptionText;
		float wr, hr;
		sf::Vector2i wndPosition;
		int currentInputBox = 1;
	};

	/*
	* 程序的创建/加入会议界面，允许用户输入会议号以创建/加入会议，属于独立界面，
	* 无法进行界面切换，最终目的只是获取用户输入。
	*/
	class EnterScreen : public CustomScreen {
	public:
		enum class EnterType {
			NONE = 0,
			CREATE,
			JOIN
		};

		EnterScreen(sf::VideoMode mode, const sf::String& title,
			sf::Image icon, int style = CustomScreen::Style::All);

		~EnterScreen();

		bool OnEnter() override;

		bool OnExit() override;

		int init() override;

		void show() override;

		void eventProcess() override;

		void OnFailed() override;

		void setType(EnterType type);

	protected:
		void needClose() override { OnExit(); }

	private:
		void reset();

		std::unique_ptr<EnterDescriptionWidget> inputMeetingIdWidget;
		std::unique_ptr <TextRectangle> createButton;
		std::unique_ptr <TextRectangle> joinButton;
		BaseText screenDescriptionText;
		float wr, hr;
		sf::Vector2i wndPosition;
		EnterType type_{ EnterType::NONE };
	};

	/*
	* 程序的设置界面，功能暂定，属于独立界面，
	* 无法进行界面切换，最终目的只是获取用户输入。
	*/
	class SettingScreen : public CustomScreen {

	};

	/*
	* 程序的会议界面，提供会议画面接收，开关摄像头/麦克风/屏幕共享功能，属于流式界面，
	* 可以切换至上一个流式界面。
	*/
	class StreamScreen : public BaseScreen {
	public:
		StreamScreen(sf::VideoMode mode, const sf::String& title,
			sf::Image icon, sf::Uint32 style = sf::Style::Default);

		~StreamScreen();

		bool OnEnter() override;

		bool OnExit() override;

		int init() override;

		void show() override;

		void eventProcess() override;

		void OnFailed() override {}

		void startLocalRenderer(webrtc::VideoTrackInterface* local_video);

		void stopLocalRenderer();

		void startRemoteRenderer(webrtc::VideoTrackInterface* remote_video);

		void stopRemoteRenderer();

		void setSessionMode(int mode);

		void setSessionId(std::string id);

		void setAudioDev(const std::map<int16_t, std::string>& list);

		struct ImageData {
			BITMAPINFO bmi;
			std::unique_ptr<uint8_t[]> image = nullptr;

			ImageData() = default;

			ImageData(const BITMAPINFO& bm, const uint8_t* data) : bmi(bm) {
				image.reset(new uint8_t[bmi.bmiHeader.biSizeImage]);
				memcpy(image.get(), data, bmi.bmiHeader.biSizeImage);
			}

			ImageData(const ImageData& other) : bmi(other.bmi) {
				if (other.image) {
					image.reset(new uint8_t[bmi.bmiHeader.biSizeImage]);
					std::copy(other.image.get(), other.image.get() + bmi.bmiHeader.biSizeImage, image.get());
				}
			}

			ImageData& operator=(const ImageData& other) {
				if (this != &other) {
					image.reset();
					bmi = other.bmi;

					if (other.image) {
						image.reset(new uint8_t[bmi.bmiHeader.biSizeImage]);
						std::copy(other.image.get(), other.image.get() + bmi.bmiHeader.biSizeImage, image.get());
					}
				}
				return *this;
			}
		};

		class VideoRenderer : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
		public:
			VideoRenderer(std::function<void()> callback,
				int width,
				int height,
				webrtc::VideoTrackInterface* track_to_render);
			virtual ~VideoRenderer();

			void Lock() { ::EnterCriticalSection(&buffer_lock_); }

			void Unlock() { ::LeaveCriticalSection(&buffer_lock_); }

			// VideoSinkInterface implementation
			void OnFrame(const webrtc::VideoFrame& frame) override;

			const BITMAPINFO& bmi() const { return bmi_; }
			const uint8_t* image() const { return image_.get(); }

		protected:
			void SetSize(int width, int height);

			enum {
				SET_SIZE,
				RENDER_FRAME,
			};

			std::function<void()> paint = nullptr;
			BITMAPINFO bmi_;
			std::unique_ptr<uint8_t[]> image_;
			CRITICAL_SECTION buffer_lock_;
			rtc::scoped_refptr<webrtc::VideoTrackInterface> rendered_track_;
		};

		template <typename T>
		class AutoLock {
		public:
			explicit AutoLock(T* obj) : obj_(obj) { obj_->Lock(); }
			~AutoLock() { obj_->Unlock(); }

		protected:
			T* obj_;
		};

	protected:
		void OnPaint();

	private:
		std::unique_ptr<VariableStateGraphicModule> closeMic;
		std::unique_ptr<VariableStateGraphicModule> openMic;
		std::unique_ptr<VariableStateGraphicModule> closeCam;
		std::unique_ptr<VariableStateGraphicModule> openCam;
		std::unique_ptr<VariableStateGraphicModule> closeShare;
		std::unique_ptr<VariableStateGraphicModule> openShare;
		std::unique_ptr<HorizonGraphicTextsModule> meetingTime;
		std::unique_ptr<TextFillRectangle> leaveMeeting;
		std::unique_ptr<BaseText> meetingDescribe;
		std::unique_ptr<SeekBarModule> volumeBar;
		std::unique_ptr<VariableStateVertxModule> audioDevArrow;
		std::unique_ptr<DropListModule> audioDevList;
		std::unique_ptr<VariableStateFillModule> audioDevBackground;
		sf::RectangleShape bottom, top;

		std::unique_ptr<VideoRenderer> local_renderer_;
		std::unique_ptr<VideoRenderer> remote_renderer_;
		sf::Texture* localSrc = nullptr;
		sf::Texture* remoteSrc = nullptr;
		VideoModule localVideo{};
		VideoModule remoteVideo{};
		base::ThreadSafeQueue<ImageData> remoteImageList{};
		base::ThreadSafeQueue<ImageData> localImageList{};
		std::atomic<bool> isMirror{ false };
		float wr, hr;
		sf::Vector2i wndPosition;
		bool micState = false;
		bool camState = false;
		bool shareState = false;
		bool isFull = true;
		bool micSettingPop = false;
		int micVolume = 50;
		int mode = 0; //1v1通话:0, 会议流程:1
		int64_t timePoint = 0;
	};
}