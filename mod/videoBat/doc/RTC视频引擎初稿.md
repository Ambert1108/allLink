
### 需求：
- 终端使用视频引擎实现以下能力
	- 视频设备获取
	- 视频画面采集
	- 视频编码
	- 视频网络发送
	- 视频网络接收
	- 视频解码
	- 视频渲染
- 当前使用工厂进行实现大部分能力，若部分能力无法实现则需要进行重写
### API:
### 视频引擎API

- 添加视频轨道
```
int addVideoTrack(
rtc::scoped_refptr< webrtc::PeerConnectionFactoryInterface>& peer_connection_factory, 
rtc::scoped_refptr<webrtc::PeerConnectionInterface>& peer_connection, rtc::scoped_refptr<webrtc::VideoTrackInterface>& video_track);
```    

- 获取摄像头列表
	- std::map<int, string> getCameraMap();
		- 返回值：摄像头列表, cameraId, camera名称

- 选择指定摄像头
	- int setCamera(int cameraId);
		- 输入值：摄像头Id;
		- 返回值：0 成功，其他失败

- 开关摄像头
    - int switchCamera(bool flag);
		- 返回值：0 成功，其他失败
    
- 开关视频镜像
	- int openVideoMirror();
		- 返回值：0 成功，其他失败
	- int closeVideoMirror();
		- 返回值：0 成功，其他失败

