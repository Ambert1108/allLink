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
#include <any>

#include "controller.h"
#include "vision.h"
#include "signling.h"

#include "seeker/logger.h"
#include "seeker/loggerApi.h"

int main(int argc, char* argv[]) {
  std::string logPattern = "%^[%d %H:%M:%S.%e %s:%#] [%L]:%$ %v";
  std::string logFilename = "webrtc.log";
  int level = 1;
  seeker::Logger::init(logFilename, false, true, true, logPattern, level);
  using namespace alllink;
  seeker::IniConfig::init("./resources/config.ini");
  oatpp::base::Environment::init();
  {
    VisionCentralContoller vcc;
    SignlingInteractionSystem client;
    auto controller = rtc::make_ref_counted<Controller>(&client, &vcc);
    vcc.run();
  }
  oatpp::base::Environment::destroy();
  return 0;
}