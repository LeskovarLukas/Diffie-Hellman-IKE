#include <iostream>
#include <asio.hpp>
#include <fstream>
#include <spdlog/spdlog.h>

#include "BigInt/BigInt.hpp"

#include "pipe.h"
#include "utility.h"
#include "tls_util.h"


void handle_socket(asio::ip::tcp::socket&);

int main() {
    std::vector<std::thread> threads;

    try {
        asio::io_context io_context;
        asio::ip::tcp::endpoint endpoint{asio::ip::tcp::v4(), 4433};
        asio::ip::tcp::acceptor acceptor{io_context, endpoint};
        
        spdlog::set_level(spdlog::level::debug);
    
        try {
            spdlog::info("Waiting for connections");
            acceptor.listen();

            while (true) {
                asio::ip::tcp::socket socket{io_context};
                acceptor.accept(socket);
                spdlog::info("Connection accepted");
                
                threads.push_back(std::thread{handle_socket, std::ref(socket)});
            }
        } catch (std::exception& e) {
            spdlog::error(e.what());
        }
        acceptor.close();
    } catch (std::exception& e) {
        spdlog::error(e.what());
    }

    for (auto& thread : threads) {
        thread.join();
    }
    return 0;
}


void handle_socket(asio::ip::tcp::socket& socket) {
    std::string message;

    try {
        Pipe pipe{std::move(socket)};
        BigInt key;
        TLS_Util tls_util(pipe, key);

        while (true) {
            try {
                pipe >> message;

                if (message == "") {
                    spdlog::warn("Received empty message");
                    continue;
                }

                std::vector<std::string> message_parts;
                split_message(message, message_parts);
                
                if (message_parts[0] == "DATA" && tls_util.is_secure()) {
                    message = receive_message(key, std::stoul(message_parts[1]), message_parts[2]);
                    spdlog::info("Received Message: {}", message);
                } else if (message_parts[0] == "CLOSE") {
                    break;
                } else {
                    tls_util.handle_message(message_parts);
                }
            } catch (std::exception& e) {
                if (!pipe) 
                    throw std::runtime_error("Pipe connection lost");
            }
        }
    } catch (std::exception& e) {
        spdlog::error(e.what());
    }
    spdlog::info("Closing client connection");
}