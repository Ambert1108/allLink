# Janus NoSIP 插件 demo流程梳理

> Since 2024年11月25日 @Ambert

## 基础概念

NoSIP插件只负责充当RTP桥，信令不在这里传输，完全由程序决定。以下是典型的用法：

1. 应用程序自行处理信令（例如sip），但需要与不支持WebRTC的对等方交互；
2. 使用NoSIP插件创建一个句柄，使用webtc生成offer（JSEP SDP），并将其传递给NoSIP插件；
3. NoSIP创建一个可用于和传统对等方通信的普通SDP，绑定到NoSIP插件自身在服务器上的RTP/RTCP端口（用实现媒体流的转发），并将这个普通SDP发送回程序；
4. 程序将这个普通SDP通过信令发给对等方等体，并等待对方回复；
5. 收到传统对等方的普通SDP，这个SDP不适合WebRTC使用，因此程序将其作为answer发给NoSIP，以匹配之前的offer
6. NoSIP将answer和offer进行匹配，并开始与传统对等方交换RTP/RTCP
   - 来自传统对等方的媒体被转换为WebRTC接受的协议发送到程序
   - 来自程序的WebRTC媒体被转换为RTP/RTCP转发给传统对等方

NoSIP插件本身不处理SIP，而是将信令留给程序。Janus通过NoSIP仅负责桥接媒体。



## Janus接口

### 创建会话

- Request

  ```json
  { 
      "janus":"create",
      "transaction":"B1Ac" //随机字符串
  }
  ```

- Response

  ```json
  { 
          "janus" : "success", 
          "transaction" : "B1Ac", 
          "data" : { 
           "id" : 123456 //唯一整数会话 ID
          } 
  }
  ```

  

创建会话后，Janus会创建一个新的端点以供使用，此端点可以以两种不同的方式使用：

1. 使用无参的GET请求到端点，以接受有关此会话的事件和传入消息的通知；
2. 使用POST请求发送json，将与会话本身交互。

要与会话交互，例如创建一个新的句柄来附加到插件或销毁当前会话，需要向会话端点发送POST JSON消息

### 注册插件

- Request

  ```json
  { 
          "janus" : "attach", 
      	"session_id" : 123456, // 新增！允许 WebSocket 服务器了解此请求属于哪个会话
          "plugin" : "janus.plugin.nosip", //插件的唯一包名称
          "transaction" : "B1Ac"
  }
  ```
  
- Response

  ```json
  { 
          “janus” ："success",
      	"session_id" : 123456, //与请求相同
          “transaction” ："B1Ac", //与请求相同
          “data” ：{ 
                  “id” ：987654 //唯一插件句柄id
          } 
  }
  ```

如果此请求以 POST 形式发送到有效的会话端点，则只有在请求中遗漏任何必填字段或请求的插件在服务器中不可用时，该请求才会失败。

### 销毁会话

- Request

  ```json
  { 
          “janus”：“destroy”，
          “transaction”："B1Ac"
  
  }
  ```

这还将销毁为此会话创建的端点。如果会话当前正在管理一个或多个插件句柄，请确保先销毁它们。服务器在收到会话销毁请求时会尝试自动执行此操作，但客户端更简洁的方法仍有助于避免潜在问题。

注意，会话也可能会在不活动的情况下自动销毁。如果 Janus `session_timeout` 在超过配置值的时间内未收到会话的任何活动（请求、长轮询） 则会话将超时，并触发`timeout`事件。如果`reclaim_session_timeout`配置了值，您仍然可以在有限的时间内使用该请求从相同或不同的传输中回收会话`claim`。超时的未回收会话将被永久销毁，并且还将销毁其所有句柄。因此，应该定期触发用于保持Janus会话活动的临时消息。

### 保持连接

- Request

  ```json
  { 
      "janus":"keepalive",
      "session_id":123456,
      "transaction":"B1Ac"
  }
  ```

- Response

  ```json
  {
  	"janus": "ack",
  	"session_id": 123456,
  	"transaction": "B1Ac"
  }
  ```

这将确保即使没有与句柄交换实际消息，服务器也能检测到会话中的活动。

### 发送trickle（Candidate）

