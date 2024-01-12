#pragma once

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
    private:
        tcp::socket socket_;
        std::vector<char> data_;
        bool authd_;

        std::string write_buffer_;

    public:
        Session(tcp::socket socket);

        void start() {
            readPacket();
        }

    private:
        void readPacket();
        void closeSocket();
        void sendMessage(const std::string& msg);
        bool handlePacket(const std::vector<char>& data, std::size_t length);
};
