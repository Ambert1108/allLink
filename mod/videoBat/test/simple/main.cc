#include <Windows.h>
#include <shellapi.h>  
#include <string>
#include <vector>
#include "absl/flags/parse.h"
#include "conductor.h"
#include "flag_defs.h"
#include "main_wnd.h"
#include "peer_connection_client.h"
#include "rtc_base/checks.h"
#include "rtc_base/ssl_adapter.h"
#include "rtc_base/string_utils.h"
#include "rtc_base/win32_socket_init.h"
#include "system_wrappers/include/field_trial.h"
#include "test/field_trial.h"
#include "seeker/logger.h"

namespace {
    class CustomSocketServer :public rtc::PhysicalSocketServer {
    public:
        bool Wait(webrtc::TimeDelta max_wait_duration, bool process_io) override {
            if (!process_io)
                return true;

            return rtc::PhysicalSocketServer::Wait(webrtc::TimeDelta::Zero(), process_io);
        }
    };
}

int main() {
    seeker::Logger::init("simpleWebrtc.log", false, true, true, "");
    std::string ip = "10.4.6.254";
    int port = 8888;
    if (SDL_Init(SDL_INIT_VIDEO))  // 初始化SDL视频模块
    {
        E_LOG("SDL_Init fuc fail");
    }

    rtc::WinsockInitializer winsock_init;//初始化网络

    CustomSocketServer ss;// 自定义 Socket 服务器
    rtc::AutoSocketServerThread main_thread(&ss);//处理网络事件同步异步// 使用自定义 Socket 服务器创建主线程

    MainWnd wnd(ip.c_str(), port,// 创建主窗口
        false, false);
    if (!wnd.Create()) {
        RTC_DCHECK_NOTREACHED();// 如果窗口创建失败，则终止程序
        return -1;
    }

    rtc::InitializeSSL();// 初始化 SSL
    PeerConnectionClient client;// 创建 PeerConnectionClient 对象
    auto conductor = rtc::make_ref_counted<Conductor>(&client, &wnd);// 创建 Conductor 对象

    main_thread.Start();
    // Main loop.
    MSG msg;
    BOOL gm;
    wnd.PreTranslateMessage();
    std::thread t;
    while (1) { // 获取并处理消息，如果获取失败或者程序接收到退出消息，则退出循环
        if ((gm = ::GetMessage(&msg, NULL, 0, 0)) != 0 && gm != -1) {
            if (!wnd.PreTranslateMessage(&msg)) {// 如果消息没有被预处理
                I_LOG("translate Message");
                ::TranslateMessage(&msg); // 翻译消息
                ::DispatchMessage(&msg);// 分发消息
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

  rtc::CleanupSSL();// 清理 SSL
  return 0;
}
