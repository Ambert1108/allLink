/*
 * Copyright (C) 2008-2011 Teluu Inc. (http://www.teluu.com)
 * Copyright (C) 2003-2008 Benny Prijono <benny@prijono.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

 /**
  * simple_pjsua.c
  *
  * This is a very simple but fully featured SIP user agent, with the
  * following capabilities:
  *  - SIP registration
  *  - Making and receiving call
  *  - Audio/media to sound device.
  *
  * Usage:
  *  - To make outgoing call, start simple_pjsua with the URL of remote
  *    destination to contact.
  *    E.g.:
  *       simpleua sip:user@remote
  *
  *  - Incoming calls will automatically be answered with 200.
  *
  * This program will quit once it has completed a single call.
  */

#include <pjsua-lib/pjsua.h>
#include <pjsua2.hpp>
#include "seeker/logger.h"
#include "seeker/loggerApi.h"

#include <string>

#define THIS_FILE       "APP"

#define SIP_DOMAIN      "10.1.63.111:5060"
#define SIP_USER        "alice"
#define SIP_PASSWD      "secret"

class MyCall : public pj::Call {
public:
  MyCall(pj::Account& acc, int call_id = PJSUA_INVALID_ID) : Call(acc, call_id) {}

  virtual void onCallState(pj::OnCallStateParam& prm) override {
    pj::CallInfo ci = getInfo();
    std::cout << "*** 呼叫状态变化: " << ci.stateText << " (" << ci.remoteUri << ")" << std::endl;

    if (ci.state == PJSIP_INV_STATE_DISCONNECTED) {
      delete this; // 自动清理资源
    }
  }
};


// 来电回调函数
/* Callback called by the library upon receiving incoming call */
static void on_incoming_call(pjsua_acc_id acc_id, pjsua_call_id call_id,
  pjsip_rx_data* rdata)
{
  pjsua_call_info ci;

  PJ_UNUSED_ARG(acc_id);
  PJ_UNUSED_ARG(rdata);

  // 获得呼叫信息
  pjsua_call_get_info(call_id, &ci);

  I_LOG("Incoming call from {} {}", (int)ci.remote_info.slen, ci.remote_info.ptr);

  // 自动接听呼叫
  /* Automatically answer incoming calls with 200/OK */
  pjsua_call_answer(call_id, 200, NULL, NULL);
}

/* Callback called by the library when call's state has changed */
static void on_call_state(pjsua_call_id call_id, pjsip_event* e)
{
  pjsua_call_info ci;

  PJ_UNUSED_ARG(e);

  pjsua_call_get_info(call_id, &ci);
  I_LOG("Call {} state = {} {}", call_id,
    (int)ci.state_text.slen,
    ci.state_text.ptr);
}

/* Callback called by the library when call's media state has changed */
static void on_call_media_state(pjsua_call_id call_id)
{
  pjsua_call_info ci;

  pjsua_call_get_info(call_id, &ci);

  if (ci.media_status == PJSUA_CALL_MEDIA_ACTIVE) {
    // When media is active, connect call to sound device.
    pjsua_conf_connect(ci.conf_slot, 0);
    pjsua_conf_connect(0, ci.conf_slot);
  }
}

static void on_call_sdp_created(pjsua_call_id call_id, pjmedia_sdp_session* sdp, pj_pool_t* pool, const pjmedia_sdp_session* rem_sdp) {
  char offer[1000], newOffer[1000];
  pjmedia_sdp_print(sdp, offer, 1000);
  I_LOG("local sdp is \n{}", offer);
  //sdp->conn = (pjmedia_sdp_conn*)pj_pool_zalloc(pool, sizeof(pjmedia_sdp_conn));
  //sdp->conn->net_type = pj_str("IN");
  //sdp->conn->addr_type = pj_str("IP6");
  //sdp->conn->addr = pj_str("192.168.1.6");
  //std::string newSdp =
  //  "v=0\r\n"
  //  "o=alice 2890844526 2890844526 IN IP4 192.168.1.2\r\n"
  //  "s=-\r\n"
  //  "c=IN IP4 192.168.1.2\r\n"
  //  "t=0 0\r\n"
  //  "m=audio 4000 RTP/AVP 0\r\n";
  //pjmedia_sdp_session* new_sdp;
  //if (pjmedia_sdp_parse(pool, const_cast<char*>(newSdp.c_str()), newSdp.length(), &new_sdp) == PJ_SUCCESS) {
  //  pjmedia_sdp_print(new_sdp, newOffer, 1000);
  //  W_LOG("modify sdp success\n{}", newOffer);
  //}
  //else {
  //  E_LOG("modify sdp failed");
  //
  //}



  int nPort;
  if (sdp != NULL) {
  
    pjmedia_sdp_media* m = sdp->media[sdp->media_count - 1];
    m->desc.port = 55512;
  
  
    pjmedia_sdp_conn* c = sdp->conn;
    char* addr;
    if (c)
      addr = c->addr.ptr;
    else
    {
    
      const pj_str_t* hostname;
      pj_sockaddr_in tmp_addr;
      char* addr;
    
      hostname = pj_gethostname();
      pj_sockaddr_in_init(&tmp_addr, hostname, 0);
      addr = pj_inet_ntoa(tmp_addr.sin_addr);
      sdp->conn = (pjmedia_sdp_conn*)pj_pool_zalloc(pool, sizeof(pjmedia_sdp_conn));
      sdp->conn->net_type = pj_str("IN");
      sdp->conn->addr_type = pj_str("IP6");
      sdp->conn->addr = pj_str("10.1.61.77");
    }
    
    sdp->origin.addr = *pj_gethostname();
  }
}

