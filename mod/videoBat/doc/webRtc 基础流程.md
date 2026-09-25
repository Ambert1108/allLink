# 目标任务
- 列出 Webrtc 通话建立的完整流程，并对流程中的关键步骤的输入输出进行详细说明
- 列出 WebRTC 中的关键概念, 并在理解后进行整理


# 完整流程

### 简述：

1. 两端(web或应用程序)与信令建立连接
2. 分别创建 PeerConnectionFactory及 PeerConnection
3. 分别获取本地媒体流，将媒体流对象作为参数传递给RTCPeerConnection
4. 分别生成 Local SDP 并交换SDP，以生成 Remote SDP
5. 分别从信令提供的 ICE服务器地址中获取 公网地址
6. 分别生成自身 ICE candidate，可能包含多个地址，发送给信令服务器，转发给对端
7. 分别收到对端 ICE candidate，将其传递给RTCPeerConnection 并尝试建立连接
8. 连接建立成功，通过OnAddTrack对收到媒体流进行处理

![[Pasted image 20241125172448.png]]


#### 详细流程

1. 发起方（p1）和参与者（p2）各自创建`peerConnectionFactory`并初始化
	
2. 发起方（p1）和参与者（p2）连接信令服务器，获取ICE服务器地址
	

> 在新建`RTCPeerConnection`时可在构造函数指定 ICE 服务器地址，没有指定的话则意味着这个连接只能在内网进行

```cpp
const configuration = {
	iceServers: [ { urls: 'stun:stun.l.google.com:19302' }, 
				  { urls: 'turn:yourturnserver.com', username: 'user', credential: 'pass' } ],
	iceTransportPolicy: 'all', // 或 'relay' 仅使用 TURN 
	bundlePolicy: 'max-bundle' // 或 'balanced' };
```

3. 发起方（p1）和参与者（p2）添加传输给对方的音频与视频轨道
	
4. 发起方（p1）创建offer SDP，设置本地会话描述，发送给信令服务器，信令服务器转发offer SDP给参与者（p2）
	
5. 参与者（p2）将offer SDP设置为远端会话描述，创建answer SDP，设置本地会话描述，发送给信令服务器，信令服务器转发answer SDP给发起方（p1），发起方（p1）将answer SDP设置为远端会话描述
	

> 在发起方（p1）设置本地会话描述执行成功后，协商 SDP 和 ICE candidate 的流程便会同时开始

6. 发起方（p1）和参与者（p2）**同时生成**ICE，图中展示为 发起方（p1）将生成的ICE candidate 发送给信令服务器，信令服务器转发给参与者（p2）
	
7. 参与者（p2）收到ICE后将其作为远端地址尝试连接，同时将生成的ICE候选发给信令服务器，信令服务器转发给发起方（p1）
	
8. 发起方（p1）收到ICE将其作为远端地址尝试连接

> Candidate 的交换不是等所有 Candidate 收集好后才进行的，而是边收集边交换


# 关键步骤 

![[Pasted image 20241125175351.png]]

1. create PeerConnectionFactory
	1. CreatePeerConnectionFactory(...)
	2. 用途：其用于 创建及管理 RTCPeerConnnection 对象
	3. 输入值：三个线程及一些指针，除信令线程可以为空，
	4. 返回值：PeerConnectionFactoryInterface 对象
	5. api位置 api/create_peerconnection_factory.h
```cpp
		RTC_EXPORT rtc::scoped_refptr<PeerConnectionFactoryInterface>
CreatePeerConnectionFactory(
    rtc::Thread* network_thread, 网络线程
    rtc::Thread* worker_thread, 工作者线程
    rtc::Thread* signaling_thread, 信令线程
    rtc::scoped_refptr<AudioDeviceModule> default_adm, 音频设备模块adm
    rtc::scoped_refptr<AudioEncoderFactory> audio_encoder_factory, 音频编码器工厂
    rtc::scoped_refptr<AudioDecoderFactory> audio_decoder_factory, 
    std::unique_ptr<VideoEncoderFactory> video_encoder_factory, 
    std::unique_ptr<VideoDecoderFactory> video_decoder_factory, 
    rtc::scoped_refptr<AudioMixer> audio_mixer,
    rtc::scoped_refptr<AudioProcessing> audio_processing,
    std::unique_ptr<AudioFrameProcessor> audio_frame_processor = nullptr,
    std::unique_ptr<FieldTrialsView> field_trials = nullptr);

}  // namespace webrtc
```

