#include <iostream>
#include <asio.hpp>
#include "include/pipe.h"

int main() {
    asio::io_context io_context;
    asio::ip::tcp::endpoint endpoint{asio::ip::tcp::v4(), 4433};

    try {
        asio::ip::tcp::acceptor acceptor{io_context, endpoint};
        asio::ip::tcp::socket socket{io_context};

        acceptor.listen();

        while (true) {
            acceptor.accept(socket);
            Pipe pipe{std::move(socket)};

            std::string message;
            pipe >> message;
            std::cout << "Received: " << message << std::endl;
            pipe << "ServerHello! ";
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}