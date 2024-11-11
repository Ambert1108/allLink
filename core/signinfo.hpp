#pragma once
#include "nlohmann/json.hpp"
#include "nlohmann/fifo_map.hpp"

#include <string>

namespace alllink {
	template<class K, class V, class dummy_compare, class A>
	using my_workaround_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
	using json = nlohmann::basic_json<my_workaround_fifo_map>;

	class SignInfo {
	public:
		SignInfo(json json_) : js(json_) {};
		//callid
		std::string call_id() {
			if (js["Call_ID"].is_null()) {
				return "";
			}
			return js["Call_ID"];
		}
		void set_call_id(std::string input) {
			js["Call_ID"] = input;
		}
		//userid
		std::string userid() {
			if (js["userId"].is_null()) {
				return "";
			}
			return js["userId"];
		}
		void set_userid(std::string input) {
			js["userId"] = input;
		}
		//password
		std::string password() {
			if (js["password"].is_null()) {
				return "";
			}
			return js["password"];
		}
		void set_password(std::string input) {
			js["password"] = input;
		}
		//cseq
		int cseq() {
			if (js["Cseq"].is_null()) {
				return -1;
			}
			return js["Cseq"];
		}
		void set_cseq(int input) {
			js["Cseq"] = input;
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
		//meth
		std::string meth() {
			if (js["meth"].is_null()) {
				return "";
			}
			return js["meth"];
		}
		void set_meth(std::string input) {
			js["meth"] = input;
		}
		//isresponse
		bool isresponse() {
			if (js["isResponse"].is_null()) {
				return false;
			}
			return js["isResponse"];
		}
		void set_isresponse(bool input) {
			js["isResponse"] = input;
		}
		//from
		std::string from() {
			if (js["from"].is_null()) {
				return "";
			}
			return js["from"];
		}
		void set_from(std::string input) {
			js["from"] = input;
		}
		//to
		std::string to() {
			if (js["to"].is_null()) {
				return "";
			}
			return js["to"];
		}
		void set_to(std::string input) {
			js["to"] = input;
		}
		//signal
		std::string signal() {
			if (js["signal"].is_null()) {
				return "";
			}
			return js["signal"];
		}
		void set_signal(std::string input) {
			js["signal"] = input;
		}
		//reason
		std::string reason() {
			if (js["reason"].is_null()) {
				return "";
			}
			return js["reason"];
		}
		void set_reason(std::string input) {
			js["reason"] = input;
		}
		//cmeth
		std::string cmeth() {
			if (js["Cmeth"].is_null()) {
				return "";
			}
			return js["Cmeth"];
		}
		void set_cmeth(std::string input) {
			js["Cmeth"] = input;
		}
		//statuscode
		int statuscode() {
			if (js["statusCode"].is_null()) {
				return -1;
			}
			return js["statusCode"];
		}
		void set_statuscode(int input) {
			js["statusCode"] = input;
		}

	private:
			json js;
	};
}