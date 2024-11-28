#pragma once
#include "nlohmann/json.hpp"
#include "nlohmann/fifomap.hpp"

#include "seeker/common.h"
#include "seeker/json.hpp"

#include <string>

namespace alllink {
	template<class K, class V, class dummy_compare, class A>
	using my_workaround_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
	using json = nlohmann::basic_json<my_workaround_fifo_map>;

	class SignInfo {
	public:
		SignInfo() = default;
		SignInfo(json json_) : js(json_) {};
		//callid
		std::string call_id() const {
			if (js["Call_ID"].is_null()) {
				return "";
			}
			return js["Call_ID"];
		}
		void set_call_id(std::string input) {
			js["Call_ID"] = input;
		}
		//userid
		std::string userid() const {
			if (js["userId"].is_null()) {
				return "";
			}
			return js["userId"];
		}
		void set_userid(std::string input) {
			js["userId"] = input;
		}
		//password
		std::string password() const {
			if (js["password"].is_null()) {
				return "";
			}
			return js["password"];
		}
		void set_password(std::string input) {
			js["password"] = input;
		}
		//cseq
		int cseq() const {
			if (js["Cseq"].is_null()) {
				return -1;
			}
			return js["Cseq"];
		}
		void set_cseq(int input) {
			js["Cseq"] = input;
		}
		//sdp
		std::string sdp() const {
			if (js["SDP"].is_null()) {
				return "";
			}
			return js["SDP"];
		}
		void set_sdp(std::string input) {
			js["SDP"] = input;
		}
		//meth
		std::string meth() const {
			if (js["meth"].is_null()) {
				return "";
			}
			return js["meth"];
		}
		void set_meth(std::string input) {
			js["meth"] = input;
		}
		//isresponse
		bool isresponse() const {
			if (js["isResponse"].is_null()) {
				return false;
			}
			return js["isResponse"];
		}
		void set_isresponse(bool input) {
			js["isResponse"] = input;
		}
		//from
		std::string from() const {
			if (js["from"].is_null()) {
				return "";
			}
			return js["from"];
		}
		void set_from(std::string input) {
			js["from"] = input;
		}
		//to
		std::string to() const {
			if (js["to"].is_null()) {
				return "";
			}
			return js["to"];
		}
		void set_to(std::string input) {
			js["to"] = input;
		}
		//signal
		std::string signal() const {
			if (js["signal"].is_null()) {
				return "";
			}
			return js["signal"];
		}
		void set_signal(std::string input) {
			js["signal"] = input;
		}
		//reason
		std::string reason() const {
			if (js["reason"].is_null()) {
				return "";
			}
			return js["reason"];
		}
		void set_reason(std::string input) {
			js["reason"] = input;
		}
		//cmeth
		std::string cmeth() const {
			if (js["Cmeth"].is_null()) {
				return "";
			}
			return js["Cmeth"];
		}
		void set_cmeth(std::string input) {
			js["Cmeth"] = input;
		}
		//statuscode
		int statuscode() const {
			if (js["statusCode"].is_null()) {
				return -1;
			}
			return js["statusCode"];
		}
		void set_statuscode(int input) {
			js["statusCode"] = input;
		}
		
		json js;
	};

	struct JanusCreate {
		std::string janus{ "create" };
		std::string transaction;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JanusCreate, janus, transaction);

	struct JanusAttach {
		std::string janus{ "attach" };
		int64_t session_id;
		std::string plugin;
		std::string transaction;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JanusAttach, janus, session_id, plugin, transaction);
	
	struct JanusDestory {
		std::string janus{ "destory" };
		std::string transaction;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JanusDestory, janus, transaction);

	struct JanusKeepAlive {
		std::string janus{ "keepalive" };
		int64_t session_id;
		std::string transaction;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JanusKeepAlive, janus, session_id, transaction);

	struct Candidate {
		std::string sdpMid;
		int sdpMLineIndex;
		std::string candidate;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Candidate, sdpMid, sdpMLineIndex, candidate);

	struct CandidateComplete {
		bool completed = true;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CandidateComplete, completed);

