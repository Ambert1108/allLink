#pragma once
#include "savequeue.h"

namespace wt {
	struct Message {
		int id;
		void* data;
	};

  class hi {
  public:
    // 静态方法
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
    static zx::ThreadSafeQueue<Message> messageQueue;
  };
}