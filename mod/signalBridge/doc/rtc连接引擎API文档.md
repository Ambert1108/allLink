## rtc连接引擎API文档

- 用于连接终端与 Janus 服务器

#### void connectToJanus(std::string JanusIp, uint16_t JnausPort)
```
- JanusIp: Janus 服务器ip
- JnausPort: Janus 服务器端口
```
- 与 Janus 服务器建立 ws 连接

#### int createSession()

- 创建会话

- 储存`session_id`

- 返回值

  ```
   0: 成功获取 session_id, 正常退出
  -1: 5s内未能获取session_id 
  ```

#### int attachNoSIP()

- 添加NoSIP插件
- 储存`handle_id`
- 返回值:

  ```
   0: 成功获取 handle_id, 正常退出
  -1: 5s内未能获取 handle_id 
  ```

#### void sendGenerate(std::string type, std::string jsep)

```
- type: offer/answer
- jsep: 待转换的 jsep sdp
```

- 向 Janus 发送"generate"请求
- 将输入的 `jsep sdp` 转换为传统 `sdp`
- 收到 `generated` 回复将回调 `OnGenerated()`

#### void sendProcess(std::string type, std::string sdp)

```
- type: offer/answer
- sdp: 待转换的传统 sdp
```

- 向 Janus 发送"process"请求
- 将输入的传统 `sdp` 转换为`jsep sdp`
- 收到 `processed` 回复将回调 `OnProcessed()`

#### void sendTrickle(const webrtc::IceCandidateInterface *candidate);

```
- candidate: 与 OnIceCandidate 参数 candidate 一致
```

- 向 Janus 服务器发送本地 candidate 候选信息
- 在 `OnIceCandidate` 中进行调用

#### void sendTrickleComplete();

- 本地 candidate 候选发送完毕后向 Janus 服务器发送完成标志请求
- 在 `OnIceGatheringChange` 中进行调用

#### virtual void OnGenerated(std::string type, std::string sdp)

```
- type: 回复中的消息类型
- sdp: 转换后得到的传统 sdp
```

- 收到 "generated" 消息的回调函数

#### virtual void OnProcessed(std::string type, std::string jsep)

```
- type: 回复中的消息类型
- jsep: 转换后得到的 jsep
```

- 收到 "processed" 消息的回调函数