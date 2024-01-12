#include "session.hpp"
#include <iostream>

Session::Session(tcp::socket socket)
    : socket_(std::move(socket))
    , data_(1024)
    , authd_(false)
{}

void Session::readPacket() {
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(data_),
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                if (handlePacket(data_, length))
                    readPacket(); // loop to next packet, async
                else closeSocket();
            } else if (ec == boost::asio::error::eof) {
                std::cout << "Disconnection from client" << std::endl;
            } else {
                std::cerr << "Read error: " << ec.message() << std::endl;
            }
        }
    );
}

void Session::closeSocket() {
    std::cout << "Closing socket..." << std::endl;
    boost::system::error_code ec;
    socket_.shutdown(tcp::socket::shutdown_both, ec);
    if (ec) {
        std::cerr << "Shutdown error: " << ec.message() << std::endl;
    }

    socket_.close(ec);
    if (ec) {
        std::cerr << "Close error: " << ec.message() << std::endl;
    }
}

void Session::sendMessage(const std::string& msg) {
    auto self(shared_from_this());
    write_buffer_ = "System: " + msg + "\n";

    boost::asio::async_write(socket_, boost::asio::buffer(write_buffer_),
        [this, self](boost::system::error_code ec, std::size_t) {
            if (ec) {
                std::cerr << "Error sending message: " << ec.message() << std::endl;
                std::cerr << "Errored content: " << write_buffer_ << std::endl;
            }
        }
    );
}

bool Session::handlePacket(const std::vector<char>& data, std::size_t length) {
    std::string command;
    std::stringstream ss(std::string(data.begin(), data.end()));
    getline(ss, command, ' ');

    if (command == "AUTH") {
        if (authd_) {
            sendMessage("Reconnect to change session account.");
            return false;
        }

        std::string username, password;
        getline(ss, username, ' ');
        getline(ss, password);

        std::cout << "User: " << username << std::endl;
        std::cout << "Pass: " << password << std::endl;
        
        if (username == "" || password == "") {
            sendMessage("Provide username and password to authenticate.");
            return false;
        }

        sendMessage("Welcome to shard, " + username + ".");
        authd_ = true;

        // TODO: validate this user/pass
    } else {
        if (!authd_) {
            sendMessage("Unknown command, disconnecting.");
            return false;
        } 

        sendMessage("Unknown command.");
    }

    return true;
}
