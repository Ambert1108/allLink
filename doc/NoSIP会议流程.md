

## NoSIP会议流程梳理

- 连接交互流程

```mermaid
sequenceDiagram
	participant userA
	participant Janus
	participant signaling
	participant 视频MCU
    participant 音频MCU
    Note over userA,signaling: 用户登录
    userA->>signaling: RE(登录)
    activate userA
    activate signaling
    signaling-->>userA: 200 OK
    
    Note over userA,Janus: 创建janus会话
    userA->>Janus: create(获取session_id)
    Janus-->>userA: success(返回session_id)
    activate Janus
    
    Note over userA,Janus: 添加NoSIP插件
    userA->>Janus: attach(添加NoSIP插件)
    Janus-->>userA: success(返回handle_id)
    
    Note over userA,Janus: 给插件发送message（包含sdp）告知Janus开始处理webrtc协商
    userA->>userA: （生成jsep sdp）
    userA->>Janus: message（generate->nosip:将jsep offersdp转换为normal offersdp）
    Janus-->>userA: ack(message的响应)
    Janus-->>userA: event(generated，返回normal sdp)
    userA->>Janus: trickle(send ICE Candidate)
    Janus-->>userA: ack(trickle的响应)
    userA->>Janus: trickle(Candidate completed:true)
    Janus-->>userA: ack(trickle completed:true的响应)
    Note over userA,音频MCU: 创建会议
    userA->>signaling: INVITE(创建会议10156+会议号 normal offersdp)
    signaling->>signaling: （解析normal offersdp的视频ipA、portA和音频ipB、portB）
    signaling-->>userA: 100 Trying
    signaling-->>userA: 180 Ringing
    signaling->>视频MCU: createRoom
    activate 视频MCU
    signaling->>视频MCU: creatChannel(目的地址为ipA，portA，获得收流地址ipE、portE)
    signaling->>音频MCU: create
    activate 音频MCU
    signaling->>音频MCU: add(目的地址为ipB，portB，获得收流地址ipF、portF)
    signaling->>signaling: （使用获得的ip、port替换normal offersdp相应的ip、port生成normal answerSdp）
    signaling-->>userA: 200 OK(normal answerSdp)
    userA->>signaling: ACK
    deactivate signaling

    Note over userA,Janus: ICE通道建立
    userA->>Janus: message（process->nosip:将normal answerSdp转换为jsep answerSdp）
    Janus->>userA: webrtcup （发送webrtcup通知ICE通道建立）
    userA --) Janus: data （发送媒体数据）
    Janus->>userA: media （audio，表示音频数据第一次到达）
    Janus->>userA: media （video，表示视频数据第一次到达）
    Janus --) 视频MCU: data 
    视频MCU --) Janus: data 
    deactivate 视频MCU
    Janus --) 音频MCU: data
    音频MCU --) Janus: data
    deactivate 音频MCU

    Janus --) userA: data
    deactivate Janus

    deactivate userA
   
    


    
```

## 概述

- 通过RTC终端实现基于webrtc/Janus的多人音视频通话

- 支持麦克风、摄像头切换/开关，屏幕共享，系统音频采集，摄像头画面镜像能力

- 依赖webrtc，Janus（nosip），theia7（信令服务器、音视频MCU服务器）



## 形态

- 通过xxx-xxx（例如：111-222）作为会议号
- 在会议中可以分别开启麦克风和摄像头
- 在会议中可以打开屏幕共享和系统音频共享，系统音频共享应该基于屏幕共享开启后开启



## 方案

- 终端发送会议号，由信令服务器确认会议是否存在，并将终端加入会议
- 终端控制麦克风和摄像头设备时，业务消息转发给信令服务器，由信令转发给各MCU，媒体能力由webrtc支持
- 终端集成视频引擎、音频引擎、连接引擎，分别实现音视频控制和Janus交互能力



## 流程

### 创建/加入会议

- `终端`向`Janus`发起创建会话请求，得到sessionId
- `终端`向`Janus`发起注册插件请求，得到handleId
- `终端`开始初始化pc，生成JSEP offer
- `终端`向`Janus`发起generate请求，得到传统 offer
- `终端`向`Janus`发起trickle请求，告知`Janus`终端ICE候选
- `终端`向`信令服务器`发起INVITE请求，告知会议号，得到传统 answer
- `终端`向`信令服务器`发起ACK响应
- `终端`向`Janus`发起process请求，得到JSEP answer
- `终端`完成pc协商，媒体流建立

### 设备控制

- `终端`修改pc，实现摄像头/麦克风控制
- `终端`向`信令服务器`发起info请求

### 媒体控制

- `终端`向`信令服务器`发起info请求，判断是否能够开启屏幕共享
- `终端`根据信令回复修改pc，实现屏幕共享、系统音频共享

### 结束会议

- `终端`向`信令服务器`发起bye请求
- `终端`关闭pc