2. create RTCPeerConnection
	1. RTCPeerConnection(configuration， dependencies)
	2. config 提供webRTC内部使用的参数信息，用于控制内部逻辑及行为方式。
	3. dependencies(PeerConnectionDependencies) 定义用户提供的可执行代码，执行用户自定义的逻辑.其中最重要的就是**PeerConnectionObserver**，是PeerConnection的事件回调，相关OnXXX事件通过其进行触发。
	4. api位置：/api/peer_connection_interface.h
```cpp
  virtual rtc::scoped_refptr<PeerConnectionInterface> CreatePeerConnection(
      const PeerConnectionInterface::RTCConfiguration& configuration,
      PeerConnectionDependencies dependencies);
```
3. AddTrack
	1. 在使用 CreateAudioTrack及CreateVideoTrack后，调用 PeerConnection::AddTrack方法
	2. 输入：媒体轨道Track以及媒体流id向量
	3. 添加轨道时，判断信令及轨道是否为空，以及轨道是否重复，PC的信令状态是否为 IsClosed
	4. api位置：/api/peer_connection_interface.h
```cpp
  virtual RTCErrorOr<rtc::scoped_refptr<RtpSenderInterface>> AddTrack(
      rtc::scoped_refptr<MediaStreamTrackInterface> track,
      const std::vector<std::string>& stream_ids) = 0;
```
4. CreateOffer
	1. 输入：observer 及 options
	2. observer(观察者)，提供回调虚函数，可以在接收到相关的事件时，执行回调中自定义的代码流程。
```cpp
void CreateOffer(CreateSessionDescriptionObserver* observer, const RTCOfferAnswerOptions& options) override;
```
5. SetLocalDescription
	1. 设置本地的媒体描述信息
```cpp
  virtual void SetLocalDescription(
      std::unique_ptr<SessionDescriptionInterface> desc,
      rtc::scoped_refptr<SetLocalDescriptionObserverInterface> observer) {}
```
6. CreateAnswer
	1. 根据offer的信息生成 Answer
```cpp
void CreateAnswer(CreateSessionDescriptionObserver* observer, const RTCOfferAnswerOptions& options) override;
```
7. SetRemoteDescription
	1. 设置远端的媒体描述信息
8. OnIceCandidate
	1. 当生成新的ICE候选者时，触发该api，执行代码中的流程，通常为发送candidate(候选者)信息给远端
```cpp
  virtual void OnIceCandidate(const IceCandidateInterface* candidate) = 0;
```
9. AddIceCandidate
	1. 接收到远端的候选者信息，通过该api将候选者传递给 RTCPeerConnection 对象
	2. 返回是否成功添加
```cpp
  virtual bool AddIceCandidate(const IceCandidateInterface* candidate) = 0;
```
10. OnAddTrack
	1. 如上所述，OnXXX 为触发类事件，当收到媒体流时，触发该api，执行媒体流处理相关流程，从而实现音视频通话。

```
  virtual void OnAddTrack(
      rtc::scoped_refptr<RtpReceiverInterface> receiver,
      const std::vector<rtc::scoped_refptr<MediaStreamInterface>>& streams) {}
```


# 关键概念
## 为什么要使用webRtc？
- 因为其可实现端到端数据传输，做到极低延迟，并且减轻服务器负担(服务器基本只用来进行信令操作)。
- 支持跨平台操作，免去跨平台开发支持的繁琐。
- 支持自定义的数据传输，扩展性高。
 
## webRtc 如何实现p2p通信，两端如何知晓对方地址？
- webRtc 并没有定义信令传输的方式，地址的传输通过**自定义的信令**(可以为websocket、http等)实现。
- 地址的获取通过使用 STUN、TURN服务器实现，STUN服务器 用于获取 NAT后的公网地址，而TURN 提供其公网地址，并在服务器上进行转发媒体流服务，用于无法通过STUN 的公网地址建立连接等场景。
- 连接的建立使用ICE实现，ICE中可以保存多个对端地址，作为 candidate，双方交互 ICE candidate，并在建立连接时，对多个地址进行测试，选择可用地址。

## 连接建立后，为保障通信双方可以正常进行媒体交互，需要做什么？
- 需要进行媒体协商，使用SDP进行交互实现，使得双方使用相同规格设置的媒体流进行交互。
- SDP 是什么：
	- session description protocol，作为一种协议，即在进行协商时进行使用。在该协议中使用多个UTF-8字段构成，其中有a行、m行等。
	- a行一般描述网络相关信息，payloadType，带宽等。
	- m行一般描述媒体相关信息，音视频的编码格式，视频level、profile、resolution等。
