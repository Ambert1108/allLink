### echotest demo

- 初始化janus

- 创建一个janus

  - 创建会话

    ```
    回复
    {
        "janus": "success",
        "transaction": "YLtelpVYYm0T",
        "data": {
            "id": 7699687077699805
        }
    }
    ```

  - 建立ws连接

    - error

      连接错误

    - open

      连接成功

      回调success(1)

      调用ws.send发送创建会话请求

    - message

      当ws接收到消息, 则会调用该函数

      将接收到的json格式解析并传递至	'handleEvent'

    - close

      关闭连接

- success(1)

  - janus.attach()

    createHandle

    - 发送attach请求

      ```
      回复
      {
          "janus": "success",
          "session_id": 7699687077699805,
          "transaction": "7pqLIPwipTZp",
          "data": {
              "id": 3206578430923509
          }
      }
      ```

      

    - 创建插件对象

    - 回调success(2)

    

- success(2)

  - 发送message消息给janus

    ```
    {
        "janus": "message",
        "body": {
            "audio": true,
            "video": true
        },
        "transaction": "ZjMOxhjSiRGL",
        "session_id": 7699687077699805,
        "handle_id": 3206578430923509
    }
    ```

    

  - createOffer（prepareWebrtc）

    - create Peerconnection

      - 准备local sdp，获取candidates

    - await captureDevices 添加媒体通道

    - 如果jsep为空

      - createOffer

        peerConnection生成offerSdp

        peerConnection setLocalDescription

      - 发送sdp

        ```
        {
            "janus": "message",
            "body": {
                "audio": true,
                "video": true
            },
            "transaction": "B6Aso3gDzP2L",
            "jsep": {
                "type": "offer",
                "sdp": "v=0\r\no=- 7735913891996476422 2 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\na=group:BUNDLE 0 1 2\r\na=extmap-allow-mixed\r\na=msid-semantic: WMS 6891bcff-f605-4b83-981f-195298a4f05d\r\nm=audio 9 UDP/TLS/RTP/SAVPF 111 63 9 0 8 13 110 126\r\nc=IN IP4 0.0.0.0\r\na=rtcp:9 IN IP4 0.0.0.0\r\na=ice-ufrag:73fb\r\na=ice-pwd:uIH1WQBExpXCz8fI/e4idm8O\r\na=ice-options:trickle\r\na=fingerprint:sha-256 \r\nm=video 9 UDP/TLS/RTP/SAVPF 96 97 102 103 104 105 106 107 108 109 127 125 39 40 45 46 98 99 100 101 112 113 114\r\nc=IN IP4 0.0.0.0\r\na=rtcp:9 IN IP4 0.0.0.0\r\na=ice-ufrag:73fb\r\na=ice-pwd:uIH1WQBExpXCz8fI/e4idm8O\r\na=ice-options:trickle\r\na=fingerprint:sha-256"
            },
            "session_id": 7699687077699805,
            "handle_id": 3206578430923509
        }
        ```

        

    - 如果jsep不为空（被叫？）

      - peerConnection setRemoteDescription
      - peerConnection addIceCandidate
      - await captureDevices 添加媒体通道
      - await createAnswer
      - 发送sdp