	template<typename T>
	struct JanusTrickle {
		std::string janus{ "trickle" };
		int64_t session_id;
		int64_t handle_id;
		std::string transaction;
		T candidate;
	};

	template<typename T>
	void to_json(nlohmann::json& j, const JanusTrickle<T>& obj) {
		j = nlohmann::json{
			{"janus", obj.janus},
			{"session_id", obj.session_id},
			{"handle_id", obj.handle_id},
			{"transaction", obj.transaction},
			{"candidate", obj.candidate}
		};
	}

	typedef JanusTrickle<Candidate> Trickle;
	typedef JanusTrickle<CandidateComplete> TrickleComplete;

	struct GenerateBody {
		std::string request{ "generate" };
		std::string info;
		std::string srtp;
		std::string srtp_profile;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GenerateBody, request, info, srtp, srtp_profile);

	struct Jsep {
		std::string sdp;
		std::string type;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Jsep, sdp, type);

	struct JanusGenerate {
		std::string janus{ "message" };
		int64_t session_id;
		int64_t handle_id;
		std::string transaction;
		GenerateBody body;
		Jsep jsep;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JanusGenerate, janus, session_id, handle_id, transaction, body, jsep);

	struct ProcessBody {
		std::string request{ "process" };
		std::string type;
		std::string sdp;
		std::string info;
		std::string srtp;
		std::string srtp_profile;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ProcessBody, request, type, sdp, info, srtp, srtp_profile);

	struct JanusProcess {
		std::string janus{ "message" };
		int64_t session_id;
		int64_t handle_id;
		std::string transaction;
		ProcessBody body;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JanusProcess, janus, session_id, handle_id, transaction, body);

	struct PluginResult {
		std::string event;
		std::string type;
		std::string sdp;
		std::string srtp;
		friend void from_json(const nlohmann::json& j, PluginResult& obj) {
			try {
				obj.event = j.at("event");
				if (j.contains("type")) obj.type = j.at("type");
				if (j.contains("sdp")) obj.sdp = j.at("sdp");
				if (j.contains("srtp")) obj.srtp = j.at("srtp");
			}
			catch (std::exception& ex) {
				throw std::invalid_argument("parser PluginResult json Error:" + *ex.what());
				return;
			}
		}
	};

	struct JanusRespData {
		int64_t id;
		std::string nosip;
		PluginResult result;
		friend void from_json(const nlohmann::json& j, JanusRespData& obj) {
			try {
				if (j.contains("id")) obj.id = j.at("id");
				if (j.contains("nosip")) obj.nosip = j.at("nosip");
				if (j.contains("result")) obj.result = j.at("result");
			}
			catch (std::exception& ex) {
				throw std::invalid_argument("parser JanusRespData json Error:" + *ex.what());
				return;
			}
		}
	};

	struct PluginData {
		std::string plugin;
		JanusRespData data;
		friend void from_json(const nlohmann::json& j, PluginData& obj) {
			try {
				obj.plugin = j.at("plugin");
				obj.data = j.at("data");
			}
			catch (std::exception& ex) {
				throw std::invalid_argument("parser PluginData json Error:" + *ex.what());
				return;
			}
		}
	};

	struct JanusReponse {
		std::string janus;
		std::string transaction;
		int64_t session_id;
		int64_t sender;
		JanusRespData data;
		PluginData plugindata;
		Jsep jsep;

		friend void from_json(const nlohmann::json& j, JanusReponse& obj) {
			try {
				obj.janus = j.at("janus");
				obj.transaction = j.at("transaction");
				if (j.contains("session_id")) obj.session_id = j.at("session_id");
				if (j.contains("sender")) obj.sender = j.at("sender");
				if (j.contains("data")) obj.data = j.at("data");
				if (j.contains("plugindata")) obj.plugindata = j.at("plugindata");
				if (j.contains("jsep")) obj.jsep = j.at("jsep");
			}
			catch (std::exception& ex) {
				throw std::invalid_argument("parser JanusReponse json Error:" + *ex.what());
				return;
			}
		}
	};
}