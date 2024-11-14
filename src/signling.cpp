#include "signling.h"

#include "api/units/time_delta.h"
#include "rtc_base/async_dns_resolver.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/net_helpers.h"
#include "rtc_base/thread.h"


namespace alllink {
  SignlingInteractionSystem::SignlingInteractionSystem() {

  }

  SignlingInteractionSystem::~SignlingInteractionSystem() = default;

  bool SignlingInteractionSystem::isConnected() const {
    return true;
  }

  void SignlingInteractionSystem::registerObserver(SignlingInteractionObserver* callback) {
    callback_ = callback;
  }

  void SignlingInteractionSystem::connect(const std::string& server, int port) {

  }

  void SignlingInteractionSystem::OnINVITE(const SignInfo& info) {

  }

  void SignlingInteractionSystem::OnOK(const SignInfo& info) {

  }

  void SignlingInteractionSystem::OnBYE(const SignInfo& info) {

  }

  void SignlingInteractionSystem::OnCANCEL(const SignInfo& info) {

  }

  void SignlingInteractionSystem::OnACK(const SignInfo& info) {

  }

  void SignlingInteractionSystem::OnUnauthorized(const SignInfo& info) {

  }

  void SignlingInteractionSystem::OnHeartbeat(const SignInfo& info) {

  }

}
