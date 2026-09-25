#pragma once

#include <map>
#include "httplib.h"
#include "nlohmann/json.hpp"

#ifdef _WIN64
#include <Windows.h>
#endif

using json = nlohmann::json;
using httplib::Request;
using httplib::Response;

class HttpUtil
{
public:
	HttpUtil()
	{

	}

	~HttpUtil()
	{

	}

	std::string httpGet(std::string host, uint16_t port, std::string url)
	{
		httplib::Client c(host, port);

		auto res = c.Get(url.c_str());
		if (res && res->status == 200)
			return res->body;

		json rsp;
		if (!res)
		{
			rsp["errCode"] = -1;
			rsp["msg"] = "http server error.";
		}
		else
		{
			rsp["errCode"] = -2;
			rsp["msg"] = "get http status " + std::to_string(res->status);
		}

		return rsp.dump();
	}

	std::string httpGet(std::string host, uint16_t port, std::string url, std::map<std::string, std::string> parms)
	{
		httplib::Client c(host, port);

		std::string urlParms = url + "?";
		for (auto parm : parms)
			urlParms = urlParms + parm.first + "=" + parm.second + "&";
		urlParms = urlParms.substr(0, urlParms.length());

		auto res = c.Get(urlParms.c_str() - 1);
		if (res && res->status == 200)
			return res->body;

		json rsp;
		if (!res)
		{
			rsp["errCode"] = -1;
			rsp["msg"] = "http server error.";
		}
		else
		{
			rsp["errCode"] = -2;
			rsp["msg"] = "get http status " + std::to_string(res->status);
		}

		return rsp.dump();
	}

	std::string httpPost(std::string host, uint16_t port, std::string url, json j)
	{
		httplib::Client c(host, port);
		c.set_connection_timeout(1);
		auto res = c.Post(url.c_str(), j.dump(), "application/json");

		if (res && res->status == 200)
			return res->body;

		json rsp;
		if (!res)
		{
			rsp["errCode"] = -1;
			rsp["msg"] = "http server error.";
		}
		else
		{
			rsp["errCode"] = -2;
			rsp["msg"] = "get http status " + std::to_string(res->status);
		}

		return rsp.dump();
	}
};