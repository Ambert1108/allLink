### echoTest 梳理

普通http rest接口

- 获取janus实例信息

  向http://yourserver:8088/janus/info发送get消息

- The server root

  创建janus会话

  ```
  {
          "janus" : "create",
          "transaction" : "<random alphanumeric string>"
  }
  ```

  返回

  ```
  {
          "janus" : "success",
          "transaction" : "<same as the request>",
          "data" : {
                  "id" : <unique integer session ID>
          }
  }
  ```

  

- The session endpoint

  一旦创建了会话，就会在服务器中通过返回的会话标识符创建一个可以使用的新端点。这个端点可以以两种不同的方式使用

  - 发送GET（从插件获取事件和消息的长轮询）
  - POST（创建插件句柄或操作会话）

- The plugin handle endpoint

  一旦创建了插件句柄，就会在服务器中创建一个可以使用的新端点。新端点是通过连接服务器根、会话标识符和返回的新插件句柄标识符来构建的。可以使用这个插件句柄处理与插件通信相关的所有事情，即，向插件发送消息，协商连接到插件的WebRTC连接，等等。

