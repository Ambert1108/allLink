#pragma once
#include "oatpp-websocket/WebSocket.hpp"
#include "oatpp-websocket/Connector.hpp"
#include "oatpp/network/tcp/client/ConnectionProvider.hpp"
#include <thread>
#include <iostream>
#include <string>
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "nlohmann/json.hpp"
#include "nlohmann/fifomap.hpp"
#include "seeker/common.h"
#include "seeker/logger.h"
#include "seeker/loggerApi.h"

template<class K, class V, class dummy_compare, class A>
using my_workaround_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using json = nlohmann::basic_json<my_workaround_fifo_map>;

class Message
{
public:
	json js;
/* ======================
		Request 
========================= */
	//janus
	std::string janus() {
		if (js["janus"].is_null()) {
			return "";
		}
		return js["janus"];
	}
	void set_janus(std::string input) {
		js["janus"] = input;
	}

	//transaction
	std::string transaction() {
		if (js["transaction"].is_null()) {
			return "";
		}
		return js["transaction"];
	}
	void set_transaction(std::string input) {
		js["transaction"] = input;
	}

	//session_id
	int session_id() {
		if (js["session_id"].is_null()) {
			return -1;
		}
		return js["session_id"];
	}
	void set_session_id(int input) {
		js["session_id"] = input;
	}

	//plugin
	//std::string plugin() {
	//	if (js["plugin"].is_null()) {
	//		return "";
	//	}
	//	return js["plugin"];
	//}
	void set_plugin(std::string input) {
		js["plugin"] = input;
	}
	//handle_id
	//int handle_id() {
	//	if (js["handle_id"].is_null()) {
	//		return -1;
	//	}
	//	return js["handle_id"];
	//}
	void set_handle_id(int input) {
		js["handle_id"] = input;
	}

	//sdp
	std::string sdp() {
		if (js["SDP"].is_null()) {
			return "";
		}
		return js["SDP"];
	}
	void set_sdp(std::string input) {
		js["SDP"] = input;
	}

/*  ============  ��� body ����  ============ */
	//body_request	//js["body"]["request"] = "generate";
	//std::string body_request() {
	//	if (js["body"]["request"].is_null()) {
	//		return "";
	//	}
	//	return js["body"]["request"];
	//}
	void set_body_request(std::string input) {
		js["body"]["request"] = input;
	}
	//body_type	//js["body"]["type"] = "generate";
	//std::string body_type() {
	//	if (js["body"]["type"].is_null()) {
	//		return "";
	//	}
	//	return js["body"]["type"];
	//}
	void set_body_type(std::string input) {
		js["body"]["type"] = input;
	}
	//body_sdp	//js["body"]["sdp"] = "generate";
	//std::string body_sdp() {
	//	if (js["body"]["sdp"].is_null()) {
	//		return "";
	//	}
	//	return js["body"]["sdp"];
	//}
	void set_body_sdp(std::string input) {
		js["body"]["sdp"] = input;
	}
	//body_info	////js["body"]["info"] = "<�û������ṩ�����ĵĲ�͸���ַ�������ѡ>";
	//std::string body_info() {
	//	if (js["body"]["info"].is_null()) {
	//		return "";
	//	}
	//	return js["body"]["info"];
	//}
	void set_body_info(std::string input) {
		js["body"]["info"] = input;
	}
	//body_srtp	//js["body"]["srtp"] = "<�Ƿ�ǿ�� (sdes_mandatory) ���ṩ (sdes_optional) SRTP ֧�֣���ѡ>";
	//std::string body_srtp() {
	//	if (js["body"]["srtp"].is_null()) {
	//		return "";
	//	}
	//	return js["body"]["srtp"];
	//}
	void set_body_srtp(std::string input) {
		js["body"]["srtp"] = input;
	}
	//body_srtp_profile	//js["body"]["srtp_profile"] = "<����ṩ SRTP����Э�� SRTP �����ļ�����ѡ>";
	//std::string body_srtp_profile() {
	//	if (js["body"]["srtp_profile"].is_null()) {
	//		return "";
	//	}
	//	return js["body"]["srtp_profile"];
	//}
	void set_body_srtp_profile(std::string input) {
		js["body"]["srtp_profile"] = input;
	}

/*  =============== ��� jsep ����  ============= */
	//jsep_sdp	//js["jsep"]["sdp"] = "......";  // ����Ӧ���滻Ϊʵ�ʵ� SDP �ַ���
	std::string jsep_sdp() {
		if (js["jsep"]["sdp"].is_null()) {
			return "";
		}
		return js["jsep"]["sdp"];
	}
	void set_jsep_sdp(std::string input) {
		js["jsep"]["sdp"] = input;
	}
	//jsep_type	//	js["jsep"]["type"] = "<offer|answer��ȡ�����ṩ�� SDP ������>";  // ����Ӧ���滻Ϊ "offer" �� "answer"
	std::string jsep_type() {
		if (js["jsep"]["type"].is_null()) {
			return "";
		}
		return js["jsep"]["type"];
	}
	void set_jsep_type(std::string input) {
		js["jsep"]["type"] = input;
	}


/* ==================== 
		Response 
======================= */
	//sender = handle_id
	int sender() {
		if (js["sender"].is_null()) {
			return -1;
		}
		return js["sender"];
	}

	//json data
	long long data_id() {
		if (js["data"]["id"].is_null()) {
			return -1;
		}
		return js["data"]["id"];
	}

	//json plugindata : plugin
	std::string plugindata_plugin() {
		if (js["plugindata"]["plugin"].is_null()) {
			return "";
		}
		return js["plugindata"]["plugin"];
	}
	//json data : nosip
	std::string plugindata_data_nosip() {
		if (js["plugindata"]["data"]["nosip"].is_null()) {
			return "";
		}
		return js["plugindata"]["data"]["nosip"];
	}
	//json result : event
	std::string plugindata_data_result_event() {
		if (js["plugindata"]["data"]["result"]["event"].is_null()) {
			return "";
		}
		return js["plugindata"]["data"]["result"]["event"];
	}
	//json result : srtp
	std::string plugindata_data_result_srtp() {
		if (js["plugindata"]["data"]["result"]["srtp"].is_null()) {
			return "";
		}
		return js["plugindata"]["data"]["result"]["srtp"];
	}
	//json result : sdp
	std::string plugindata_data_result_sdp() {
		if (js["plugindata"]["data"]["result"]["sdp"].is_null()) {
			return "";
		}
		return js["plugindata"]["data"]["result"]["sdp"];
	}
	//json result : type
	std::string plugindata_data_result_type() {
		if (js["plugindata"]["data"]["result"]["type"].is_null()) {
			return "";
		}
		return js["plugindata"]["data"]["result"]["type"];
	}

};