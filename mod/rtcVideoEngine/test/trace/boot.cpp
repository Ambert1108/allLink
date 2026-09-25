#include <Windows.h>
#include <shellapi.h>  // must come after windows.h
// clang-format on

#include <iostream>
#include <conio.h>
#include <fcntl.h>
#include <io.h>
#include <cctype>
#include <string>
#include <vector>
#include <conio.h>

#include "controller.h"
#include "defines.h"
#include "presenter.h"
#include "mediaclient.h"

#include "seeker/logger.h"
#include "seeker/loggerApi.h"

#include "absl/flags/parse.h"
#include "rtc_base/checks.h"
#include "rtc_base/ssl_adapter.h"
#include "rtc_base/string_utils.h"  // For ToUtf8
#include "rtc_base/win32_socket_init.h"
#include "system_wrappers/include/field_trial.h"
#include "test/field_trial.h"
#include "api/async_dns_resolver.h"
#include "rtc_base/async_dns_resolver.h"

namespace {
  class CustomSocketServer :public rtc::PhysicalSocketServer {
  public:
    bool Wait(webrtc::TimeDelta max_wait_duration, bool process_io) override {
      if (!process_io)
        return true;

      return rtc::PhysicalSocketServer::Wait(webrtc::TimeDelta::Zero(), process_io);
    }
  };

  std::string bindIp = "127.0.0.1";
  int32_t bindPort = 8888;

}  // namespace

int main(int argc, char* argv[]) {
  rtc::WinsockInitializer winsock_init;
  CustomSocketServer ss;
  rtc::AutoSocketServerThread main_thread(&ss);
  std::string logPattern = "%^[%d %H:%M:%S.%e %s:%#] [%L]:%$ %v";
  std::string logFilename = "webrtc.log";
  int level = 1;
  seeker::Logger::init(logFilename, false, true, true, logPattern, level);
  using namespace wt;
  MainWnd wnd(bindIp.c_str(), bindPort);
  if (!wnd.Create()) {
    RTC_DCHECK_NOTREACHED();
    return -1;
  }
  rtc::InitializeSSL();
  MediaClient client;
  auto controller = rtc::make_ref_counted<Controller>(&client, &wnd);
  main_thread.Start();

  //MSG msg;
  Message msg;
  while (true) {
    if(hi::GetMsg(msg)) {
      D_LOG("get msg");
      if (!wnd.PreTranslateMessage(msg)) continue;
    }
    // TODO:将输入的字符传入gui中解析处理
    D_LOG("main work");
    if (!wnd.PreTranslateMessage()) break;
    Sleep(1);
  }
  
  rtc::CleanupSSL();
  return 0;
}
