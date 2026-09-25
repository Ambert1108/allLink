### 视频引擎API

- 添加视频轨道

  void addVideoTrack(

  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>& peer_connection_factory,
  rtc::scoped_refptr<webrtc::PeerConnectionInterface>& peer_connection,
  rtc::scoped_refptr<webrtc::VideoTrackInterface>& video_track);

- 开关摄像头

  void switchCamera(bool ifOpen);

- 获取摄像头状态

  bool VideoEngine::getCameraState()