#include <pjsua2.hpp>
#include <iostream>
#include <SFML/Graphics.hpp>

using namespace pj;

// Subclass to extend the Account and get notifications etc.
class MyAccount : public Account {
public:
  virtual void onRegState(OnRegStateParam& prm) {
    AccountInfo ai = getInfo();
    std::cout << (ai.regIsActive ? "*** Register:" : "*** Unregister:")
      << " code=" << prm.code << std::endl;
  }
};

int main()
{
  bool isRunning = false;
  Endpoint ep;

  ep.libCreate();

  // Initialize endpoint
  EpConfig ep_cfg;
  ep.libInit(ep_cfg);

  // Create SIP transport. Error handling sample is shown
  TransportConfig tcfg;
  tcfg.port = 5060;
  try {
    ep.transportCreate(PJSIP_TRANSPORT_UDP, tcfg);
  }
  catch (Error& err) {
    std::cout << err.info() << std::endl;
    return 1;
  }

  // Start the library (worker threads etc)
  ep.libStart();
  std::cout << "*** PJSUA2 STARTED ***" << std::endl;

  // Configure an AccountConfig
  AccountConfig acfg;
  acfg.idUri = "sip:changjinglu@10.1.63.111:5060";
  acfg.regConfig.registrarUri = "sip:10.1.63.111:5060";
  AuthCredInfo cred("digest", "*", "changjinglu", 0, "123456");
  acfg.sipConfig.authCreds.push_back(cred);

  while (!isRunning) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
      isRunning = true;
    }
    pj_thread_sleep(10);
  }

  // Create the account
  MyAccount* acc = new MyAccount;
  acc->create(acfg);

  // Here we don't have anything else to do..
  while (isRunning) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
      isRunning = false;
    }
    pj_thread_sleep(10);
  }


  // Delete the account. This will unregister from server
  delete acc;

  // This will implicitly shutdown the library
  return 0;
}