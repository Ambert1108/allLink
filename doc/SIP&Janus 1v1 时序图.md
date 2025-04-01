# SIP 1v1时序图

> Since @Ambert 2025年3月31日
>
> 基于sip协议实现信令交互；
>
> 基于Janus-nosip插件实现rtp中继；


```mermaid
sequenceDiagram
  actor Alice
  participant signaling
  participant caller janus
  participant callee janus
  actor Bob
  Note over Alice,signaling: 用户登录
  Alice->>signaling: REGISTER
  activate Alice
  activate signaling
  signaling-->>Alice: 200 ok
  
  Note over Alice,Bob: 建立呼叫
  Alice-->>Alice: init PeerconnectionFactory
  Alice-->>Alice: create Peerconnection
  Alice-->>Alice: add audio&video track
  Alice-->>Alice: create JSEP offer&setLocalDesc
  Alice-->>signaling: F1 INVITE (JSEP offer)
  signaling->>caller janus: create session
  caller janus->>signaling: success（返回session_id）
  Alice-->>Alice: generate ICE Candidate
  signaling->>caller janus: attach NoSIP
  caller janus->>signaling: success（返回handle_id）
  Alice-->>signaling: F2 INFO（trickle）
  signaling->>caller janus: trickle（send ICE Candidate）
  signaling->>caller janus: message（generate：JSEP offer）
  caller janus->>signaling: event（generated：Normal offer）
  signaling->>callee janus: create session
  callee janus->>signaling: success（返回session_id）
  signaling->>callee janus: attach NoSIP
  callee janus->>signaling: success（返回handle_id）
  signaling->>callee janus: message（process：Normal offer）
  callee janus->>signaling: event（processed：JSEP offer）
  signaling-->>Bob: F3 INVITE (JSEP offer)
  Bob-->>Bob: setRemoteDesc
  Bob-->>Bob: init PeerconnectionFactory
  Bob-->>Bob: create Peerconnection
  signaling-->>Alice: F4 100 Trying
  Bob-->>Bob: add audio&video track
  Bob-->>Bob: create JSEP answer&setLocalDesc
  Bob-->>signaling: F5 180 Ringing (JSEP answer)
  Bob-->>Bob: generate ICE Candidate
  signaling-->>Alice: F6 180 Ringing
  Bob-->>signaling: F7 INFO（trickle）
  signaling->>callee janus: trickle（send ICE Candidate）
  signaling->>callee janus: message（generate：JSEP anser）
  callee janus->>signaling: event（generated：Normal anser）
  Bob-->>signaling: F8 200 OK
  signaling->>caller janus: message（process：Normal anser）
  caller janus->>signaling: event（processed：JSEP anser）
  signaling-->>Alice: F9 200 OK（JSEP anser）
  Alice-->>Alice: setRemoteDesc
  Alice->>Bob: F10 ACK
  
  Note over Alice,Bob: 开始通话
  caller janus->>signaling: webrtcup
  signaling-->>Alice: F10 INFO（webrtcup）
  caller janus->>signaling: media
  signaling-->>Alice: F11 INFO（media）
  callee janus->>signaling: webrtcup
  signaling-->>Bob: F12 INFO（webrtcup）
  callee janus->>signaling: media
  signaling-->>Bob: F13 INFO（media）
  
```



