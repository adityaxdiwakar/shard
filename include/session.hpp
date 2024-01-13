#pragma once

#include "server.hpp"
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class Server;

class Session : public std::enable_shared_from_this<Session> {
    private:
        Server& parent_;
        tcp::socket socket_;
        std::optional<std::string> user_;

    private: // message buffers
        std::string write_buffer_;
        std::vector<char> data_;

    public:
        Session(tcp::socket socket, Server& parent);

        void start() {
            read_packet();
        }

    private:
        void read_packet();
        bool announce_auth();
        void close_socket();
        void send_message(const std::string& msg);
        bool handle_packet(const std::vector<char>& data, std::size_t length);
};