- handleEvent

  - 如果json["janus"] === "event"

    ```
    {
        "janus": "event",
        "session_id": 7699687077699805,
        "transaction": "B6Aso3gDzP2L",
        "sender": 3206578430923509,
        "plugindata": {
            "plugin": "janus.plugin.echotest",
            "data": {
                "echotest": "event",
                "result": "ok"
            }
        },
        "jsep": {
            "type": "answer",
            "sdp": "v=0\r\no=- 7735913891996476422 2 IN IP4 10.1.29.246\r\ns=-\r\nt=0 0\r\na=group:BUNDLE 0 1 2\r\na=ice-options:trickle\r\na=fingerprint:sha-256 D2:29:65:DA:4E:59:D6:01:EC:10:4F:82:14:1F:35:9B:D2:FA:AD:6F:C1:48:AD:BF:FD:5A:8B:86:E3:10:0B:C7\r\na=extmap-allow-mixed\r\na=msid-semantic: WMS *\r\nm=audio 9 UDP/TLS/RTP/SAVPF 111\r\nc=IN IP4 10.1.29.246\r\na=sendrecv\r\na=mid:0\r\na=rtcp-mux\r\na=ice-host\r\na=end-of-candidates\r\nm=video 9 UDP/TLS/RTP/SAVPF 96 97\r\nc=IN IP4 10.1.29.246\r\na=sendrecv\r\na=mid:1\r\na=rtcp-mux\r\na=ice-ufrag:ov1A\r\na=ice-cname:janus\r\na=candidate:1 1 udp 2015363327 10.1.29.246 39067 typ host\r\na=end-of-candidates\r\nm=application 9 UDP/DTLS/SCTP webrtc-datachannel\r\nc=IN IP4 10.1.29.246\r\na=sendrecv\r\na=mid:2\r\na=sctp-port:5000\r\na=ice-ufrag:ov1A\r\na=ice-pwd:75fqKyNnlqIJsA9jYHFgEU\r\na=ice-options:trickle\r\na=setup:active\r\na=candidate:1 1 udp 2015363327 10.1.29.246 39067 typ host\r\na=end-of-candidates\r\n"
        }
    }
    ```

    

    - 回调onmessage
      - 如果jsep不为空 handleRemoteJsep（prepareWebrtcPeer）
      - peerConnection setRemoteDescription
      - peerConnection addIceCandidate





- send trickle在sendsdp后



### 流程

