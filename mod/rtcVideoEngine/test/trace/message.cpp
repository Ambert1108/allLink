#include "message.h"

namespace wt {
  // 在类外初始化静态成员变量
  zx::ThreadSafeQueue<Message> hi::messageQueue;
}