#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>

#include "server.hpp"
#include "engine.hpp"

int main() {
    try {
        auto x = std::make_unique<MatchingEngine>();
        x->process(std::make_shared<Order<Direction::BUY>>("adi", 1, "NUTS", 100'32, 1));
        x->process(std::make_shared<Order<Direction::SELL>>("kat", 2, "NUTS", 100'31, 2));
        x->process(std::make_shared<Order<Direction::BUY>>("adi", 3, "NUTS", 100'32, 1));

        boost::asio::io_context ioc;
        Server server(ioc, 20000);
        ioc.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
