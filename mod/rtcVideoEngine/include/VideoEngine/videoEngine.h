#include<iostream>
#include<seeker/common.h>
#include<seeker/loggerApi.h>
#include<seeker/logger.h>
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
#include "pc/video_track_source.h"
#include "test/vcm_capturer.h"
#include "api/video/i420_buffer.h"
#include "third_party/libyuv/include/libyuv/convert_argb.h"
#include "libyuv.h"
//#include "api/video/video_de"
#include "rtc_base/thread.h"

//#include "MyCapturer.h"
#include "MyCapturer.h"

class CapturerTrackSource : public webrtc::VideoTrackSource {
public:
  static rtc::scoped_refptr<CapturerTrackSource> Create(int i);

protected:
	explicit CapturerTrackSource(
		std::unique_ptr<webrtc::test::VcmCapturer> capturer)
		: VideoTrackSource(/*remote=*/false), capturer_(std::move(capturer)) {
	}

private:
  rtc::VideoSourceInterface<webrtc::VideoFrame>* source() override;
	std::unique_ptr<webrtc::test::VcmCapturer> capturer_;
};

class VideoEngine {
public:
  VideoEngine();
  ~VideoEngine();

  void switchCamera(bool ifOpen);

  void addVideoTrack(rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>& peer_connection_factory,
    rtc::scoped_refptr<webrtc::PeerConnectionInterface>& peer_connection,
    rtc::scoped_refptr<webrtc::VideoTrackInterface>& video_track);

  bool getCameraState();

  int getCameraNameMap(std::map<int, std::string>& cameraMap);

  int setCameraDevice(const int index);

  void close();

  void removeTrack();
  
  void switchTrack(rtc::scoped_refptr<webrtc::VideoTrackInterface>& new_video_track);

  void addScreenTrack(rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>& peer_connection_factory,
    rtc::scoped_refptr<webrtc::PeerConnectionInterface>& peer_connection,
    rtc::scoped_refptr<webrtc::VideoTrackInterface>& video_track);

  rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
    peer_connection_factory_;
  rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track_;
  rtc::scoped_refptr<webrtc::VideoTrackInterface> screen_track_;
  rtc::scoped_refptr<webrtc::VideoTrackInterface> new_video_track_;
  bool cameraState = true;
  rtc::scoped_refptr<CapturerTrackSource> video_device = nullptr;
  rtc::scoped_refptr<MyCapturer> screen_device = nullptr;
};