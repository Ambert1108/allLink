#pragma once
#include "basescreen.h"
#include "config/path.h"
#include "component/module.h"
#include "component/widget.h"
#include "message.h"

#include "seeker/common.h"

namespace alllink {

	enum class MessageType : int {
		/*登录窗口消息载体*/

		CREATE_MEETING = 0,
		JOIN_MEETING,
		START_LOGIN,
		START_LOGOUT,
		SETTING


	};

	static constexpr int msgTo(MessageType msg) { return static_cast<int>(msg); }

	class CustomScreen : public BaseScreen {
	public:
		enum Style {
			Close = 0,       //表示窗口只有关闭
			Minisize,        //表示窗口拥有最小化和关闭
			All              //默认选项，窗口拥有最小化、最大化及关闭
		};

		CustomScreen(sf::VideoMode mode, const sf::String& title, sf::Image icon, int style);

		void checkStatus(sf::Event& event);

	protected:
		Style style_;
		sf::RectangleShape topSide_;
		sf::VideoMode wndSize_;
		/* 最大化按钮 */
		VariableStateRectangleModule square;
		/* 最小化按钮 */
		VariableStateVertxModule horizontalLine;
		/* 关闭按钮 */
		VariableStateVertxModule cross;
		/* 窗口是否最大化 */
		bool isDesktop;
	private:
		HWND hwnd;
		sf::Vector2i screenSize;
		sf::Vector2i dragOffset;
		bool isDragging;
		int maxX = 0;
		int minX = 0;
		int maxY = 0;
		int minY = 0;
		sf::View view;
		sf::Vector2i wndPos;
	};

	/*
	* 程序的开始界面，提供登录/注销，创建/加入会议和设置功能，属于流式界面，
	* 可以切换至下一个流式界面。
	*/
	class StartScreen : public CustomScreen {
	public:
		enum class LoginType {
			OFFLINE = 0,
			ONLINE
		};

		StartScreen(sf::VideoMode mode, const sf::String& title,
			sf::Image icon, int style = CustomScreen::Style::All);

		~StartScreen();

		std::shared_ptr<BaseScreen> Next() override;

		std::shared_ptr<BaseScreen> Last() override;

		void OnEnter() override;

		void OnExit() override;

		int init() override;

		void show() override;

		void eventProcess() override;

		LoginType type() const;

	private:
		std::unique_ptr<VerticalGraphicTextsModule> createMeeting;
		std::unique_ptr<VerticalGraphicTextsModule> joinMeeting;
		std::unique_ptr<HorizonGraphicTextsModule> startLogin;
		std::unique_ptr<HorizonGraphicTextsModule> isLogin;
		std::unique_ptr<VariableStateGraphicModule> setting;
		BaseText todayDate;
		BaseText useId;
		LoginType type_{ LoginType::OFFLINE };
		sf::RectangleShape taskSide;
	};

	/*
	* 程序的登录界面，允许用户选择服务器并输入账号密码，属于独立界面，
	* 无法进行界面切换，最终目的只是获取用户输入。
	*/
	class LoginScreen : public CustomScreen {
	public:
		enum class LoginType {
			OFFLINE = 0,
			ONLINE
		};

		LoginScreen(sf::VideoMode mode, const sf::String& title,
			sf::Image icon, int style = CustomScreen::Style::All);

		~LoginScreen();

		std::shared_ptr<BaseScreen> Next() override;

		std::shared_ptr<BaseScreen> Last() override;

		void OnEnter() override;

		void OnExit() override;

		int init() override;

		void show() override;

		void eventProcess() override;

		LoginType type() const;

	private:
		std::unique_ptr<VerticalGraphicTextsModule> createMeeting;
		std::unique_ptr<VerticalGraphicTextsModule> joinMeeting;
		std::unique_ptr<HorizonGraphicTextsModule> startLogin;
		std::unique_ptr<HorizonGraphicTextsModule> isLogin;
		std::unique_ptr<VariableStateGraphicModule> setting;
		BaseText todayDate;
		BaseText useId;
		LoginType type_{ LoginType::OFFLINE };
		sf::RectangleShape taskSide;
	};

	/*
	* 程序的创建/加入会议界面，允许用户输入会议号以创建/加入会议，属于独立界面，
	* 无法进行界面切换，最终目的只是获取用户输入。
	*/
	class EnterScreen : public CustomScreen {

	};

	/*
	* 程序的设置界面，功能暂定，属于独立界面，
	* 无法进行界面切换，最终目的只是获取用户输入。
	*/
	class SettingScreen : public CustomScreen {

	};

	/*
	* 程序的会议界面，提供会议画面接收，开关摄像头/麦克风/屏幕共享功能，属于流式界面，
	* 可以切换至上一个流式界面。
	*/
	class StreamScreen : public CustomScreen {

	};
}