#include "server.hpp"
#include <iostream>

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
              auto session = std::make_shared<Session>(std::move(socket_), *this);
              session->start();
            }

            accept();
        }
    );
}

void Server::add_session(std::string username, Session* session) {
    std::lock_guard<std::mutex> lock(lock_);
    sessions_[username] = session;
}

void Server::rm_session(std::string username) {
    std::lock_guard<std::mutex> lock(lock_);
    sessions_.erase(username);
}
