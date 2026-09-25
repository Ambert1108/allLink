# 连接引擎-SIP API

- namesapce: rtcengine
- 视频引擎: Y组
- 音频引擎: S组

### bool createPeerConnection(VideoCodecType videoCodecType, AudioCodecType audioCodecType, bool isCaller = true)

```
- 接口描述
 - 初始化音/视频引擎
 - 初始化一个对等连接对象(pc)
 - 添加音/视频轨道
 - 创建本地sdp

- 参数说明
 - videoCodecType: 视频编码格式(H264/VP9)
 - audioCodecType: 音频编码格式(PCMA/OPUS)
 - isCaller(true): 是否为主叫, 默认是主叫

- 返回值
 - true: 成功 / false: 失败
```
### void setLocal(std::string sdp)

```
- 接口描述
 - 设置本地描述信息

- 参数说明
 - sdp: 本地sdp信息(jsep)
 
- 返回值
 - 无
```
### void setRemote(std::string sdp)

```
- 接口描述
 - 设置远端描述信息

- 参数说明
 - sdp: 远端sdp信息(jsep)
 
- 返回值
 - 无
```

### void removePeerConnection();

```
- 接口描述
 - 停止屏幕信息收集
 - 关闭音/视频引擎
 - 释放pc相关资源
 
- 参数说明
 - 无
 
- 返回值
 - 无
```

-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


### void setCamera(int devId)

```
- 接口描述
 - 选择摄像头
 
- 参数说明
 - devId: 摄像头设备Id
 
- 返回值
 - 无
```

### bool openCamera()

```
- 接口描述
 - 开启摄像头
 
- 参数说明
 - 无
 
- 返回值
 - true: 成功 / false: 失败
```

### bool closeCamera()

```
- 接口描述
 - 关闭摄像头
 
- 参数说明
 - 无
 
- 返回值
 - true: 成功 / false: 失败
```

### void setMicphone(int devId)

```
- 接口描述
 - 选择麦克风
 
- 参数说明
 - devId: 麦克风设备Id
 
- 返回值
 - 无
```

### void setMicphoneVolume(int val)

```
- 接口描述
 - 设置麦克风音量
 
- 参数说明
 - val: 麦克风音量值
 
- 返回值
 - 无
```

### void setSpeaker(int devId)

```
- 接口描述
 - 选择扬声器
 
- 参数说明
 - decId: 扬声器设备Id
 
- 返回值
 - 无
```

### bool openMicphone()

```
- 接口描述
 - 开启麦克风
 
- 参数说明
 - 无
 
- 返回值
 - true: 成功 / false: 失败
```

### bool closeMicphone()

```
- 接口描述
 - 关闭麦克风
 
- 参数说明
 - 无
 
- 返回值
 - true: 成功 / false: 失败
```

### void setScreen(int devId)

```
- 接口描述
 - 选择共享屏幕
 
- 参数说明
 - devId: 共享屏幕Id
 
- 返回值
 - 无
```

### bool openScreenShare()

```
- 接口描述
 - 开启屏幕共享
 
- 参数说明
 - 无
 
- 返回值
 - true: 成功 / false: 失败
```

### bool closeScreenShare()

```
- 接口描述
 - 关闭屏幕共享
 
- 参数说明
 - 无
 
- 返回值
 - true: 成功 / false: 失败
```

### void setWindow(int devId)

```
- 接口描述
 - 选择共享窗口
 
- 参数说明
 - devId: 共享窗口Id
 
- 返回值
 - 无
```

### bool openWindowShare()

```
- 接口描述
 - 开启窗口共享
 
- 参数说明
 - 无
 
- 返回值
 - true: 成功 / false: 失败
```

### bool closeWindowShare()

```
- 接口描述
 - 关闭窗口共享
 
- 参数说明
 - 无
 
- 返回值
 - true: 成功 / false: 失败
```

### int switchLayout(string layoutNum)

```
- 接口描述
 - 切换会议画面布局
 
- 参数说明
 - layoutNum: 布局编号
 
- 返回值
 - true: 成功 / false: 失败
```

### void getMediaInfo(MediaInfo &mediaInfo);

```
- 接口描述
 - 获取当前媒体信息
 
- 参数说明
 - MediaInfo:媒体信息结构体
  - inFrameRate：输入帧率
  - outFrameRate：输出帧率
  - inBitrate：输入带宽
  - outBitrate：输出带宽
 
- 返回值
 - 无
```


## 回调接口

### virtual void OnlocalJsep(std::string sdp) = 0;

```
- 接口描述
 - 告知 本地sdp(jsep)信息
 
- 参数说明
 - sdp: 本地sdp(jsep)信息
```

### virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) = 0;

```
- 接口描述
 - 告知 本地采集到的ice信息
 
- 参数说明
 - candidate: 本地ice信息
```

### virtual void OnIceCandidateComplete() = 0;

```
- 接口描述
 - 告知 本地ice采集已完成
 
- 参数说明
 - 无
```

### virtual void OnReceiveTrack(rtc::scoped_refptr[webrtc::RtpReceiverInterface](webrtc::RtpReceiverInterface) receiver) = 0;

```
- 接口描述
 - 告知 收到远端视频画面
 
- 参数说明
 - receiver: 远端视频画面接收器, 透传
```

-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

### virtual void OnVideoInputDevInfo(std::map<int16_t, std::string> list) = 0;

```
- 接口描述
 - 告知 所有摄像头信息
 
- 参数说明
 - list: 视频设备列表
```
### virtual void OnAudioInputDevInfo(std::map<int16_t, std::string> list) = 0;
```
- 接口描述
 - 告知 所有麦克风信息
 
- 参数说明
 - list: 麦克风设备列表
```
### virtual void OnAudioOutputDevInfo(std::map<int16_t, std::string> list) = 0;
```
- 接口描述
 - 告知 所有扬声器信息
 
- 参数说明
 - list: 扬声器设备列表
```
### virtual void OnScreenInfo(std::map<int16_t, std::string> list) = 0;
```
- 接口描述
 - 告知 所有屏幕信息
 
- 参数说明
 - list: 屏幕设备列表
```
### virtual void OnWindowInfo(std::map<int16_t, std::string> list) = 0;
```
- 接口描述
 - 告知 所有窗口信息
 
- 参数说明
 - list: 窗口信息列表
```
