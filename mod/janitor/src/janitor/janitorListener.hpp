
#ifndef janitorListener_hpp
#define janitorListener_hpp

#include "oatpp-websocket/ConnectionHandler.hpp"
#include "oatpp-websocket/WebSocket.hpp"
#include <queue>
#include "wsUtil.hpp"
namespace rtcengine {
    class JLObserver {
    public:

        virtual ~JLObserver() = default;

        virtual void OnAck() = 0;
        virtual void OnEvent(const Message& message) = 0;
        virtual void OnSuccess(const Message& message) = 0;
        virtual void OnWebrtcup() = 0;
        virtual void OnMedia() = 0;
        virtual void OnPong() = 0;
    };


    class JanitorListener : public oatpp::websocket::WebSocket::Listener {
    private:
        static constexpr const char* TAG = "Client_WSListener";
    private:
        std::mutex& m_writeMutex;

        oatpp::data::stream::BufferOutputStream m_messageBuffer;

        JLObserver* callback_;
    public:

        JanitorListener(std::mutex& writeMutex)
            : m_writeMutex(writeMutex)
        {
        }

        void onPing(const WebSocket& socket, const oatpp::String& message) override;

        void onPong(const WebSocket& socket, const oatpp::String& message) override;

        void onClose(const WebSocket& socket, v_uint16 code, const oatpp::String& message) override;

        void readMessage(const WebSocket& socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) override;


        void registerObserver(JLObserver* callback);
    };
}
#endif // WSListener_hpp