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
  VisionCentralContoller vcc;
  SignlingInteractionSystem client;
  auto controller = rtc::make_ref_counted<Controller>(&client, &vcc);
  vcc.run();
  return 0;
}
std::string replace_with_integer_parts(const std::string& input) {
  // 正则表达式匹配小数点及其后的数字
  std::regex re(R"((\d+)\.\d+)");
  // 替换为只保留整数部分
  std::string output = std::regex_replace(input, re, "$1");
  return output;
}

//int main() {
//  std::string input = "11.41-13.12";
//  std::string result = replace_with_integer_parts(input);
//  std::cout << result << std::endl; // 输出: 11-13
//  return 0;
//}