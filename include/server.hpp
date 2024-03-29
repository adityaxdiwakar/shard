#pragma once

#include "session.hpp"
#include <boost/asio.hpp>

class Session;

using boost::asio::ip::tcp;
class Server : public std::enable_shared_from_this<Server> {
    // TODO: change this from std::string -> uint32_t (user id)
    std::unordered_map<std::string, Session*> sessions_;
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    std::mutex lock_;

    public:
        Server(boost::asio::io_context& ioc, short port);
        void add_session(std::string username, Session* session);
        void rm_session(std::string username);

    private:
        void accept();
};
