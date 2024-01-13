#include "session.hpp"
#include <iostream>
#include <optional>

Session::Session(tcp::socket socket, Server& parent)
    : parent_(parent)
    , socket_(std::move(socket))
    , data_(1024)
    , user_(std::nullopt)
{}

void Session::read_packet() {
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(data_),
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                if (handle_packet(data_, length)) 
                    read_packet(); // loop to next packet, async
                else close_socket();
            } else { // error occurred, will end session
                // below is only logging
                if (ec == boost::asio::error::eof) {
                    std::cout << "Disconnection from client" << std::endl;
                } else {
                    std::cerr << "Read error: " << ec.message() << std::endl;
                }
            }
        }
    );
}

void Session::close_socket() {
    if (user_.has_value())
        parent_.rm_session(user_.value());

    std::cout << "Closing socket..." << std::endl;
    boost::system::error_code ec;
    socket_.shutdown(tcp::socket::shutdown_both, ec);
    socket_.close(ec);
}

void Session::send_message(const std::string& msg) {
    auto self(shared_from_this());
    write_buffer_ = "System: " + msg + "\n";

    boost::asio::async_write(socket_, boost::asio::buffer(write_buffer_),
        [this, self](boost::system::error_code ec, std::size_t) {
            if (ec) {
                std::cerr << "Error sending message: " << ec.message() << std::endl;
                std::cerr << "Errored content: " << write_buffer_ << std::endl;
            } // else message sent successfully
        }
    );
}

bool Session::handle_packet(const std::vector<char>& data, std::size_t length) {
    std::string command;
    std::stringstream ss(std::string(data.begin(), data.end()));
    getline(ss, command, ' ');

    // guard check: any command other than AUTH is invalid
    if (!user_.has_value() && command != "AUTH") {
        send_message("Failed to authenticate.");
        return false;
    }

    if (command == "AUTH") {
        // guard check: cannot AUTH an already authenticated session
        if (user_.has_value()) {
            send_message("Reconnect to change session account.");
            return false;
        }

        std::string username, password;
        getline(ss, username, ' ');
        getline(ss, password);

        std::cout << "Welcomed user " << username << " to shard-exch." << std::endl;
        
        if (username == "" || password == "") {
            send_message("Provide username and password to authenticate.");
            return false;
        }

        // TODO: validate this user/pass using authorizer

        user_ = username;
        parent_.add_session(username, this);
        send_message("Welcome to shard, " + username + ".");

        return true;
    }

    if (command == "LMT") { // limit orders
    } else if (command == "MKT") { // market orders
    } else {
        if (!user_.has_value()) {
            send_message("Unknown command, disconnecting.");
            return false;
        } 

        send_message("Unknown command.");
    }

    return true;
}
