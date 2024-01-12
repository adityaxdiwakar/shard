#pragma once

#include "session.hpp"
#include <boost/asio.hpp>

using boost::asio::ip::tcp;
class Server {
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    std::vector<std::shared_ptr<Session>> sessions_;

    public:
        Server(boost::asio::io_context& ioc, short port);

    private:
        void accept();
};
