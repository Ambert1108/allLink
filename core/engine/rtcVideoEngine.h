#pragma once

#include<iostream>
#include<seeker/common.h>
#include<seeker/loggerApi.h>
#include<seeker/logger.h>
#include <Windows.h>
#include <Shellscalingapi.h>
#include <dwmapi.h>
#include <d3d11.h>
#include <dxgi.h>
#include <string>
#include <chrono>
#include <condition_variable>
#include <stddef.h>
#include <stdint.h>
#include <memory>
#include <optional>
#include <utility>
#include <vector>
#include <atomic>
#include "api/media_stream_interface.h"
#include "api/peer_connection_interface.h"
#include "api/rtp_sender_interface.h"
#include "api/rtp_parameters.h"
#include "api/video_codecs/video_encoder.h"
#include "api/video_codecs/video_encoder_factory.h"
#include "api/video/video_frame.h"
#include "modules/video_coding/include/video_error_codes.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "modules/video_capture/video_capture.h"
#include "modules/video_capture/video_capture_factory.h"
#include "modules/desktop_capture/desktop_capturer.h"
#include "modules/desktop_capture/desktop_capture_options.h"
#include "pc/video_track_source.h"
#include "test/vcm_capturer.h"
#include "api/video/i420_buffer.h"
#include "third_party/libyuv/include/libyuv/convert_argb.h"
#include "libyuv.h"
#include "rtc_base/thread.h"
#include "screenCapturer.h"

namespace rtcengine {
	class CapturerTrackSource : public webrtc::VideoTrackSource {
	public:
		static rtc::scoped_refptr<CapturerTrackSource> Create(int deviceId = 0) {
			const size_t kWidth = 640;
			const size_t kHeight = 480;
			const size_t kFps = 30;
			std::unique_ptr<webrtc::test::VcmCapturer> capturer;
			std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> info(
				webrtc::VideoCaptureFactory::CreateDeviceInfo());
			if (!info) {
				return nullptr;
			}

			int num_devices = info->NumberOfDevices();
			I_LOG("num_devices:{}, open index = {}", num_devices, deviceId);
			if (deviceId >= num_devices) {
				//如果需要开启的摄像头超出摄像头数量，则使用默认为0摄像头
				deviceId = 0;
			}
			for (int i = 0; i < num_devices; ++i) {
				char devName[256] = { 0 };
				char uniqueName[256] = { 0 };
				if (info->GetDeviceName(i, devName, 256, uniqueName, 256) != -1)
				{
					std::string devNameStr = devName;
					std::string devNameUniq = uniqueName;
					I_LOG("devName = {}, uniqueName = {}", devName, uniqueName);
				}
			}
			for (int i = 0; i < num_devices; ++i) {
				capturer = absl::WrapUnique(
					webrtc::test::VcmCapturer::Create(kWidth, kHeight, kFps, i));
				if (capturer && i == deviceId) {
					I_LOG("open index = {}", deviceId);
					return rtc::make_ref_counted<CapturerTrackSource>(std::move(capturer));
				}
			}
			return nullptr;
		}

	protected:
		explicit CapturerTrackSource(
			std::unique_ptr<webrtc::test::VcmCapturer> capturer)
			: VideoTrackSource(/*remote=*/false), capturer_(std::move(capturer)) {
		}

	private:
		rtc::VideoSourceInterface<webrtc::VideoFrame>* source() override {
			return capturer_.get();
		}
		std::unique_ptr<webrtc::test::VcmCapturer> capturer_;
	};

  class RTCVideoEngine {
  public:
    RTCVideoEngine();
    ~RTCVideoEngine();

    int addVideoTrack(rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>& peer_connection_factory,
      rtc::scoped_refptr<webrtc::PeerConnectionInterface>& peer_connection,
      rtc::scoped_refptr<webrtc::VideoTrackInterface>& video_track);

		void addScreenTrack(rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>& peer_connection_factory,
			rtc::scoped_refptr<webrtc::PeerConnectionInterface>& peer_connection,
			rtc::scoped_refptr<webrtc::VideoTrackInterface>& video_track);

		void setScreenCapture(uint8_t id);

		const std::string GetSourceListString();
		bool GetSourceList(webrtc::DesktopCapturer::SourceList* sources);
		webrtc::DesktopCaptureOptions CreateDesktopCaptureOptions();

    int switchCamera(bool flag);
    int switchScreen(bool flag);
    int openVideoMirror() {};
    int closeVideoMirror() {};
    bool getCameraState();
    bool getScreenState();

    std::map<int, std::string> getCameraMap();

    int setCamera(const int index, rtc::scoped_refptr<webrtc::VideoTrackInterface>& video_track);

    void switchTrack(rtc::scoped_refptr<webrtc::VideoTrackInterface>& new_video_track);

    void close();

    rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
      peer_connection_factory_;
    rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track_;
    rtc::scoped_refptr<webrtc::VideoTrackInterface> screen_track_;
    rtc::scoped_refptr<webrtc::VideoTrackInterface> new_video_track_;
    rtc::scoped_refptr<ScreenCapturer> screen_device = nullptr;
    rtc::scoped_refptr<CapturerTrackSource> video_device = nullptr;
    bool cameraState = true;
    bool screenState = true;
  };
}