- Request

  ```json
  { 
          "janus" : "trickle",
          “transaction”：“B1Ac”,
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
          "transaction" : "B1Ac",
      	"session_id":123456, //会话id
      	"handle_id":987654,  //插件id
          "candidate" : {
                  "completed" : true
          }
  }
  ```
  
  

## NoSIP接口

NoSIP主要支持两个请求，`generate`和`process`，它们都是异步的。`generate`请求接受 JSEP offer或answer，并生成传统对等方可以使用的普通 SDP；`process`请求处理传统对等方的普通 SDP，并将其与之前可能生成的插件进行匹配，以便返回可用于设置 PeerConnection 的 JSEP offer或answer。

### generate

- Request

  ```json
  { 
      	"janus" : "message",
      	"session_id": 123456,
  		"handle_id": 987654,
          "transaction" : "B1Ac",
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
          "transaction": "B1Ac",
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
          "transaction" : "B1Ac",
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
          "transaction": "B1Ac",
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

  



WebRTC生成的offer示例

```json
v=0
o=- 8358923225229262047 2 IN IP4 127.0.0.1
s=-
t=0 0
a=group:BUNDLE 0 1
a=extmap-allow-mixed
a=msid-semantic: WMS stream_id
m=audio 9 UDP/TLS/RTP/SAVPF 111 63 9 102 0 8 13 110 126
c=IN IP4 0.0.0.0
a=rtcp:9 IN IP4 0.0.0.0
a=ice-ufrag:OWCe
a=ice-pwd:+2aang/WJBvA/IwhdMKZSRNy
a=ice-options:trickle
a=fingerprint:sha-256 37:00:B0:99:8B:5B:14:2C:AC:B3:D6:E6:FE:9B:BE:49:21:D9:72:65:59:07:60:38:CF:21:EE:BB:07:99:2D:FC
a=setup:actpass
a=mid:0
a=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level
a=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
a=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01
a=extmap:4 urn:ietf:params:rtp-hdrext:sdes:mid
a=sendrecv
a=msid:stream_id audio_label
a=rtcp-mux
a=rtcp-rsize
a=rtpmap:111 opus/48000/2
a=rtcp-fb:111 transport-cc
a=fmtp:111 minptime=10;useinbandfec=1
a=rtpmap:63 red/48000/2
a=fmtp:63 111/111
a=rtpmap:9 G722/8000
a=rtpmap:102 ILBC/8000
a=rtpmap:0 PCMU/8000
a=rtpmap:8 PCMA/8000
a=rtpmap:13 CN/8000
a=rtpmap:110 telephone-event/48000
a=rtpmap:126 telephone-event/8000
a=ssrc:605668600 cname:FVW7rDXmTJ0fxuVV
a=ssrc:605668600 msid:stream_id audio_label
m=video 9 UDP/TLS/RTP/SAVPF 96 97 98 99 100 101 39 40 127 103 104
c=IN IP4 0.0.0.0
a=rtcp:9 IN IP4 0.0.0.0
a=ice-ufrag:OWCe
a=ice-pwd:+2aang/WJBvA/IwhdMKZSRNy
a=ice-options:trickle
a=fingerprint:sha-256 37:00:B0:99:8B:5B:14:2C:AC:B3:D6:E6:FE:9B:BE:49:21:D9:72:65:59:07:60:38:CF:21:EE:BB:07:99:2D:FC
a=setup:actpass
a=mid:1
a=extmap:14 urn:ietf:params:rtp-hdrext:toffset
a=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
a=extmap:13 urn:3gpp:video-orientation
a=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01
a=extmap:5 http://www.webrtc.org/experiments/rtp-hdrext/playout-delay
a=extmap:6 http://www.webrtc.org/experiments/rtp-hdrext/video-content-type
a=extmap:7 http://www.webrtc.org/experiments/rtp-hdrext/video-timing
a=extmap:8 http://www.webrtc.org/experiments/rtp-hdrext/color-space
a=extmap:4 urn:ietf:params:rtp-hdrext:sdes:mid
a=extmap:10 urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id
a=extmap:11 urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id
a=sendrecv
a=msid:stream_id video_label
a=rtcp-mux
a=rtcp-rsize
a=rtpmap:96 VP8/90000
a=rtcp-fb:96 goog-remb
a=rtcp-fb:96 transport-cc
a=rtcp-fb:96 ccm fir
a=rtcp-fb:96 nack
a=rtcp-fb:96 nack pli
a=rtpmap:97 rtx/90000
a=fmtp:97 apt=96
a=rtpmap:98 VP9/90000
a=rtcp-fb:98 goog-remb
a=rtcp-fb:98 transport-cc
a=rtcp-fb:98 ccm fir
a=rtcp-fb:98 nack
a=rtcp-fb:98 nack pli
a=fmtp:98 profile-id=0
a=rtpmap:99 rtx/90000
a=fmtp:99 apt=98
a=rtpmap:100 VP9/90000
a=rtcp-fb:100 goog-remb
a=rtcp-fb:100 transport-cc
a=rtcp-fb:100 ccm fir
a=rtcp-fb:100 nack
a=rtcp-fb:100 nack pli
a=fmtp:100 profile-id=2
a=rtpmap:101 rtx/90000
a=fmtp:101 apt=100
a=rtpmap:39 AV1/90000
a=rtcp-fb:39 goog-remb
a=rtcp-fb:39 transport-cc
a=rtcp-fb:39 ccm fir
a=rtcp-fb:39 nack
a=rtcp-fb:39 nack pli
a=fmtp:39 level-idx=5;profile=0;tier=0
a=rtpmap:40 rtx/90000
a=fmtp:40 apt=39
a=rtpmap:127 red/90000
a=rtpmap:103 rtx/90000
a=fmtp:103 apt=127
a=rtpmap:104 ulpfec/90000
a=ssrc-group:FID 854469220 1304095393
a=ssrc:854469220 cname:FVW7rDXmTJ0fxuVV
a=ssrc:854469220 msid:stream_id video_label
a=ssrc:1304095393 cname:FVW7rDXmTJ0fxuVV
a=ssrc:1304095393 msid:stream_id video_label
```



主叫生成的JSEP offer

```json
v=0
o=mozilla...THIS_IS_SDPARTA-99.0 5850086988686651471 0 IN IP4 0.0.0.0
s=-
t=0 0
a=fingerprint:sha-256 A1:81:6A:CC:5E:FD:87:3A:FC:4F:D6:40:AB:63:F7:AE:43:03:AC:CA:10:E8:CB:91:75:80:67:51:25:8B:C2:F1
a=group:BUNDLE 0 1
a=ice-options:trickle
a=msid-semantic:WMS *
m=audio 9 UDP/TLS/RTP/SAVPF 109 9 0 8 101
c=IN IP4 0.0.0.0
a=sendrecv
a=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level
a=extmap:2/recvonly urn:ietf:params:rtp-hdrext:csrc-audio-level
a=extmap:3 urn:ietf:params:rtp-hdrext:sdes:mid
a=fmtp:109 maxplaybackrate=48000;stereo=1;useinbandfec=1
a=fmtp:101 0-15
a=ice-pwd:7f1ba720f525f2ba4899d74f47ddb2d1
a=ice-ufrag:1416ce2f
a=mid:0
a=msid:{fbc20ce0-3fd3-4b30-a97d-62c1764d88bd} {3d0e2d9e-6360-4d7f-9ee1-2e94c718b118}
a=rtcp-mux
a=rtpmap:109 opus/48000/2
a=rtpmap:9 G722/8000/1
a=rtpmap:0 PCMU/8000
a=rtpmap:8 PCMA/8000
a=rtpmap:101 telephone-event/8000/1
a=setup:actpass
a=ssrc:2668424127 cname:{1431bfa4-dfae-4653-a67e-db3a77e6d064}
m=video 9 UDP/TLS/RTP/SAVPF 120 124 121 125 126 127 97 98
c=IN IP4 0.0.0.0
a=sendrecv
a=extmap:3 urn:ietf:params:rtp-hdrext:sdes:mid
a=extmap:4 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
a=extmap:5 urn:ietf:params:rtp-hdrext:toffset
a=extmap:6/recvonly http://www.webrtc.org/experiments/rtp-hdrext/playout-delay
a=extmap:7 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01
a=fmtp:126 profile-level-id=42e01f;level-asymmetry-allowed=1;packetization-mode=1
a=fmtp:97 profile-level-id=42e01f;level-asymmetry-allowed=1
a=fmtp:120 max-fs=12288;max-fr=60
a=fmtp:124 apt=120
a=fmtp:121 max-fs=12288;max-fr=60
a=fmtp:125 apt=121
a=fmtp:127 apt=126
a=fmtp:98 apt=97
a=ice-pwd:7f1ba720f525f2ba4899d74f47ddb2d1
a=ice-ufrag:1416ce2f
a=mid:1
a=msid:{fbc20ce0-3fd3-4b30-a97d-62c1764d88bd} {88b45b88-07fd-4b2d-9a2d-bd346afc859a}
a=rtcp-fb:120 nack
a=rtcp-fb:120 nack pli
a=rtcp-fb:120 ccm fir
a=rtcp-fb:120 goog-remb
a=rtcp-fb:120 transport-cc
a=rtcp-fb:121 nack
a=rtcp-fb:121 nack pli
a=rtcp-fb:121 ccm fir
a=rtcp-fb:121 goog-remb
a=rtcp-fb:121 transport-cc
a=rtcp-fb:126 nack
a=rtcp-fb:126 nack pli
a=rtcp-fb:126 ccm fir
a=rtcp-fb:126 goog-remb
a=rtcp-fb:126 transport-cc
a=rtcp-fb:97 nack
a=rtcp-fb:97 nack pli
a=rtcp-fb:97 ccm fir
a=rtcp-fb:97 goog-remb
a=rtcp-fb:97 transport-cc
a=rtcp-mux
a=rtcp-rsize
a=rtpmap:120 VP8/90000
a=rtpmap:124 rtx/90000
a=rtpmap:121 VP9/90000
a=rtpmap:125 rtx/90000
a=rtpmap:126 H264/90000
a=rtpmap:127 rtx/90000
a=rtpmap:97 H264/90000
a=rtpmap:98 rtx/90000
a=setup:actpass
a=ssrc:1930996731 cname:{1431bfa4-dfae-4653-a67e-db3a77e6d064}
a=ssrc:2326429550 cname:{1431bfa4-dfae-4653-a67e-db3a77e6d064}
a=ssrc-group:FID 1930996731 2326429550
```



NoSIP回复的普通offersdp 

```json
v=0
o=mozilla...THIS_IS_SDPARTA-99.0 5850086988686651471 0 IN IP4 1.1.1.1
s=-
t=0 0
m=audio 20160 RTP/AVP 109 9 0 8 101
c=IN IP4 10.1.29.246
a=sendrecv
a=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level
a=extmap:2/recvonly urn:ietf:params:rtp-hdrext:csrc-audio-level
a=extmap:3 urn:ietf:params:rtp-hdrext:sdes:mid
a=fmtp:109 maxplaybackrate=48000;stereo=1;useinbandfec=1
a=fmtp:101 0-15
a=mid:0
a=msid:{fbc20ce0-3fd3-4b30-a97d-62c1764d88bd} {3d0e2d9e-6360-4d7f-9ee1-2e94c718b118}
a=rtpmap:109 opus/48000/2
a=rtpmap:9 G722/8000/1
a=rtpmap:0 PCMU/8000
a=rtpmap:8 PCMA/8000
a=rtpmap:101 telephone-event/8000/1
m=video 20162 RTP/AVP 120 121 126 97
c=IN IP4 10.1.29.246
a=sendrecv
a=extmap:3 urn:ietf:params:rtp-hdrext:sdes:mid
a=extmap:4 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
a=extmap:5 urn:ietf:params:rtp-hdrext:toffset
a=extmap:6/recvonly http://www.webrtc.org/experiments/rtp-hdrext/playout-delay
a=extmap:7 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01
a=fmtp:126 profile-level-id=42e01f;level-asymmetry-allowed=1;packetization-mode=1
a=fmtp:97 profile-level-id=42e01f;level-asymmetry-allowed=1
a=fmtp:120 max-fs=12288;max-fr=60
a=fmtp:121 max-fs=12288;max-fr=60
a=mid:1
a=msid:{fbc20ce0-3fd3-4b30-a97d-62c1764d88bd} {88b45b88-07fd-4b2d-9a2d-bd346afc859a}
a=rtcp-fb:120 nack
a=rtcp-fb:120 nack pli
a=rtcp-fb:120 ccm fir
a=rtcp-fb:120 goog-remb
a=rtcp-fb:120 transport-cc
a=rtcp-fb:121 nack
a=rtcp-fb:121 nack pli
a=rtcp-fb:121 ccm fir
a=rtcp-fb:121 goog-remb
a=rtcp-fb:121 transport-cc
a=rtcp-fb:126 nack
a=rtcp-fb:126 nack pli
a=rtcp-fb:126 ccm fir
a=rtcp-fb:126 goog-remb
a=rtcp-fb:126 transport-cc
a=rtcp-fb:97 nack
a=rtcp-fb:97 nack pli
a=rtcp-fb:97 ccm fir
a=rtcp-fb:97 goog-remb
a=rtcp-fb:97 transport-cc
a=rtpmap:120 VP8/90000
a=rtpmap:121 VP9/90000
a=rtpmap:126 H264/90000
a=rtpmap:97 H264/90000
```



被叫得到offer后发给NoSIP，NoSIP回复的JSEP offer

```json
v=0
o=mozilla...THIS_IS_SDPARTA-99.0 1732613197965074 1 IN IP4 10.1.29.246
s=-
t=0 0
a=group:BUNDLE 0 1
a=ice-options:trickle
a=fingerprint:sha-256 D2:29:65:DA:4E:59:D6:01:EC:10:4F:82:14:1F:35:9B:D2:FA:AD:6F:C1:48:AD:BF:FD:5A:8B:86:E3:10:0B:C7
a=extmap-allow-mixed
a=msid-semantic: WMS *
m=audio 9 UDP/TLS/RTP/SAVPF 109 9 0 8 101
c=IN IP4 10.1.29.246
a=sendrecv
a=mid:0
a=rtcp-mux
a=ice-ufrag:0Gwi
a=ice-pwd:S+EeY15fpAwEv1No50xYto
a=ice-options:trickle
a=setup:actpass
a=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level
a=extmap:2/recvonly urn:ietf:params:rtp-hdrext:csrc-audio-level
a=extmap:3 urn:ietf:params:rtp-hdrext:sdes:mid
a=fmtp:109 maxplaybackrate=48000;stereo=1;useinbandfec=1
a=fmtp:101 0-15
a=rtpmap:109 opus/48000/2
a=rtpmap:9 G722/8000/1
a=rtpmap:0 PCMU/8000
a=rtpmap:8 PCMA/8000
a=rtpmap:101 telephone-event/8000/1
a=msid:{fbc20ce0-3fd3-4b30-a97d-62c1764d88bd} {3d0e2d9e-6360-4d7f-9ee1-2e94c718b118}
a=ssrc:28136521 cname:janus
a=candidate:1 1 udp 2015363327 10.1.29.246 41890 typ host
a=end-of-candidates
m=video 9 UDP/TLS/RTP/SAVPF 120 121 126 97 122 123 127 98
c=IN IP4 10.1.29.246
a=sendrecv
a=mid:1
a=rtcp-mux
a=ice-ufrag:0Gwi
a=ice-pwd:S+EeY15fpAwEv1No50xYto
a=ice-options:trickle
a=setup:actpass
a=extmap:3 urn:ietf:params:rtp-hdrext:sdes:mid
a=extmap:4 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
a=extmap:5 urn:ietf:params:rtp-hdrext:toffset
a=extmap:6/recvonly http://www.webrtc.org/experiments/rtp-hdrext/playout-delay
a=extmap:7 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01
a=fmtp:126 profile-level-id=42e01f;level-asymmetry-allowed=1;packetization-mode=1
a=fmtp:97 profile-level-id=42e01f;level-asymmetry-allowed=1
a=fmtp:120 max-fs=12288;max-fr=60
a=fmtp:121 max-fs=12288;max-fr=60
a=rtcp-fb:120 nack
a=rtcp-fb:120 nack pli
a=rtcp-fb:120 ccm fir
a=rtcp-fb:120 goog-remb
a=rtcp-fb:120 transport-cc
a=rtcp-fb:121 nack
a=rtcp-fb:121 nack pli
a=rtcp-fb:121 ccm fir
a=rtcp-fb:121 goog-remb
a=rtcp-fb:121 transport-cc
a=rtcp-fb:126 nack
a=rtcp-fb:126 nack pli
a=rtcp-fb:126 ccm fir
a=rtcp-fb:126 goog-remb
a=rtcp-fb:126 transport-cc
a=rtcp-fb:97 nack
a=rtcp-fb:97 nack pli
a=rtcp-fb:97 ccm fir
a=rtcp-fb:97 goog-remb
a=rtcp-fb:97 transport-cc
a=rtpmap:120 VP8/90000
a=rtpmap:121 VP9/90000
a=rtpmap:126 H264/90000
a=rtpmap:97 H264/90000
a=rtpmap:122 rtx/90000
a=fmtp:122 apt=120
a=rtpmap:123 rtx/90000
a=fmtp:123 apt=121
a=rtpmap:127 rtx/90000
a=fmtp:127 apt=126
a=rtpmap:98 rtx/90000
a=fmtp:98 apt=97
a=ssrc-group:FID 3076014647 2442178148
a=msid:{fbc20ce0-3fd3-4b30-a97d-62c1764d88bd} {88b45b88-07fd-4b2d-9a2d-bd346afc859a}
a=ssrc:3076014647 cname:janus
a=ssrc:2442178148 cname:janus
a=candidate:1 1 udp 2015363327 10.1.29.246 41890 typ host
a=end-of-candidates

