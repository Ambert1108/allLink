#pragma once
#include "savequeue.h"

#include <any>
#include <unordered_map>

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
		
		MEETING_END,                    //通知中控器通话应该结束
		SWITCH_AUDIO_INPUT,             //通知中控器切换音频输入设备
		SWITCH_AUDIO_INPUT_STR,         //通知中控器切换音频输入设备字符版本
		SWITCH_MIC_VOLUME,              //通知中控器修改麦克风输入音量
		SET_MIC_PHONE,                  //通知中控器设置麦克风状态
		SET_CAMERA,                     //通知中控器设置摄像头状态
		SET_SHARE,                      //通知中控器设置屏幕共享状态
		REQUEST_IFRAME,                 //通知中控器请求I帧

		/* 信令消息 */

		LOGIN_SUCCESS,
		MEETING_OK,
		CALL_MODE,
		PEER_RINGING,

		/* 中控器消息 */

		SEND_SDP_TO_PEER,               //发送offer/answer sdp给对端
		SEND_ICE_TO_PEER,               //发送ice candidate给对端
		SEND_ICE_COMPLETE_TO_PEER,      //发送ice candidate收集完毕消息给对端
		SEND_MSG_FAILED,                //发送offer/answer sdp或ice candidate失败
		ADD_TRACK,                      //添加新轨道
		REMOVE_TRACK,                   //移除轨道
		DISCONNECT_PEER,                //通知中控器断开连接
		RECONNECT_SERVER,               //通知中控器重新连接信令服务器和Janus服务器
		RECONNECT_SERVER_FAILED,        //通知视觉控制器重新连接信令服务器失败
		RECONNECT_PEER,                 //通知中控器重新连接对端
		AUDIO_DEV_INFO                  //通知视觉控制器读取音频设备信息
	};

	static constexpr int msgTo(MessageType msg) { return static_cast<int>(msg); }

	static std::string enumToString(MessageType e) {
		static const std::unordered_map<MessageType, std::string> enumMap = {
				{MessageType::SWITCH_AUDIO_INPUT, "SWITCH_AUDIO_INPUT"},
				{MessageType::SWITCH_AUDIO_INPUT_STR, "SWITCH_AUDIO_INPUT_STR"},
				{MessageType::SWITCH_MIC_VOLUME, "SWITCH_MIC_VOLUME"},
				{MessageType::SET_MIC_PHONE, "SET_MIC_PHONE"},
				{MessageType::SET_CAMERA, "SET_CAMERA"},
				{MessageType::SET_SHARE, "SET_SHARE"},
				{MessageType::SEND_SDP_TO_PEER, "SEND_SDP_TO_PEER"},
				{MessageType::SEND_ICE_TO_PEER, "SEND_ICE_TO_PEER"},
				{MessageType::SEND_ICE_COMPLETE_TO_PEER, "SEND_ICE_COMPLETE_TO_PEER"},
				{MessageType::DISCONNECT_PEER, "DISCONNECT_PEER"},
				{MessageType::RECONNECT_SERVER, "RECONNECT_SERVER"},
				{MessageType::RECONNECT_PEER, "RECONNECT_PEER"},
				{MessageType::MEETING_OK, "MEETING_OK"},
				{MessageType::PEER_RINGING, "PEER_RINGING"}
		};
		auto it = enumMap.find(e);
		if (it != enumMap.end()) {
			return it->second;
		}
		return "Unknown";
	}
}