#include <iostream>
#include <asio.hpp>
#include <spdlog/spdlog.h>


int main() {
    spdlog::set_level(spdlog::level::debug);

    
    try {
        asio::io_context io_context;

        asio::ip::tcp::resolver resolver(io_context);
        asio::ip::tcp::resolver::results_type endpoints = resolver.resolve("localhost", "4433");

        asio::ip::tcp::socket socket(io_context);
        asio::connect(socket, endpoints);

        spdlog::info("Connected to server");
        while (true) {
            spdlog::debug("Status: {}", socket.is_open());
            std::string message;
            std::getline(std::cin, message);

            asio::write(socket, asio::buffer(message));

            if (message == "quit") {
                break;
            }
        }
    } catch (std::exception& e) {
        spdlog::error("Exception: {}", e.what());
    }

    return 0;
}