```
1. 创建会话
{"janus":"create","transaction":"xF7mJ610KKGR"}
{
   "janus": "success",
   "transaction": "xF7mJ610KKGR",
   "data": {
      "id": 1094223938572966
   }
}

2. 创建插件
{"janus":"attach","plugin":"janus.plugin.echotest","opaque_id":"echotest-XCUrM4wMRgs9","transaction":"NnnnVsGDAdkn","session_id":1094223938572966}
{
   "janus": "success",
   "session_id": 1094223938572966,
   "transaction": "NnnnVsGDAdkn",
   "data": {
      "id": 2296016276064636
   }
}

3.发送message
{"janus":"message","body":{"audio":true,"video":true},"transaction":"l1oqdKrwqzj2","session_id":1094223938572966,"handle_id":2296016276064636}
{
   "janus": "ack",
   "session_id": 1094223938572966,
   "transaction": "l1oqdKrwqzj2",
   "hint": "I'm taking my time!"
}
{
   "janus": "event",
   "session_id": 1094223938572966,
   "transaction": "l1oqdKrwqzj2",
   "sender": 2296016276064636,
   "plugindata": {
      "plugin": "janus.plugin.echotest",
      "data": {
         "echotest": "event",
         "result": "ok"
      }
   }
}

4.发送message，含offerSdp
{"janus":"message","body":{"audio":true,"video":true},"transaction":"1DbeMSUMVDrs","jsep":{"type":"offer","sdp":"v=0\r\no=- 1703371300741671035 2 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\na=group:BUNDLE 0 1 2\r\na=extmap-allow-mixed\r\na=msid-semantic: WMS 1f09c46b-0a9b-4008-a0fa-ec5f135c3360\r\nm=audio 9 UDP/TLS/RTP/SAVPF 111 63 9 0 8 13 110 126\r\nc=IN IP4 0.0.0.0\r\na=rtcp:9 IN IP4 0.0.0.0\r\na=ice-ufrag:bDFA\r\na=ice-pwd:geKqt3/uCc9Stxe9j8tfu2vc\r\na=ice-options:trickle\r\na=fingerprint:sha-256 BD:8C:1C:EC:E5:AF:DA:C1:0A:62:2F:67:96:5D:47:AB:F4:9E:D3:BF:E4:E6:AC:C9:50:5E:9B:BC:7C:E4:59:22\r\na=setup:actpass\r\na=mid:0\r\na=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level\r\na=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\r\na=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01\r\na=extmap:4 urn:ietf:params:rtp-hdrext:sdes:mid\r\na=sendrecv\r\na=msid:1f09c46b-0a9b-4008-a0fa-ec5f135c3360 d8e2c63b-7326-4d1b-bad7-b2b3edfb85a6\r\na=rtcp-mux\r\na=rtcp-rsize\r\na=rtpmap:111 opus/48000/2\r\na=rtcp-fb:111 transport-cc\r\na=fmtp:111 minptime=10;useinbandfec=1\r\na=rtpmap:63 red/48000/2\r\na=fmtp:63 111/111\r\na=rtpmap:9 G722/8000\r\na=rtpmap:0 PCMU/8000\r\na=rtpmap:8 PCMA/8000\r\na=rtpmap:13 CN/8000\r\na=rtpmap:110 telephone-event/48000\r\na=rtpmap:126 telephone-event/8000\r\na=ssrc:3160529075 cname:NZV0kG3BHD+jEeFV\r\na=ssrc:3160529075 msid:1f09c46b-0a9b-4008-a0fa-ec5f135c3360 d8e2c63b-7326-4d1b-bad7-b2b3edfb85a6\r\nm=video 9 UDP/TLS/RTP/SAVPF 96 97 102 103 104 105 106 107 108 109 127 125 39 40 45 46 98 99 100 101 112 113 114\r\nc=IN IP4 0.0.0.0\r\na=rtcp:9 IN IP4 0.0.0.0\r\na=ice-ufrag:bDFA\r\na=ice-pwd:geKqt3/uCc9Stxe9j8tfu2vc\r\na=ice-options:trickle\r\na=fingerprint:sha-256 BD:8C:1C:EC:E5:AF:DA:C1:0A:62:2F:67:96:5D:47:AB:F4:9E:D3:BF:E4:E6:AC:C9:50:5E:9B:BC:7C:E4:59:22\r\na=setup:actpass\r\na=mid:1\r\na=extmap:14 urn:ietf:params:rtp-hdrext:toffset\r\na=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\r\na=extmap:13 urn:3gpp:video-orientation\r\na=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01\r\na=extmap:5 http://www.webrtc.org/experiments/rtp-hdrext/playout-delay\r\na=extmap:6 http://www.webrtc.org/experiments/rtp-hdrext/video-content-type\r\na=extmap:7 http://www.webrtc.org/experiments/rtp-hdrext/video-timing\r\na=extmap:8 http://www.webrtc.org/experiments/rtp-hdrext/color-space\r\na=extmap:4 urn:ietf:params:rtp-hdrext:sdes:mid\r\na=extmap:10 urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id\r\na=extmap:11 urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id\r\na=sendrecv\r\na=msid:1f09c46b-0a9b-4008-a0fa-ec5f135c3360 2bbbfff0-b222-43ec-8d6f-f570457ac0ce\r\na=rtcp-mux\r\na=rtcp-rsize\r\na=rtpmap:96 VP8/90000\r\na=rtcp-fb:96 goog-remb\r\na=rtcp-fb:96 transport-cc\r\na=rtcp-fb:96 ccm fir\r\na=rtcp-fb:96 nack\r\na=rtcp-fb:96 nack pli\r\na=rtpmap:97 rtx/90000\r\na=fmtp:97 apt=96\r\na=rtpmap:102 H264/90000\r\na=rtcp-fb:102 goog-remb\r\na=rtcp-fb:102 transport-cc\r\na=rtcp-fb:102 ccm fir\r\na=rtcp-fb:102 nack\r\na=rtcp-fb:102 nack pli\r\na=fmtp:102 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=42001f\r\na=rtpmap:103 rtx/90000\r\na=fmtp:103 apt=102\r\na=rtpmap:104 H264/90000\r\na=rtcp-fb:104 goog-remb\r\na=rtcp-fb:104 transport-cc\r\na=rtcp-fb:104 ccm fir\r\na=rtcp-fb:104 nack\r\na=rtcp-fb:104 nack pli\r\na=fmtp:104 level-asymmetry-allowed=1;packetization-mode=0;profile-level-id=42001f\r\na=rtpmap:105 rtx/90000\r\na=fmtp:105 apt=104\r\na=rtpmap:106 H264/90000\r\na=rtcp-fb:106 goog-remb\r\na=rtcp-fb:106 transport-cc\r\na=rtcp-fb:106 ccm fir\r\na=rtcp-fb:106 nack\r\na=rtcp-fb:106 nack pli\r\na=fmtp:106 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=42e01f\r\na=rtpmap:107 rtx/90000\r\na=fmtp:107 apt=106\r\na=rtpmap:108 H264/90000\r\na=rtcp-fb:108 goog-remb\r\na=rtcp-fb:108 transport-cc\r\na=rtcp-fb:108 ccm fir\r\na=rtcp-fb:108 nack\r\na=rtcp-fb:108 nack pli\r\na=fmtp:108 level-asymmetry-allowed=1;packetization-mode=0;profile-level-id=42e01f\r\na=rtpmap:109 rtx/90000\r\na=fmtp:109 apt=108\r\na=rtpmap:127 H264/90000\r\na=rtcp-fb:127 goog-remb\r\na=rtcp-fb:127 transport-cc\r\na=rtcp-fb:127 ccm fir\r\na=rtcp-fb:127 nack\r\na=rtcp-fb:127 nack pli\r\na=fmtp:127 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=4d001f\r\na=rtpmap:125 rtx/90000\r\na=fmtp:125 apt=127\r\na=rtpmap:39 H264/90000\r\na=rtcp-fb:39 goog-remb\r\na=rtcp-fb:39 transport-cc\r\na=rtcp-fb:39 ccm fir\r\na=rtcp-fb:39 nack\r\na=rtcp-fb:39 nack pli\r\na=fmtp:39 level-asymmetry-allowed=1;packetization-mode=0;profile-level-id=4d001f\r\na=rtpmap:40 rtx/90000\r\na=fmtp:40 apt=39\r\na=rtpmap:45 AV1/90000\r\na=rtcp-fb:45 goog-remb\r\na=rtcp-fb:45 transport-cc\r\na=rtcp-fb:45 ccm fir\r\na=rtcp-fb:45 nack\r\na=rtcp-fb:45 nack pli\r\na=fmtp:45 level-idx=5;profile=0;tier=0\r\na=rtpmap:46 rtx/90000\r\na=fmtp:46 apt=45\r\na=rtpmap:98 VP9/90000\r\na=rtcp-fb:98 goog-remb\r\na=rtcp-fb:98 transport-cc\r\na=rtcp-fb:98 ccm fir\r\na=rtcp-fb:98 nack\r\na=rtcp-fb:98 nack pli\r\na=fmtp:98 profile-id=0\r\na=rtpmap:99 rtx/90000\r\na=fmtp:99 apt=98\r\na=rtpmap:100 VP9/90000\r\na=rtcp-fb:100 goog-remb\r\na=rtcp-fb:100 transport-cc\r\na=rtcp-fb:100 ccm fir\r\na=rtcp-fb:100 nack\r\na=rtcp-fb:100 nack pli\r\na=fmtp:100 profile-id=2\r\na=rtpmap:101 rtx/90000\r\na=fmtp:101 apt=100\r\na=rtpmap:112 red/90000\r\na=rtpmap:113 rtx/90000\r\na=fmtp:113 apt=112\r\na=rtpmap:114 ulpfec/90000\r\na=ssrc-group:FID 3649275046 3987062648\r\na=ssrc:3649275046 cname:NZV0kG3BHD+jEeFV\r\na=ssrc:3649275046 msid:1f09c46b-0a9b-4008-a0fa-ec5f135c3360 2bbbfff0-b222-43ec-8d6f-f570457ac0ce\r\na=ssrc:3987062648 cname:NZV0kG3BHD+jEeFV\r\na=ssrc:3987062648 msid:1f09c46b-0a9b-4008-a0fa-ec5f135c3360 2bbbfff0-b222-43ec-8d6f-f570457ac0ce\r\nm=application 9 UDP/DTLS/SCTP webrtc-datachannel\r\nc=IN IP4 0.0.0.0\r\na=ice-ufrag:bDFA\r\na=ice-pwd:geKqt3/uCc9Stxe9j8tfu2vc\r\na=ice-options:trickle\r\na=fingerprint:sha-256 BD:8C:1C:EC:E5:AF:DA:C1:0A:62:2F:67:96:5D:47:AB:F4:9E:D3:BF:E4:E6:AC:C9:50:5E:9B:BC:7C:E4:59:22\r\na=setup:actpass\r\na=mid:2\r\na=sctp-port:5000\r\na=max-message-size:262144\r\n"},"session_id":1094223938572966,"handle_id":2296016276064636}

5. 发送trickle
{"janus":"trickle","candidate":{"candidate":"candidate:2121371881 1 udp 2122260223 26.26.26.1 55873 typ host generation 0 ufrag bDFA network-id 2 network-cost 50","sdpMid":"0","sdpMLineIndex":0},"transaction":"BsuFcbuNyhAB","session_id":1094223938572966,"handle_id":2296016276064636}

收到ack
{
   "janus": "ack",
   "session_id": 1094223938572966,
   "transaction": "BsuFcbuNyhAB"
}

6.收到answerSdp
{
   "janus": "event",
   "session_id": 1094223938572966,
   "transaction": "1DbeMSUMVDrs",
   "sender": 2296016276064636,
   "plugindata": {
      "plugin": "janus.plugin.echotest",
      "data": {
         "echotest": "event",
         "result": "ok"
      }
   },
   "jsep": {
      "type": "answer",
      "sdp": "v=0\r\no=- 1703371300741671035 2 IN IP4 10.1.29.246\r\ns=-\r\nt=0 0\r\na=group:BUNDLE 0 1 2\r\na=ice-options:trickle\r\na=fingerprint:sha-256 D2:29:65:DA:4E:59:D6:01:EC:10:4F:82:14:1F:35:9B:D2:FA:AD:6F:C1:48:AD:BF:FD:5A:8B:86:E3:10:0B:C7\r\na=extmap-allow-mixed\r\na=msid-semantic: WMS *\r\nm=audio 9 UDP/TLS/RTP/SAVPF 111\r\nc=IN IP4 10.1.29.246\r\na=sendrecv\r\na=mid:0\r\na=rtcp-mux\r\na=ice-ufrag:/lpx\r\na=ice-pwd:rysfgPW+f3DOZ74lhXezrI\r\na=ice-options:trickle\r\na=setup:active\r\na=rtpmap:111 opus/48000/2\r\na=fmtp:111 useinbandfec=1\r\na=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level\r\na=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\r\na=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01\r\na=extmap:4 urn:ietf:params:rtp-hdrext:sdes:mid\r\na=msid:janus janus0\r\na=ssrc:2861915288 cname:janus\r\na=candidate:1 1 udp 2015363327 10.1.29.246 42140 typ host\r\na=end-of-candidates\r\nm=video 9 UDP/TLS/RTP/SAVPF 96 97\r\nc=IN IP4 10.1.29.246\r\na=sendrecv\r\na=mid:1\r\na=rtcp-mux\r\na=ice-ufrag:/lpx\r\na=ice-pwd:rysfgPW+f3DOZ74lhXezrI\r\na=ice-options:trickle\r\na=setup:active\r\na=rtpmap:96 VP8/90000\r\na=rtcp-fb:96 ccm fir\r\na=rtcp-fb:96 nack\r\na=rtcp-fb:96 nack pli\r\na=rtcp-fb:96 goog-remb\r\na=rtcp-fb:96 transport-cc\r\na=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\r\na=extmap:13 urn:3gpp:video-orientation\r\na=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01\r\na=extmap:5 http://www.webrtc.org/experiments/rtp-hdrext/playout-delay\r\na=extmap:4 urn:ietf:params:rtp-hdrext:sdes:mid\r\na=extmap:10 urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id\r\na=extmap:11 urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id\r\na=rtpmap:97 rtx/90000\r\na=fmtp:97 apt=96\r\na=ssrc-group:FID 1040951604 1965591214\r\na=msid:janus janus1\r\na=ssrc:1040951604 cname:janus\r\na=ssrc:1965591214 cname:janus\r\na=candidate:1 1 udp 2015363327 10.1.29.246 42140 typ host\r\na=end-of-candidates\r\nm=application 9 UDP/DTLS/SCTP webrtc-datachannel\r\nc=IN IP4 10.1.29.246\r\na=sendrecv\r\na=mid:2\r\na=sctp-port:5000\r\na=ice-ufrag:/lpx\r\na=ice-pwd:rysfgPW+f3DOZ74lhXezrI\r\na=ice-options:trickle\r\na=setup:active\r\na=candidate:1 1 udp 2015363327 10.1.29.246 42140 typ host\r\na=end-of-candidates\r\n"
   }
}

7. 收到webrtcup
{
   "janus": "webrtcup",
   "session_id": 1094223938572966,
   "sender": 2296016276064636
}
```





































