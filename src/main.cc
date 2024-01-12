#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>

#include "server.hpp"

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
