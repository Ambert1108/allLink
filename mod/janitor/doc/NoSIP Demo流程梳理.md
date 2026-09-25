##  NoSIP Demo流程梳理

### 基础概念

NoSIP插件只负责充当RTP桥，信令不在这里传输，完全由程序决定。大概流程是：

1. 客户端A 自行处理信令，并且要与不支持WebRTC 的对等方（另一个客户端B）交互；
2. 使用NoSIP 插件创建一个句柄，使用webRTC 生成offer（JSEP SDP），并通过message 信息（generate）将其传递给NoSIP 插件；
3. NoSIP 创建一个可用于和传统对等方通信的普通SDP ，绑定到NoSIP 插件自身在服务器上的RTP/RTCP 端口（用实现媒体流的转发），并将这个普通SDP 发送回客户端A ；
4. 客户端A 将这个普通SDP通过信令发给对等方（客户端B），并等待对方回复；
5. 客户端A 收到传统对等方（客户端B）的普通SDP，这个SDP不适合WebRTC使用，因此客户端A 将其作为answer通过message信息（process）发给NoSIP，以匹配之前的offer
6. NoSIP 将answer 和offer 进行匹配，并开始与传统对等方（客户端B）交换RTP/RTCP
   - 来自传统对等方（客户端B）的媒体被转换为WebRTC 接受的协议发送到客户端A 
   - 来自客户端A 的WebRTC 媒体被转换为RTP/RTCP 转发给传统对等方（客户端B）


## Janus接口

### 创建会话

会话是用来管理客户端和服务器之间的通信的，相当于上下文，创建会话是与janus交互的第一步

- Request

  ```json
  { 
      "janus":"create",
      "transaction":"a1b2c3d4" //请求的唯一标识符：随机字符串
  }
  ```

- Response

  ```json
  { 
          "janus" : "success", 
          "transaction" : "a1b2c3d4", 
          "data" : { 
           "id" : 123456 //会话的唯一标识符：整数ID
          } 
  }
  ```

 

### 注册插件

会话可以管理多个插件（不同插件可以实现不同功能），要使用插件，需要先把会话附加到插件上，即：需要创建一个新的插件对象

- Request

  ```json
  { 
        "janus" : "attach", 
      	"session_id" : 12345,           // 新增！ 会话ID
        "plugin" : "janus.plugin.nosip", //插件的唯一名称
        "transaction" : "a1b2c3d4"
  }
  ```

- Response

  ```json
  { 
        “janus” ："success",
      	 "session_id" : 12345,           //会话ID
        “transaction” ："a1b2c3d4",  //与请求相同
        “data” ：{ 
                  “id” ：98765       //唯一的插件对象标识符：插件ID
          } 
  }
  ```

### 销毁会话

- Request

  ```json
  { 
          “janus”：“destroy”，
           "session_id" : 12345,           //需要销毁的会话ID
          “transaction”："a1b2c3d4"
  
  }
  ```

如果会话当前正在管理一个或多个插件句柄，需要先销毁它们。

会话也可能会在不活动的情况下自动销毁。
    -  Janus `session_timeout` 在超过配置值的时间内未收到会话的任何活动（请求、长轮询） 则会话将超时，并触发`timeout`事件。
    -  如果`reclaim_session_timeout`配置了值，您仍然可以在有限的时间内使用该请求从相同或不同的传输中回收会话`claim`。超时的未回收会话将被永久销毁，并且还将销毁其所有句柄。
因此，应该定期触发用于保持Janus会话活动的临时消息。

### 保持连接

- Request

  ```json
  { 
      "janus":"keepalive",
      "session_id":123456,
      "transaction":"a1b2c3d4"
  }
  ```

- Response

  ```json
  {
  	"janus": "ack",
  	"session_id": 123456,
  	"transaction": "a1b2c3d4"
  }
  ```

这将确保即使没有与句柄交换实际消息，服务器也能检测到会话中的活动。

### 发送trickle（Candidate）

- Request

  ```json
  { 
          "janus" : "trickle",
          “transaction”：“a1b2c3d4”,
      	"session_id":123456, //会话id
      	"handle_id":987654,  //插件id
          “candidate”：{ 
                  “sdpMid”："video",
                  “sdpMLineIndex”：1,
                  “candidate”："..."
          } 
  }
  ```

- 收集并发送完成

  ```json
  {
          "janus" : "trickle",
          "transaction" : "a1b2c3d4",
      	"session_id":123456, //会话id
      	"handle_id":987654,  //插件id
          "candidate" : {
                  "completed" : true
          }
  }
  ```

  

## NoSIP接口

NoSIP主要支持两个请求，`generate`和`process`，它们都是异步的。
- `generate`请求接受 JSEP offer或answer，并生成传统对等方可以使用的普通 SDP；
- `process`请求处理传统对等方的普通 SDP，并将其与之前可能生成的插件进行匹配，以便返回可用于设置 PeerConnection 的 JSEP offer或answer。

### generate

- Request

  ```json
  { 
      	"janus" : "message",
      	"session_id": 123456,
  		"handle_id": 987654,
          "transaction" : "a1b2c3d4",
      	"body" : {
              "request" : "generate", 
              "info" : "<用户可以提供上下文的不透明字符串；可选>", 
              "srtp" : "<是否强制 (sdes_mandatory) 或提供 (sdes_optional) SRTP 支持；可选>", 
              "srtp_profile" : "<如果提供 SRTP，则协商 SRTP 配置文件；可选>"
          },
          "jsep": {
              "sdp": "......",
              "type": "<offer|answer，取决于提供的 SDP 的性质>"
          }
  }
  ```

- Response

  ```json
  { 
      	"janus": "event",
          "session_id": 123456,
          "transaction": "a1b2c3d4",
          "sender": 987654,
          "plugindata": {
              "plugin": "janus.plugin.nosip",
              "data" : {
                  "nosip":"event",
                  "result" : {
                     "event" : "generated", 
                      "type" : "<offer|answer，取决于提供的 JSEP 的性质>", 
                      "sdp" : "<普通 SDP 内容>"  
                  }
              }
          }
  }
  ```

### process

- Request

  ```json
  { 
      	"janus" : "message",
      	"session_id": 123456,
  		"handle_id": 987654,
          "transaction" : "a1b2c3d4",
      	"body" : {
            	"request" : "process", 
              "type" : "<offer|answer，取决于提供的 SDP 的性质>", 
              "sdp" : "<要转换的普通 SDP 内容>",
              "info" : "<用户可以为上下文提供的不透明字符串；可选>", 
              "srtp" : "<是否强制 (sdes_mandatory) 或提供 (sdes_optional) SRTP 支持；可选>", 
              "srtp_profile" : "<要协商的 SRTP 配置文件，如果提供 SRTP；可选>"   
          }
  }
  ```

- Response

  ```json
  {
      	"janus": "event",
          "session_id": 123456,
          "transaction": "a1b2c3d4",
          "sender": 987654,
          "plugindata": {
              "plugin": "janus.plugin.nosip",
              "data" : {
                  "nosip":"event",
                  "result" : {
                      "event" : "processed",
                      "srtp" : "<是否强制 (sdes_mandatory) 或提供 (sdes_optional) SRTP 支持；可选>"
                  }
              }
          },
          "jsep": {
                  "sdp": "......",
                  "type": "<offer|answer，取决于提供的 SDP 的性质>"
          }
  }
  ```

  
