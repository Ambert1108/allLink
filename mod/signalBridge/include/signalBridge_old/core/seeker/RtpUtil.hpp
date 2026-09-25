/**
@project ffmpegSDK
@author Tao Zhang
@since 2022/5/11
@version 0.1.0-SNAPSHOT 2022/11/19
*/



#pragma once

//#include "seeker/common.h"
#include "common.h"


namespace rtp {

struct RtpHeader {
  uint8_t version{2};
  uint8_t padding{0};
  uint8_t extension{0};
  uint8_t csrcCount{0};
  uint8_t marker{0};
  uint8_t payloadType{0};
  uint16_t seqNum{0};
  uint32_t timestamp{0};
  uint32_t ssrc{0};
};



inline bool getMark(const uint8_t* rtpHeader) {
  using seeker::ByteArray;
  uint8_t secondByte{0x00};
  ByteArray::readData(rtpHeader + 1, secondByte, false);
  bool marker = (secondByte >> 7) & 0x01;
  return marker;
}


inline int getPayloadType(const uint8_t* rtpHeader) {
  using seeker::ByteArray;
  uint8_t secondByte{0x00};
  ByteArray::readData(rtpHeader + 1, secondByte, false);
  int payloadType = (secondByte >> 0) & 0x7F;
  return payloadType;
}


inline uint16_t getSeq(const uint8_t* rtpHeader) {
  using seeker::ByteArray;
  constexpr uint8_t POSITION = 2;
  uint16_t seqNum{0};
  ByteArray::readData(rtpHeader + POSITION, seqNum, false);
  return seqNum;
}


inline uint32_t getTimestamp(const uint8_t* rtpHeader) {
  using seeker::ByteArray;
  constexpr uint8_t POSITION = 4;
  uint32_t timestamp{0};
  ByteArray::readData(rtpHeader + POSITION, timestamp, false);
  return timestamp;
}


inline uint32_t getSsrc(const uint8_t* rtpHeader) {
  using seeker::ByteArray;
  constexpr uint8_t POSITION = 8;
  uint32_t ssrc{0};
  ByteArray::readData(rtpHeader + POSITION, ssrc, false);
  return ssrc;
}



inline RtpHeader getRtpHeader(const uint8_t* rtpHeader) {
  using seeker::ByteArray;
  auto* buf = rtpHeader;
  uint8_t firstByte{0x00};
  uint8_t secondByte{0x00};

  RtpHeader header;

  size_t pos{0};
  ByteArray::readData(buf + pos, firstByte, false);
  pos += sizeof(firstByte);
  ByteArray::readData(buf + pos, secondByte, false);
  pos += sizeof(secondByte);

  header.version = (firstByte >> 6) & 0x03;
  header.padding = (firstByte >> 5) & 0x01;
  header.extension = (firstByte >> 4) & 0x01;
  header.csrcCount = (firstByte >> 0) & 0x0F;
  header.marker = (secondByte >> 7) & 0x01;
  header.payloadType = (secondByte >> 0) & 0x7F;

  if (header.extension) {
    throw std::runtime_error("do not supported header extension.");
  }

  ByteArray::readData(buf + pos, header.seqNum, false);
  pos += sizeof(header.seqNum);
  ByteArray::readData(buf + pos, header.timestamp, false);
  pos += sizeof(header.timestamp);
  ByteArray::readData(buf + pos, header.ssrc, false);
  pos += sizeof(header.ssrc);


  return header;
}



}  // namespace rtp
