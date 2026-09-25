#pragma once
#include <string>

#include "absl/flags/flag.h"

extern const uint16_t kDefaultServerPort;  // From defaults.[h|cc]

// Define flags for the peerconnect_client testing tool, in a separate
// header file so that they can be shared across the different main.cc's
// for each platform.

ABSL_FLAG(bool,
  autoconnect,
  false,
  "Connect to the server without user "
  "intervention.");
ABSL_FLAG(std::string, server, "localhost", "The server to connect to.");
ABSL_FLAG(int,
  port,
  kDefaultServerPort,
  "The port on which the server is listening.");
ABSL_FLAG(
  bool,
  autocall,
  false,
  "Call the first available other client on "
  "the server without user intervention.  Note: this flag should only be set "
  "to true on one of the two clients.");

ABSL_FLAG(
  std::string,
  force_fieldtrials,
  "",
  "Field trials control experimental features. This flag specifies the field "
  "trials in effect. E.g. running with "
  "--force_fieldtrials=WebRTC-FooFeature/Enabled/ "
  "will assign the group Enabled to field trial WebRTC-FooFeature. Multiple "
  "trials are separated by \"/\"");