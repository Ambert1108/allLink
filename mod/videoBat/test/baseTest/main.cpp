#include <windows.h>
#include <shellapi.h>

#include <string>
#include <vector>

#include "absl/flags/parse.h"
#include "conductor.h"
//#include "examples/peerconnection/client/flag_defs.h"
#include "main_wnd.h"
#include "peerconnection.h"
#include "rtc_base/checks.h"
#include "rtc_base/ssl_adapter.h"
#include "rtc_base/string_utils.h"  // For ToUtf8
#include "rtc_base/win32_socket_init.h"
#include "system_wrappers/include/field_trial.h"
#include "test/field_trial.h"

class CustomSocketServer : public  rtc::PhysicalSocketServer {
public:
  bool Wait(webrtc::TimeDelta max_wait_duration, bool process_io) override {
    if (!process_io)
      return true;

    return rtc::PhysicalSocketServer::Wait(webrtc::TimeDelta::Zero(), process_io);
  }

};


int main() {
  rtc::WinsockInitializer winsock_init;

  CustomSocketServer ss;
  rtc::AutoSocketServerThread main_thread(&ss);

  // 输入服务器ip及端口
  std::string server;
  int port;
  std::cin >> server >> port;


  if (port < 1 || port>65535) {
    std::cout << "invalid port" << std::endl;
    return -1;
  }

  MainWnd wnd(server.c_str(), port, false, false);
  if (!wnd.Create()) {
    RTC_DCHECK_NOTREACHED();
    return -1;
  }


  rtc::InitializeSSL();

  //创建一个peerConnection 客户端
  PeerConnectionClient client;

  auto conductor = rtc::make_ref_counted<Conductor>(&client, &wnd);

  main_thread.Start();
  //conductor->StartLogin(server, port);
  // Main loop.
  MSG msg;
  BOOL gm;
  wnd.PreTranslateMessage();
  //while (client.is_connected()) {
  //  for (const auto& it : client.peers()) {
  //    I_LOG("peerId={}", it.first);
  //  }
  //}
  while (true) {
    if ((gm = ::GetMessage(&msg, NULL, 0, 0)) != 0 && gm != -1) {
      if (!wnd.PreTranslateMessage(&msg)) {
        ::TranslateMessage(&msg); 
        ::DispatchMessage(&msg);
      }
    }
  }

  rtc::CleanupSSL();

  return 0;
}

