#include "server.hpp"

Server::Server(boost::asio::io_context& ioc, short port)
    : acceptor_(ioc, tcp::endpoint(tcp::v4(), port))
    , socket_(ioc)
{
    accept();
}

void Server::accept() {
    acceptor_.async_accept(socket_, 
        [this](boost::system::error_code ec) {
            if (!ec) {
                auto session = std::make_shared<Session>(std::move(socket_));
                sessions_.push_back(session);
                session->start();
            }

            accept();
        }
    );
}
