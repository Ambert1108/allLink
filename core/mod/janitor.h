#include "janitorListener.hpp"
#include <random>
#include <deque>

class JanitorObserver {
public:

    virtual ~JanitorObserver() = default;

    virtual void OnGenerated(const std::string& sdp, const std::string& type) = 0;
    virtual void OnProcessed(const std::string& sdp, const std::string& type) = 0;
};

class Janitor:public JLObserver {
public:
    Janitor(std::string ip, v_uint16 port);

    ~Janitor();

    void init();

    void addNoSIP();

    void generateSDP(std::string jsepSdp, std::string type);

    void processSDP(std::string normalSdp, std::string type);

    void sendTrickleToJanus(const std::string& ice);

    void sendTrickleCompleteToJanus();

    void registerObserver(JanitorObserver* callback);
	
private:
    enum State{
        ONLINE=0,
        SESSIONING,
        HANDLEING,
        WAIT,
        GENERATING,
        PROCESSING
    };

    std::string ip;
    v_uint16 port;
    const char* TAG = "Janus-WS-Client";

    std::mutex dequeMutex;
    std::deque<Message> unmatchDeque;

    std::shared_ptr<JanitorListener> listener = nullptr;
    std::shared_ptr<oatpp::websocket::WebSocket> socket = nullptr;

    //message
    std::atomic <State> engineState{ONLINE};
    long long sessionID = 0;
    long long handleID = 0;
    int64_t transaction = 0;
    std::atomic<bool> destory = false;

    //std::thread readThread;
    std::thread aliveThread;
    std::thread listenThread;

    void keepAlive();
    void socketTask(const std::shared_ptr<oatpp::websocket::WebSocket>& websocket);


    JanitorObserver* callback_;
protected:
    void OnEvent(const Message& message) override;
    void OnSuccess(const Message& message) override;

};