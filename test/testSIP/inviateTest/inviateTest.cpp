#include <pjsua-lib/pjsua.h>
#include <pjlib.h>
#include <pjlib-util.h>
#include <pjnath.h>
#include <pjsua2.hpp>

#include <iostream>
#include <string>

#define LOCAL_DOMAIN "sip:26.26.26.1" // 替换为本地IP或域名
#define TARGET_URI "sip:zzx@example.com" // 目标地址

static pj_pool_t* pool;
static pjsip_endpoint* endpt;


/* 发送请求的回调（可选） */
static void on_send_complete(void* token, pjsip_event* e) {
  
  //if (bytes_sent == -1) {
  //  PJ_LOG(1, ("send", "发送失败"));
  //}
  //else {
  //  PJ_LOG(3, ("send", "请求已发送"));
  //}
}

int main() {
  pj_status_t status;
  pjsua_acc_id acc_id;
  pjsip_tx_data* tdata;

  // 初始化PJSUA
  status = pjsua_create();

  pjsua_config cfg;
  pjsua_logging_config log_cfg;
  pjsua_media_config media_cfg;

  pjsua_config_default(&cfg);
  pjsua_logging_config_default(&log_cfg);
  pjsua_media_config_default(&media_cfg);

  log_cfg.msg_logging = PJ_TRUE;
  log_cfg.console_level = 4;

  status = pjsua_init(&cfg, &log_cfg, &media_cfg);

  // 添加UDP传输
  pjsua_transport_id transport_id;
  pjsua_transport_config transport_cfg;
  pjsua_transport_config_default(&transport_cfg);
  transport_cfg.port = 5060;

  status = pjsua_transport_create(PJSIP_TRANSPORT_UDP, &transport_cfg, &transport_id);

  // 启动PJSUA
  status = pjsua_start();

  // 创建内存池
  endpt = pjsua_get_pjsip_endpt();
  pool = pjsua_pool_create("invite", 512, 512);

  // 构造INVITE请求
  pjsip_method method;
  pjsip_method_set(&method, PJSIP_INVITE_METHOD);

  // 解析目标URI
  pjsip_uri* target_uri = pjsip_parse_uri(pool, TARGET_URI, strlen(TARGET_URI), 0);
  if (!target_uri) {
    PJ_LOG(1, ("main", "无效的目标URI"));
    return -1;
  }

  // 创建From头
  pj_str_t from_str = pj_str(LOCAL_DOMAIN);
  pjsip_from_hdr* from = pjsip_from_hdr_create(pool);
  from->uri = pjsip_parse_uri(pool, from_str.ptr, from_str.slen, 0);

  // 创建To头
  pjsip_to_hdr* to = pjsip_to_hdr_create(pool);
  to->uri = target_uri;

  // 创建Call-ID
  pjsip_cid_hdr* call_id = pjsip_cid_hdr_create(pool);
  std::string callId = "asg1tqa136";
  call_id->id = pj_str((char*)callId.c_str());


  // 创建CSeq
  pjsip_cseq_hdr* cseq = pjsip_cseq_hdr_create(pool);
  cseq->cseq = 1; // 初始CSeq值
  pjsip_method_copy(pool, &cseq->method, &method);

  // 创建Contact头
  pjsip_contact_hdr* contact = pjsip_contact_hdr_create(pool);
  contact->uri = pjsip_parse_uri(pool, from_str.ptr, from_str.slen, 0);

  pj_str_t target = pj_str(TARGET_URI);
  // 创建请求
  status = pjsip_endpt_create_request(endpt, &method, &target,
    &from_str, &target,
    &from_str,
    &call_id->id, cseq->cseq,
    NULL, &tdata);
  //CHECK(status);

  // 添加Max-Forwards头
  pjsip_max_fwd_hdr* max_fwd = pjsip_max_fwd_hdr_create(pool, 70);
  pjsip_msg_add_hdr(tdata->msg, (pjsip_hdr*)max_fwd);

  // 添加SDP体
  const char* sdp_content =
    "v=0\r\n"
    "o=alice 2890844526 2890844526 IN IP4 192.168.1.2\r\n"
    "s=-\r\n"
    "c=IN IP4 192.168.1.2\r\n"
    "t=0 0\r\n"
    "m=audio 4000 RTP/AVP 0\r\n";
  pj_str_t sdp_str = pj_str((char*)sdp_content);
  pj_str_t mime_application = { "application", 11 };
  pj_str_t mime_sdp = { "sdp", 3 };
  tdata->msg->body = pjsip_msg_body_create(pool,
    &mime_application, &mime_sdp,
    &sdp_str);

  // 发送请求
  status = pjsip_endpt_send_request(endpt, tdata, -1, NULL, &on_send_complete);
  if (status != PJ_SUCCESS) {
    pjsip_tx_data_dec_ref(tdata);
    //CHECK(status);
  }

  // 等待发送完成（实际应用中需事件循环）
  pj_thread_sleep(2000);

  // 清理
  pjsua_destroy();
  return 0;
}