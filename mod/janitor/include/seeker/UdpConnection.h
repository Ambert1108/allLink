/**
  文件注释样例，参考： http://www.edparrish.net/common/cppdoc.html
  socket连接流程封装

  @project netTester
  @author Tao Zhang, Tao, Tao
  @since 2020/4/26
  @version 0.1.3 2020/4/30
*/
#pragma once
#include "socketUtil.h"
#include <string>


using std::string;


class UdpConnection {
  int localPort{0};
  int remotePort{0};

  string localIp{""};
  string remoteIp{""};

  bool inited{false};
  SOCKET sock{0};


  sockaddr_in localAddr{};

 public:
  sockaddr_in remoteAddr{};
  sockaddr_in lastAddr{};
  socklen_t lastAddrLen = sizeof(lastAddr);

  UdpConnection();
  void updateRemoteAddr(const sockaddr_in& addr);
  void updateRemoteAddr(uint16_t port, string ip);
  void updateRemoteAddr();

  UdpConnection& setLocalIp(const string& ip);
  UdpConnection& setLocalPort(int port);


  UdpConnection& setRemoteIp(const string& ip);
  string getRemoteIp() { return remoteIp; }
  UdpConnection& setRemotePort(int port);
  int getRemotePort() { return remotePort; }

  string getLastIp() { return inet_ntoa(lastAddr.sin_addr); }
  int getLastPort() { return ntohs(lastAddr.sin_port); }
  int getLocalPort(){
      struct sockaddr_in connAddr;
      socklen_t len = sizeof connAddr;
      int ret = getsockname(sock, (sockaddr*)&connAddr, &len);
      if (0 != ret) {
          return -1;
      }
      return ntohs(connAddr.sin_port); // 获取端口号
  }
  UdpConnection& init();
  void close();

  SOCKET getSocket();

  void sendData(char* buf, size_t len);

  void reply(char* buf, size_t len);

  int recvData(char* buf, size_t len);
};