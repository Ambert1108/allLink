#include"rtcAudioEngine.h"
namespace rtcengine {
	rtcAudioEngine::rtcAudioEngine() {

	}
	rtcAudioEngine::~rtcAudioEngine() {
		if (audio_track) {
			audio_track = nullptr;
		}
		if (adm) {
			adm->StopRecording();
			adm->StopPlayout();
			adm = nullptr;
		}
		if (task_queue_factory)
		{
			task_queue_factory = nullptr;
		}
	}

	rtc::scoped_refptr<webrtc::AudioDeviceModule> rtcAudioEngine::InitAdm() {
		if (!task_queue_factory) {
			task_queue_factory = nullptr;
		}
		task_queue_factory = webrtc::CreateDefaultTaskQueueFactory();
		if (!adm) {
			adm = nullptr;
		}
		adm = webrtc::AudioDeviceModule::Create(webrtc::AudioDeviceModule::kPlatformDefaultAudio, task_queue_factory.get());
		if (!adm) {
			E_LOG("AudioDeviceModule create failed");
			return nullptr;
		}
		int32_t init_result = adm->Init();
		if (init_result != 0) {
			E_LOG("AudioDeviceModule Init [{}] error", init_result);
			return nullptr;
		}
		return adm;
	}

	void rtcAudioEngine::GetRecordingDevices(std::map<int16_t, std::string>& recordingDevice) {
		if (!adm) {
			E_LOG("adm no create");
			return;
		}
		int16_t recording_num_devices = adm->RecordingDevices();
		I_LOG("recording_num_devices [{}]", recording_num_devices);
		for (int i = 0; i < recording_num_devices; ++i) {
			char name[256];
			char guid[256];
			if (adm->RecordingDeviceName(i, name, guid) == 0) {
				// ��ӡ�豸���ƺ�GUID
				I_LOG("RecordingDevice i [{}] name [{}] guid [{}]", i, name, guid);
				recordingDevice[i] = name;
			}
		}
	}

	void rtcAudioEngine::GetPlayoutDevices(std::map<int16_t, std::string>& playoutDevices) {
		if (!adm) {
			E_LOG("adm no create");
			return;
		}
		int16_t playout_num_devices = adm->PlayoutDevices();
		for (int i = 0; i < playout_num_devices; ++i) {
			char name[256];
			char guid[256];
			if (adm->PlayoutDeviceName(i, name, guid) == 0) {
				I_LOG("PlayoutDevices i [{}] name [{}] guid [{}]", i, name, guid);
				playoutDevices[i] = name;
			}
		}
	}

	bool rtcAudioEngine::SetRecordingDevices(uint16_t recordingIndex) {
		if (!adm) {
			E_LOG("adm no create");
			return false;
		}
		int f1 = adm->SetRecordingDevice(recordingIndex);
		int f2 = adm->InitRecording();
		return f1 && f2;
	}

	bool rtcAudioEngine::SetPlayoutDevices(uint16_t playoutIndex) {
		if (!adm) {
			E_LOG("adm no create");
			return false;
		}
		int f1 = adm->SetPlayoutDevice(playoutIndex);
		int f2 = adm->InitPlayout();
		return f1 && f2;
	}

	bool rtcAudioEngine::ReplaceRecordingDevices(uint16_t recordingIndex) {
		if (!adm) {
			E_LOG("adm no create");
			return false;
		}
		int f1 = adm->StopRecording();
		int f2 = adm->SetRecordingDevice(recordingIndex);
		int f3 = adm->InitRecording();
		int f4 = adm->StartRecording();
		return f1 && f2 && f3 && f4;
	}

	bool rtcAudioEngine::ReplacePlayoutDevices(uint16_t playoutIndex) {
		if (!adm) {
			E_LOG("adm no create");
			return false;
		}
		int f1 = adm->StopPlayout();
		int f2 = adm->SetPlayoutDevice(playoutIndex);
		int f3 = adm->InitPlayout();
		int f4 = adm->StartPlayout();
		return f1 && f2 && f3 && f4;
	}

	bool rtcAudioEngine::AddAudioTracks(rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory_, rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_)
	{
		rtc::scoped_refptr<webrtc::AudioTrackInterface> a(
			peer_connection_factory_->CreateAudioTrack(
				kAudioLabel,
				peer_connection_factory_->CreateAudioSource(cricket::AudioOptions())
				.get()));
		auto result_or_error = peer_connection_->AddTrack(a, { kStreamId });
		if (!result_or_error.ok()) {
			E_LOG("Failed to add audio track to PeerConnection: ", result_or_error.error().message());
			return false;
		}
		audio_track = a;
		return true;
	}

	bool rtcAudioEngine::setMicrophone(bool new_state)
	{
		if (audio_track) {
			return audio_track->set_enabled(new_state);
		}
		E_LOG("audio_track no create");
		return false;
	}

	void rtcAudioEngine::close()
	{
		audio_track = nullptr;
		adm->StopRecording();
		adm->StopPlayout();
		adm = nullptr;
		task_queue_factory = nullptr;
	}

}