```

被叫生成的JSEP answer

```json
v=0
o=mozilla...THIS_IS_SDPARTA-99.0 1186317856315596638 0 IN IP4 0.0.0.0
s=-
t=0 0
a=fingerprint:sha-256 4C:E8:19:82:4D:1E:C2:D1:22:85:C6:5B:75:A5:4D:9C:51:7D:0F:0C:40:6E:51:01:48:E6:36:7F:A6:15:47:05
a=group:BUNDLE 0 1
a=ice-options:trickle
a=msid-semantic:WMS *
m=audio 9 UDP/TLS/RTP/SAVPF 109 9 0 8 101
c=IN IP4 0.0.0.0
a=sendrecv
a=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level
a=extmap:3 urn:ietf:params:rtp-hdrext:sdes:mid
a=fmtp:109 maxplaybackrate=48000;stereo=1;useinbandfec=1
a=fmtp:101 0-15
a=ice-pwd:95247bdbdff28e3ecb3ad6425b30ed2a
a=ice-ufrag:787e4e7d
a=mid:0
a=msid:{1bc995ce-63c3-465e-91b5-101b93d4183f} {c4a6cb40-ce73-48fa-9354-2480b005c1c6}
a=rtcp-mux
a=rtpmap:109 opus/48000/2
a=rtpmap:9 G722/8000/1
a=rtpmap:0 PCMU/8000
a=rtpmap:8 PCMA/8000
a=rtpmap:101 telephone-event/8000/1
a=setup:active
a=ssrc:490000387 cname:{7eb975fc-e80c-4fdf-b2f1-dcbe9ba7074b}
m=video 9 UDP/TLS/RTP/SAVPF 120 122 121 123 126 127 97 98
c=IN IP4 0.0.0.0
a=sendrecv
a=extmap:3 urn:ietf:params:rtp-hdrext:sdes:mid
a=extmap:4 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
a=extmap:5 urn:ietf:params:rtp-hdrext:toffset
a=extmap:7 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01
a=fmtp:126 profile-level-id=42e01f;level-asymmetry-allowed=1;packetization-mode=1
a=fmtp:97 profile-level-id=42e01f;level-asymmetry-allowed=1
a=fmtp:120 max-fs=12288;max-fr=60
a=fmtp:122 apt=120
a=fmtp:121 max-fs=12288;max-fr=60
a=fmtp:123 apt=121
a=fmtp:127 apt=126
a=fmtp:98 apt=97
a=ice-pwd:95247bdbdff28e3ecb3ad6425b30ed2a
a=ice-ufrag:787e4e7d
a=mid:1
a=msid:{1bc995ce-63c3-465e-91b5-101b93d4183f} {ab385243-ea9b-4a39-8f8f-bb5897b6b42e}
a=rtcp-fb:120 nack
a=rtcp-fb:120 nack pli
a=rtcp-fb:120 ccm fir
a=rtcp-fb:120 goog-remb
a=rtcp-fb:120 transport-cc
a=rtcp-fb:121 nack
a=rtcp-fb:121 nack pli
a=rtcp-fb:121 ccm fir
a=rtcp-fb:121 goog-remb
a=rtcp-fb:121 transport-cc
a=rtcp-fb:126 nack
a=rtcp-fb:126 nack pli
a=rtcp-fb:126 ccm fir
a=rtcp-fb:126 goog-remb
a=rtcp-fb:126 transport-cc
a=rtcp-fb:97 nack
a=rtcp-fb:97 nack pli
a=rtcp-fb:97 ccm fir
a=rtcp-fb:97 goog-remb
a=rtcp-fb:97 transport-cc
a=rtcp-mux
a=rtpmap:120 VP8/90000
a=rtpmap:122 rtx/90000
a=rtpmap:121 VP9/90000
a=rtpmap:123 rtx/90000
a=rtpmap:126 H264/90000
a=rtpmap:127 rtx/90000
a=rtpmap:97 H264/90000
a=rtpmap:98 rtx/90000
a=setup:active
a=ssrc:736722097 cname:{7eb975fc-e80c-4fdf-b2f1-dcbe9ba7074b}
a=ssrc:1114047617 cname:{7eb975fc-e80c-4fdf-b2f1-dcbe9ba7074b}
a=ssrc-group:FID 736722097 1114047617
```

NoSIP回复的普通answerSdp

```json
v=0
o=mozilla...THIS_IS_SDPARTA-99.0 1186317856315596638 0 IN IP4 1.1.1.1
s=-
t=0 0
m=audio 20164 RTP/AVP 109 9 0 8 101
c=IN IP4 10.1.29.246
a=sendrecv
a=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level
a=extmap:3 urn:ietf:params:rtp-hdrext:sdes:mid
a=fmtp:109 maxplaybackrate=48000;stereo=1;useinbandfec=1
a=fmtp:101 0-15
a=mid:0
a=msid:{1bc995ce-63c3-465e-91b5-101b93d4183f} {c4a6cb40-ce73-48fa-9354-2480b005c1c6}
a=rtpmap:109 opus/48000/2
a=rtpmap:9 G722/8000/1
a=rtpmap:0 PCMU/8000
a=rtpmap:8 PCMA/8000
a=rtpmap:101 telephone-event/8000/1
m=video 20166 RTP/AVP 120 121 126 97
c=IN IP4 10.1.29.246
a=sendrecv
a=extmap:3 urn:ietf:params:rtp-hdrext:sdes:mid
a=extmap:4 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
a=extmap:5 urn:ietf:params:rtp-hdrext:toffset
a=extmap:7 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01
a=fmtp:126 profile-level-id=42e01f;level-asymmetry-allowed=1;packetization-mode=1
a=fmtp:97 profile-level-id=42e01f;level-asymmetry-allowed=1
a=fmtp:120 max-fs=12288;max-fr=60
a=fmtp:121 max-fs=12288;max-fr=60
a=mid:1
a=msid:{1bc995ce-63c3-465e-91b5-101b93d4183f} {ab385243-ea9b-4a39-8f8f-bb5897b6b42e}
a=rtcp-fb:120 nack
a=rtcp-fb:120 nack pli
a=rtcp-fb:120 ccm fir
a=rtcp-fb:120 goog-remb
a=rtcp-fb:120 transport-cc
a=rtcp-fb:121 nack
a=rtcp-fb:121 nack pli
a=rtcp-fb:121 ccm fir
a=rtcp-fb:121 goog-remb
a=rtcp-fb:121 transport-cc
a=rtcp-fb:126 nack
a=rtcp-fb:126 nack pli
a=rtcp-fb:126 ccm fir
a=rtcp-fb:126 goog-remb
a=rtcp-fb:126 transport-cc
a=rtcp-fb:97 nack
a=rtcp-fb:97 nack pli
a=rtcp-fb:97 ccm fir
a=rtcp-fb:97 goog-remb
a=rtcp-fb:97 transport-cc
a=rtpmap:120 VP8/90000
a=rtpmap:121 VP9/90000
a=rtpmap:126 H264/90000
a=rtpmap:97 H264/90000
```

主叫得到answer后发给NoSIP，NoSIP回复的JSEP answer

```json
v=0
o=mozilla...THIS_IS_SDPARTA-99.0 1732613198385934 1 IN IP4 10.1.29.246
s=-
t=0 0
a=group:BUNDLE 0 1
a=ice-options:trickle
a=fingerprint:sha-256 D2:29:65:DA:4E:59:D6:01:EC:10:4F:82:14:1F:35:9B:D2:FA:AD:6F:C1:48:AD:BF:FD:5A:8B:86:E3:10:0B:C7
a=extmap-allow-mixed
a=msid-semantic: WMS *
m=audio 9 UDP/TLS/RTP/SAVPF 109 9 0 8 101
c=IN IP4 10.1.29.246
a=sendrecv
a=mid:0
a=rtcp-mux
a=ice-ufrag:Qpgf
a=ice-pwd:TG230Yo1C1nH0yC77xNHWr
a=ice-options:trickle
a=setup:active
a=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level
a=extmap:3 urn:ietf:params:rtp-hdrext:sdes:mid
a=fmtp:109 maxplaybackrate=48000;stereo=1;useinbandfec=1
a=fmtp:101 0-15
a=mid:0
a=rtpmap:109 opus/48000/2
a=rtpmap:9 G722/8000/1
a=rtpmap:0 PCMU/8000
a=rtpmap:8 PCMA/8000
a=rtpmap:101 telephone-event/8000/1
a=msid:{1bc995ce-63c3-465e-91b5-101b93d4183f} {c4a6cb40-ce73-48fa-9354-2480b005c1c6}
a=ssrc:2753164113 cname:janus
a=candidate:1 1 udp 2015363327 10.1.29.246 35424 typ host
a=end-of-candidates
m=video 9 UDP/TLS/RTP/SAVPF 120 121 126 97 124 125 127 98
c=IN IP4 10.1.29.246
a=sendrecv
a=mid:1
a=rtcp-mux
a=ice-ufrag:Qpgf
a=ice-pwd:TG230Yo1C1nH0yC77xNHWr
a=ice-options:trickle
a=setup:active
a=extmap:3 urn:ietf:params:rtp-hdrext:sdes:mid
a=extmap:4 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
a=extmap:5 urn:ietf:params:rtp-hdrext:toffset
a=extmap:7 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01
a=fmtp:126 profile-level-id=42e01f;level-asymmetry-allowed=1;packetization-mode=1
a=fmtp:97 profile-level-id=42e01f;level-asymmetry-allowed=1
a=fmtp:120 max-fs=12288;max-fr=60
a=fmtp:121 max-fs=12288;max-fr=60
a=mid:1
a=rtcp-fb:120 nack
a=rtcp-fb:120 nack pli
a=rtcp-fb:120 ccm fir
a=rtcp-fb:120 goog-remb
a=rtcp-fb:120 transport-cc
a=rtcp-fb:121 nack
a=rtcp-fb:121 nack pli
a=rtcp-fb:121 ccm fir
a=rtcp-fb:121 goog-remb
a=rtcp-fb:121 transport-cc
a=rtcp-fb:126 nack
a=rtcp-fb:126 nack pli
a=rtcp-fb:126 ccm fir
a=rtcp-fb:126 goog-remb
a=rtcp-fb:126 transport-cc
a=rtcp-fb:97 nack
a=rtcp-fb:97 nack pli
a=rtcp-fb:97 ccm fir
a=rtcp-fb:97 goog-remb
a=rtcp-fb:97 transport-cc
a=rtpmap:120 VP8/90000
a=rtpmap:121 VP9/90000
a=rtpmap:126 H264/90000
a=rtpmap:97 H264/90000
a=rtpmap:124 rtx/90000
a=fmtp:124 apt=120
a=rtpmap:125 rtx/90000
a=fmtp:125 apt=121
a=rtpmap:127 rtx/90000
a=fmtp:127 apt=126
a=rtpmap:98 rtx/90000
a=fmtp:98 apt=97
a=ssrc-group:FID 2854938885 36873820
a=msid:{1bc995ce-63c3-465e-91b5-101b93d4183f} {ab385243-ea9b-4a39-8f8f-bb5897b6b42e}
a=ssrc:2854938885 cname:janus
a=ssrc:36873820 cname:janus
a=candidate:1 1 udp 2015363327 10.1.29.246 35424 typ host
a=end-of-candidates
```

