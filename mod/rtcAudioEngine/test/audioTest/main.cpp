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
#include "api/media_stream_interface.h"
#include "api/peer_connection_interface.h"
#include "rtc_base/thread.h"
#include "absl/memory/memory.h"
#include "api/audio/audio_device.h"
#include "api/audio/audio_mixer.h"
#include "api/audio/audio_processing.h"
#include "api/audio_codecs/audio_decoder_factory.h"
#include "api/audio_codecs/audio_encoder_factory.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
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
using namespace std;
class AudioRecorder : public webrtc::AudioTransport {
public:
    AudioRecorder(const char* filename, int sample_rate, size_t channels)
        : recording(false), file(filename, std::ios::out | std::ios::binary),
        samplesPerSec(sample_rate), bytesPerSample(sizeof(int16_t)), channels(channels) {
    }

    virtual ~AudioRecorder() {
        if (file.is_open()) {
            file.close();
        }
    }

    void StartRecording() {
        recording = true;
    }
    void StopRecording() {
        recording = false;
    }
    // 旧版本的RecordedDataIsAvailable
    int32_t RecordedDataIsAvailable(const void* audioSamples,
        size_t nSamples,
        size_t nBytesPerSample,
        size_t nChannels,
        uint32_t samplesPerSec,
        uint32_t totalDelayMS,
        int32_t clockDrift,
        uint32_t currentMicLevel,
        bool keyPressed,
        uint32_t& newMicLevel) override {
        I_LOG("67");
        return RecordedDataIsAvailable(audioSamples,
            nSamples,
            nBytesPerSample,
            nChannels,
            samplesPerSec,
            totalDelayMS,
            clockDrift,
            currentMicLevel,
            keyPressed,
            newMicLevel,
            std::nullopt);
    }
    int32_t RecordedDataIsAvailable(const void* audioSamples,
        size_t nSamples,
        size_t nBytesPerSample,
        size_t nChannels,
        uint32_t samplesPerSec,
        uint32_t totalDelayMS,
        int32_t clockDrift,
        uint32_t currentMicLevel,
        bool keyPressed,
        uint32_t& newMicLevel,
        std::optional<int64_t> estimatedCaptureTimeNS) override {
        if (!recording) 
            return 0;
        // 将音频数据写入文件
        if (file.write(static_cast<const char*>(audioSamples), nSamples * nBytesPerSample * nChannels)) {
            return 0;
        }
        else {
            return -1;
        }
    }

    int32_t NeedMorePlayData(size_t nSamples,
        size_t nBytesPerSample,
        size_t nChannels,
        uint32_t samplesPerSec,
        void* audioSamples,
        size_t& nSamplesOut,
        int64_t* elapsed_time_ms,
        int64_t* ntp_time_ms) override {
        nSamplesOut = 0;
        return 0;
    }

    void PullRenderData(int bits_per_sample,
        int sample_rate,
        size_t number_of_channels,
        size_t number_of_frames,
        void* audio_data,
        int64_t* elapsed_time_ms,
        int64_t* ntp_time_ms) override {
    }

private:
    bool recording;
    std::ofstream file;
    const uint32_t samplesPerSec;
    const size_t bytesPerSample;
    const size_t channels;
};

int main() {
	SetConsoleOutputCP(65001);
	std::unique_ptr<webrtc::TaskQueueFactory> task_queue_factory = webrtc::CreateDefaultTaskQueueFactory();
	rtc::scoped_refptr<webrtc::AudioDeviceModule> adm = webrtc::AudioDeviceModule::Create(webrtc::AudioDeviceModule::kPlatformDefaultAudio, task_queue_factory.get());
	adm->Init();
    //AudioRecorder recorder("output.pcm", 48000, 2);
   // adm->RegisterAudioCallback(&recorder);
	int16_t num_devices = adm->RecordingDevices();
	I_LOG("num [{}]", num_devices);
	for (int i = 0; i < num_devices; ++i) {
		char name[256];
		char guid[256];
		if (adm->RecordingDeviceName(i, name, guid) == 0) {
			// 打印设备名称和GUID
			I_LOG("i [{}] name [{}] guid [{}]", i, name, guid);
		}
	}
	adm->SetRecordingDevice(0); 

    int16_t num_devices1 = adm->PlayoutDevices();
    I_LOG("num [{}]", num_devices1);
    for (int i = 0; i < num_devices1; ++i) {
        char name[256];
        char guid[256];
        if (adm->PlayoutDeviceName(i, name, guid) == 0) {
            // 打印设备名称和GUID
            I_LOG("i [{}] name [{}] guid [{}]", i, name, guid);
        }
    }
	// 初始化录音
	adm->InitRecording();
    //recorder.StartRecording();
	// 开始录音
	adm->StartRecording();

    adm->SetPlayoutDevice(1);
    adm->InitPlayout();
    adm->StartPlayout();
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
	// 停止录音
	adm->StopRecording();
    //recorder.StopRecording();
	return 0;
}