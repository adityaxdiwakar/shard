#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <map>

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
    private:
        tcp::socket socket_;
        std::vector<char> data_;
        bool authd_;

        std::string write_buffer_;

    public:
        Session(tcp::socket socket)
            : socket_(std::move(socket))
            , data_(1024)
            , authd_(false)
        {}

        void start() {
            readPacket();
        }

    private:
        void readPacket() {
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

        void closeSocket() {
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

        void sendMessage(const std::string& msg) {
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

        bool handlePacket(const std::vector<char>& data, std::size_t length) {
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
                getline(ss, password, ' ');

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
};

class Server {
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    std::vector<std::shared_ptr<Session>> sessions_;

    public:
        Server(boost::asio::io_context& ioc, short port)
            : acceptor_(ioc, tcp::endpoint(tcp::v4(), port))
            , socket_(ioc)
        {
            accept();
        }

    private:
        void accept() {
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
};

int main() {
    try {
        boost::asio::io_context ioc;
        Server server(ioc, 20000);
        ioc.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
