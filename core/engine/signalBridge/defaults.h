//
// Created by 姚惠晶 on 2024/11/5.
//

#ifndef BASICWEBRTC_DEFAULTS_H
#define BASICWEBRTC_DEFAULTS_H


#include <stdint.h>

#include <string>

extern const char kAudioLabel[];
extern const char kVideoLabel[];
extern const char kStreamId[];
extern const uint16_t kDefaultServerPort;

std::string GetEnvVarOrDefault(const char* env_var_name,
                               const char* default_value);
std::string GetPeerConnectionString();
std::string GetDefaultServerName();
std::string GetPeerName();



#endif //BASICWEBRTC_DEFAULTS_H
