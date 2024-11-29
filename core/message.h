#pragma once
#include "savequeue.h"
#include <any>

namespace alllink {
	struct Message {
		int id;
		std::any data;
	};

  class hi {
  public:
    static void PostMsg(const Message& message) {
      messageQueue.Push(message);
    }

    static bool GetMsg(Message& message) {
      return messageQueue.TryPop(message);
    }

    static bool GetMsgBlock(Message& message) {
      return messageQueue.WaitPop(message);
    }

  private:
    // 静态线程安全的队列实例
    static base::ThreadSafeQueue<Message> messageQueue;
  };

	enum class MessageType : int {
		/*全局消息载体*/

		/* 开始窗口消息 */

		CREATE_MEETING = 0,
		JOIN_MEETING,
		START_LOGIN,
		START_LOGOUT,
		SETTING,

		/* 登录窗口消息 */

		IS_LOGIN,

		/* 加入会议窗口消息 */

		IS_ENTER,

		/* 会议窗口消息 */
		
		MEETING_END,

		/* 信令消息 */

		LOGIN_SUCCESS,

		/* 中控器消息 */

		SET_REMOTE_DESC,                //通知中控器设置远端sdp
		SEND_JSEP_SDP_TO_PEER,          //发送jsep offer/answer sdp给对端
		SEND_PROCESS_TO_JANUS,          //发送process请求给Janus服务器
		SEND_SDP_TO_PEER,               //发送offer/answer sdp给对端
		SEND_ICE_TO_PEER,               //发送ice candidate给对端
		SEND_ICE_COMPLETE_TO_PEER,      //发送ice candidate收集完毕消息给对端
		SEND_MSG_FAILED,                //发送offer/answer sdp或ice candidate失败
		ADD_TRACK,                      //添加新轨道
		REMOVE_TRACK                    //移除轨道
	};

	static constexpr int msgTo(MessageType msg) { return static_cast<int>(msg); }
}