- SDP 有什么用：
	- 在webRtc中，用来协商媒体格式，通信双方可以交换各自支持的编码格式、媒体类型等信息，以达成一致。
		- 一般流程为：
				- A 生成 offer sdp 并将其设置为 local(A) sdp 
				- A 将 offer sdp 发送至 B
				- B 收到 offer sdp, 生成 remote(B) sdp，并根据offer sdp 生成 answer sdp
				- B 将生成的 answer sdp 作为 local(B) sdp
				- A 将收到的 answer sdp 设置为 remote(A) sdp
	- 虽然sdp中可以携带网络地址，但webRtc中不使用sdp进行传输网络地址，而是使用ICE candidate，因为ICE可以根据移动网络的变化动态更新，而sdp中为静态地址，并且存在防火墙或者NAT后的设备，sdp中地址并非实际地址。



sdp 是否可以携带 iceCandidate？
- 可以
- a=candidate: 方式携带
- 可以通过OnIceCandidate将candidate 填充至 sdp中统一发送。

iceCandidate 里面是什么
- candidate: 字符串类型，表示候选者的 SDP（Session Description Protocol）格式字符串。这个字符串包含了候选者的网络信息，例如 IP 地址、端口和候选者的类型（如主机、反向代理等）。
    
- sdpMid: 字符串类型，标识候选者所对应的媒体流的媒介标识符（Media Identifier）。这用于在多媒体会话中区分不同的媒体流。
    
- sdpMLineIndex: 整数类型，表示候选者在 SDP 中的媒体行索引，指示候选者属于哪个媒体流。
    
- usernameFragment: 字符串类型，表示候选者的用户名片段。它用于标识候选者在连接中的身份。

```
const candidate = new RTCIceCandidate({ candidate: "candidate:842163049 1 udp 1677729535 192.168.1.2 54321 typ host", sdpMid: "0", sdpMLineIndex: 0, usernameFragment: "user1" });
```


#### 源码目录介绍

```cpp
api              ;提供了对外的接口，音视频引擎层和 Module 直接的接口。
audio            ;音频流的一部分抽象，属于引擎的一部分逻辑。
base             ;这一部分还没有学习到，属于 Chromium 项目的一部分，貌似 WebRTC 中用的并不多。
build            ;编译脚本。这里需要注意的是，不同平台的代码在下载的时候，获取的工具集是不一样的。
build_overrides  ;编译工具。
buildtools       ;编译工具链。
call             ;主要是媒体流的接口抽象。为媒体引擎和 codec 层提供桥接。这里说的媒体流是 RTP 流。pc 层也抽象了媒体流，那是编码前、或者解码后。
common_audio     ;音频算法实现，比如 fft。
common_video     ;视频算法实现，比如 h264 协议格式。
data             ;测试数据
examples         ;WebRTC 使用的例子。提供了 peerconnection_client、peerconnection_server、stun、turn 的 demo。
help             ;没有学习到。
infra            ;没有学习到。
logging          ;WebRTC 的 log 库。
media            ;媒体引擎层，包括音频、视频引擎实现。
modules          ;WebRTC 把一些逻辑比较独立的抽象为 Module，利于扩展维护。
ortc             ;媒体描述协议，类似 sdp 协议。
out              ;build 输出目录，这是 webrtc 官方编译指导中示范目录。
p2p              ;主要是实现 candidate 收集，NAT 穿越。
pc               ;实现 jsep 协议。
resources        ;测试数据
rtc_base         ;包括 Socket、线程、锁等 OS 基础功能实现。
rtc_tools        ;网络监测工具、音视频分析工具。很多工具都是脚本实现。
sdk              ;主要是移动端相关实现。
stats            ;WebRTC 统计模块实现。
style-guide      ;编码规范说明
system_wrappers  ;OS 相关功能的封装，比如 cpu、clock 等。
test             ;单元测试代码实现，用 gmock
testing          ;gmock、gtest等源码，属于整个 Chromium 项目。
third_party      ;第三方库依赖。比如，boringssl，abseil-cpp，libvpx等
tools            ;公共工具集，整个 Chromium 项目依赖的。
tools_webrtc     ;WebRTC 用到的工具集。比如代码检查 valgrind 的使用。
video            ;视频 RTP 流的抽象接口，属于视频引擎的一部分。
```



