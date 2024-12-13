#pragma once
#include<iostream>
#include<seeker/common.h>
#include<seeker/loggerApi.h>
#include<seeker/logger.h>
#include"utils/httplib.h"
#include <string>
#include <chrono>
#include <condition_variable>
#include <stddef.h>
#include <stdint.h>
#include <memory>
#include <optional>
#include <utility>
#include <vector>
#include <map>
#include "api/media_stream_interface.h"
#include "api/peer_connection_interface.h"
#include "rtc_base/thread.h"
#include "absl/memory/memory.h"
#include "api/audio_options.h"
#include "api/create_peerconnection_factory.h"
#include "api/rtp_sender_interface.h"
#include "p2p/base/port_allocator.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/rtc_certificate_generator.h"
#include "rtc_base/strings/json.h"
#include "test/vcm_capturer.h"
#include "api/task_queue/default_task_queue_factory.h"
#include "media/engine/webrtc_media_engine.h"
#include "modules/audio_device/include/audio_device.h"
namespace rtcengine {
	const char kAudioLabel[] = "audio_label";
	const char kStreamId[] = "audio_id";
	class rtcAudioEngine
	{
	public:
		rtcAudioEngine();
		~rtcAudioEngine();
		
		//创建adm，成功返回adm，失败返回nullptr
		rtc::scoped_refptr<webrtc::AudioDeviceModule> InitAdm();
		
		//获取麦克风设备，返回麦克风id-麦克风name的map
		void GetRecordingDevices(std::map<int16_t, std::string>& recordingDevice);
		
		//获取扬声器设备，返回扬声器id-扬声器name的map
		void GetPlayoutDevices(std::map<int16_t, std::string>& playoutDevices);
		
		//设置麦克风，失败返回false，成功返回true
		bool SetRecordingDevices(uint16_t recordingIndex);
		
		//设置扬声器，失败返回false，成功返回true
		bool SetPlayoutDevices(uint16_t playoutIndex);
		
		//更换麦克风，失败返回false，成功返回true
		bool ReplaceRecordingDevices(uint16_t recordingIndex);
		
		//更换扬声器，失败返回false，成功返回true
		bool ReplacePlayoutDevices(uint16_t playoutIndex);
		
		//添加音频轨道，失败返回false，成功返回true
		bool AddAudioTracks(rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory_, rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_);
		
		//设置麦克风，失败返回false，成功返回true
		bool setMicrophone(bool new_state);
		
		//设置扬声器音量大小。失败返回false，成功返回true
		bool setPlayoutVolume(const int volume);

		//设置麦克风音量大小。失败返回false，成功返回true
		bool setMicrophoneVolume(const int volume);

		//获取扬声器音量大小
		uint32_t PlayoutVolume();

		//获取麦克风音量大小
		uint32_t MicrophoneVolume();

		//清除adm、audio_track;
		void close();
		std::string modifySdp(const std::string& sdp);
	private:
		rtc::scoped_refptr<webrtc::AudioDeviceModule> adm;
		std::unique_ptr<webrtc::TaskQueueFactory> task_queue_factory;
		rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track;
		bool shouldKeepCodec(const std::string& line);
	};
}