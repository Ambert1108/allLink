#include<iostream>
#include"controller.h"
int main() {
	seeker::Logger::init();
	I_LOG("hello rtcVideoEngine");
	try
	{
		std::shared_ptr< httplib::Client> cli = std::make_shared<httplib::Client>("10.1.29.246", 8088);
		auto conductor = rtc::make_ref_counted<Conductor>(cli);
		conductor->start();
		while (true) {
			Sleep(1000);
		}
	}
	catch (const std::exception& e)
	{
		E_LOG("error [{}]", e.what());
	}
	seeker::Logger::shutdown();
	return 0;
}