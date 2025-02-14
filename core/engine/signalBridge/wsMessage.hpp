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
    /*void printMessage() {
        std::string tmpMeth = meth();
        std::string tmpFrom = from();
        std::string tmpTo = to();
        I_LOG(" meth[{}] from[{}] to[{}] signal[{}]", tmpMeth, tmpFrom, tmpTo, signal());
    }*/

    // janus
    std::string janus() {
        if (js["janus"].is_null()) {
            return "";
        }
        return js["janus"];
    }
    void set_janus(std::string input) {
        js["janus"] = input;
    }

    // transaction
    std::string transaction() {
        if (js["transaction"].is_null()) {
            return "";
        }
        return js["transaction"];
    }
    void set_transaction(std::string input) {
        js["transaction"] = input;
    }

    // session_id
    int64_t session_id() {
        if (js["session_id"].is_null()) {
            return -1;
        }
        return js["session_id"];
    }
    void set_session_id(int64_t input) {
        js["session_id"] = input;
    }

    // handle_id
    int64_t handle_id() {
        if (js["handle_id"].is_null()) {
            return -1;
        }
        return js["handle_id"];
    }
    void set_handle_id(int64_t input) {
        js["handle_id"] = input;
    }

    // callid
    std::string call_id() {
        if (js["Call_ID"].is_null()) {
            return "";
        }
        return js["Call_ID"];
    }
    void set_call_id(std::string input) {
        js["Call_ID"] = input;
    }

    // userid
    std::string userid() {
        if (js["userId"].is_null()) {
            return "";
        }
        return js["userId"];
    }
    void set_userid(std::string input) {
        js["userId"] = input;
    }

    // password
    std::string password() {
        if (js["password"].is_null()) {
            return "";
        }
        return js["password"];
    }
    void set_password(std::string input) {
        js["password"] = input;
    }

    // cseq
    int cseq() {
        if (js["Cseq"].is_null()) {
            return -1;
        }
        return js["Cseq"];
    }
    void set_cseq(int input) {
        js["Cseq"] = input;
    }

    // sdp
    std::string sdp() {
        if (js["SDP"].is_null()) {
            return "";
        }
        return js["SDP"];
    }
    void set_sdp(std::string input) {
        js["SDP"] = input;
    }

    // meth
    std::string meth() {
        if (js["meth"].is_null()) {
            return "";
        }
        return js["meth"];
    }
    void set_meth(std::string input) {
        js["meth"] = input;
    }

    // isresponse
    bool isresponse() {
        if (js["isResponse"].is_null()) {
            return false;
        }
        return js["isResponse"];
    }
    void set_isresponse(bool input) {
        js["isResponse"] = input;
    }

    // from
    std::string from() {
        if (js["from"].is_null()) {
            return "";
        }
        return js["from"];
    }
    void set_from(std::string input) {
        js["from"] = input;
    }

    // to
    std::string to() {
        if (js["to"].is_null()) {
            return "";
        }
        return js["to"];
    }
    void set_to(std::string input) {
        js["to"] = input;
    }

    // signal
    std::string signal() {
        if (js["signal"].is_null()) {
            return "";
        }
        return js["signal"];
    }
    void set_signal(std::string input) {
        js["signal"] = input;
    }

    // reason
    std::string reason() {
        if (js["reason"].is_null()) {
            return "";
        }
        return js["reason"];
    }
    void set_reason(std::string input) {
        js["reason"] = input;
    }

    // cmeth
    std::string cmeth() {
        if (js["Cmeth"].is_null()) {
            return "";
        }
        return js["Cmeth"];
    }
    void set_cmeth(std::string input) {
        js["Cmeth"] = input;
    }

    // statuscode
    int statuscode() {
        if (js["statusCode"].is_null()) {
            return -1;
        }
        return js["statusCode"];
    }
    void set_statuscode(int input) {
        js["statusCode"] = input;
    }

    //completed
    bool completed() {
        if (js["completed"].is_null()) {
            return false;
        }
        return js["completed"];
    }
    void set_completed(bool input) {
        js["completed"] = input;
    }

    //completed
    std::string candidate() {
        if (js["candidate"].is_null()) {
            return "";
        }
        return js["candidate"];
    }
    void set_candidate(std::string input) {
        js["candidate"] = input;
    }

    //sdpMLineIndex
    int sdpMLineIndex() {
        if (js["sdpMLineIndex"].is_null()) {
            return -1;
        }
        return js["sdpMLineIndex"];
    }
    void set_sdpMLineIndex(int input) {
        js["sdpMLineIndex"] = input;
    }

    //sdpMid
    std::string sdpMid() {
        if (js["sdpMid"].is_null()) {
            return "";
        }
        return js["sdpMid"];
    }
    void set_sdpMid(std::string input) {
        js["sdpMid"] = input;
    }

    int64_t timePoint() const {
        if (js["timePoint"].is_null()) {
            E_LOG("timePoint is null");
            return -1;
        }
        if (!js["timePoint"].is_number()) {
            E_LOG("timePoint is not number");
            return -1;
        }
        return js["timePoint"];
    }
};