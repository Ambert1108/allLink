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
		/*登录窗口消息载体*/

		/* 开始窗口消息 */

		CREATE_MEETING = 0,
		JOIN_MEETING,
		START_LOGIN,
		START_LOGOUT,
		SETTING,

		/* 登录窗口消息 */
		IS_LOGIN,

		/* 信令消息 */
		LOGIN_SUCCESS
	};

	static constexpr int msgTo(MessageType msg) { return static_cast<int>(msg); }
}