/* Display error and exit application */
static void error_exit(const char* title, pj_status_t status)
{
  pjsua_perror(THIS_FILE, title, status);
  pjsua_destroy();
  exit(1);
}

/*
 * main()
 *
 * argv[1] may contain URL to call.
 */
int main(int argc, char* argv[])
{
  pjsua_acc_id acc_id;
  pj_status_t status;

  // 创建PJSIP
  /* Create pjsua first! */
  status = pjsua_create();
  if (status != PJ_SUCCESS) error_exit("Error in pjsua_create()", status);

  // 校验被叫SIP地址是否正确
  /* If argument is specified, it's got to be a valid SIP URL */
  if (argc > 1) {
    status = pjsua_verify_url(argv[1]);
    if (status != PJ_SUCCESS) error_exit("Invalid URL in argv", status);
  }


  // 初始化PJSUA，设置回调函数
  /* Init pjsua */
  {
    pjsua_config cfg;
    pjsua_logging_config log_cfg;

    pjsua_config_default(&cfg);
    cfg.cb.on_incoming_call = &on_incoming_call;
    cfg.cb.on_call_media_state = &on_call_media_state;
    cfg.cb.on_call_state = &on_call_state;
    //cfg.cb.on_call_sdp_created = &on_call_sdp_created;

    pjsua_logging_config_default(&log_cfg);
    log_cfg.console_level = 4;

    status = pjsua_init(&cfg, &log_cfg, NULL);
    if (status != PJ_SUCCESS) error_exit("Error in pjsua_init()", status);
  }

  // 创建PJSIP的传输端口
  /* Add UDP transport. */
  {
    pjsua_transport_config cfg;

    pjsua_transport_config_default(&cfg);
    cfg.port = 61266;
    cfg.port_range = 100;
    status = pjsua_transport_create(PJSIP_TRANSPORT_UDP, &cfg, NULL);
    if (status != PJ_SUCCESS) error_exit("Error creating transport", status);
  }

  // 启动PJSIP
  /* Initialization is done, now start pjsua */
  status = pjsua_start();
  if (status != PJ_SUCCESS) error_exit("Error starting pjsua", status);


  // 设置并注册PJSIP账号
  /* Register to SIP server by creating SIP account. */
  {
    pjsua_acc_config cfg;

    pjsua_acc_config_default(&cfg);
    cfg.id = pj_str("sip:" SIP_USER "@" SIP_DOMAIN);
    cfg.reg_uri = pj_str("sip:" SIP_DOMAIN);
    cfg.cred_count = 1;
    cfg.cred_info[0].realm = pj_str(SIP_DOMAIN);
    cfg.cred_info[0].scheme = pj_str("digest");
    cfg.cred_info[0].username = pj_str(SIP_USER);
    cfg.cred_info[0].data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
    cfg.cred_info[0].data = pj_str(SIP_PASSWD);

    status = pjsua_acc_add(&cfg, PJ_TRUE, &acc_id);
    if (status != PJ_SUCCESS) error_exit("Error adding account", status);
  }


  // 如果设置了被叫地址，发起呼叫，否则作为被叫等待接听
  /* If URL is specified, make call to the URL. */
  if (argc > 1) {
    std::string dstUrl = argv[1];
    I_LOG("dst url is {}", dstUrl);
    pj_str_t uri = pj_str(argv[1]);
    status = pjsua_call_make_call(acc_id, &uri, 0, NULL, NULL, NULL);
    if (status != PJ_SUCCESS) error_exit("Error making call", status);
  }

  /* Wait until user press "q" to quit. */
  for (;;) {
    char option[10];

    puts("Press 'h' to hangup all calls, 'q' to quit");
    if (fgets(option, sizeof(option), stdin) == NULL) {
      puts("EOF while reading stdin, will quit now..");
      break;
    }

    if (option[0] == 'q')
      break;

    if (option[0] == 'h')
      pjsua_call_hangup_all();
  }

  /* Destroy pjsua */
  pjsua_destroy();

  return 0;
}
