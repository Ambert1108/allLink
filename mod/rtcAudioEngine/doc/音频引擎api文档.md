# api文档
- 创建adm
`rtc::scoped_refptr<webrtc::AudioDeviceModule> InitAdm();`
  - 返回值：
	- 成功
		- adm
	- 失败
		- nullptr
- 添加音频轨道
`bool AddAudioTracks(rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory_,rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_);`
  - 返回值：
	- 成功
		- adm
	- 失败
		- nullptr
- 打开麦克风
`bool openMicrophone()`
  - 返回值：
	- 成功
		- true
	- 失败
		- false
- 关闭麦克风
`bool closeMicrophone()`
  - 返回值：
	- 成功
		- true
	- 失败
		- false
- 获取音频输入设备
`void GetRecordingDevices(std::map<int16_t,std::string>& recordingDevice);`

- 获取音频输出设备
`void GetPlayoutDevices(std::map<int16_t,std::string>& playoutDevices);`

- 指定音频输入设备
`bool SetRecordingDevices(uint16_t recordingIdex);`
  - 返回值：
	- 成功
		- true
	- 失败
		- false

- 指定音频输出设备
`bool SetPlayoutDevices(uint16_t playoutIndex);`
  - 返回值：
	- 成功
		- true
	- 失败
		- false

- 更换音频输入设备
`bool ReplaceRecordingDevices(uint16_t recordingIndex);`
  - 返回值：
	- 成功
		- true
	- 失败
		- false

- 更换音频输出设备
`bool ReplacePlayoutDevices(uint16_t playoutIndex);`
  - 返回值：
	- 成功
		- true
	- 失败
		- false
- 设置扬声器音量大小
`bool setPlayoutVolume(const uint32_t volume);`
  - 返回值：
	- 成功
		- true
	- 失败
		- false
		
- 设置麦克风音量大小
`bool setMicrophoneVolume(const uint32_t volume);`
  - 返回值：
	- 成功
		- true
	- 失败
		- false
- 获取扬声器音量大小
`uint32_t PlayoutVolume()`
  - 返回值：
	- 成功
		- 音量大小
	- 失败
		- -1	
- 获取麦克风音量大小
`uint32_t MicrophoneVolume()`
  - 返回值：
	- 成功
		- 音量大小
	- 失败
		- -1	

- 修改sdp
`std::string modifySdp(const std::string& sdp`