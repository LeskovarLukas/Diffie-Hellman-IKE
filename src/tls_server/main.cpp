#include <iostream>
#include <asio.hpp>
#include <fstream>
#include <spdlog/spdlog.h>
#include <future>

#include "BigInt/BigInt.hpp"
#include "CLI11.hpp"

#include "pipe.h"
#include "utility.h"
#include "tls_util.h"


void handle_socket(asio::ip::tcp::socket&);

int main(int argc, char* argv[]) {
    CLI::App app{"tls_server"};

    int port = 4433;
    spdlog::level::level_enum log_level = spdlog::level::info;
    std::map<std::string, spdlog::level::level_enum> log_level_map = {
        {"trace", spdlog::level::trace},
        {"debug", spdlog::level::debug},
        {"info", spdlog::level::info},
        {"warn", spdlog::level::warn},
        {"error", spdlog::level::err},
        {"critical", spdlog::level::critical}
    };

    app.add_option("-p,--port", port, "Port");
    app.add_option("-l,--log-level", log_level, "Log level")->transform(CLI::CheckedTransformer(log_level_map, CLI::ignore_case));


    CLI11_PARSE(app, argc, argv);


    spdlog::set_level(log_level);
    std::vector<std::thread> threads;
    std::vector<asio::ip::tcp::socket*> sockets;

    try {
        asio::io_context io_context;
        asio::ip::tcp::endpoint endpoint{asio::ip::tcp::v4(), (asio::ip::port_type)port};
        asio::ip::tcp::acceptor acceptor{io_context, endpoint};
    
        try {

            spdlog::info("Waiting for connections");
            acceptor.listen();

            while (true) {
                asio::ip::tcp::socket* socket = new asio::ip::tcp::socket(io_context);
                sockets.push_back(socket);
                acceptor.accept(*socket);
                spdlog::info("Connection accepted");

                threads.push_back(std::thread{handle_socket, std::ref(*socket)});
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
        delete sockets.back();
    }
    return 0;
}


void handle_socket(asio::ip::tcp::socket& socket) {
    try {
        Pipe pipe{std::move(socket)};
        BigInt key;
        TLS_Util tls_util(pipe, key);

        while (pipe) {
            try {
                auto receive_future = std::async(std::launch::async, [&]() {
                    std::string message;
                    pipe >> message;
                    return message;
                });
                
                if (receive_future.wait_for(std::chrono::seconds(10)) == std::future_status::timeout) {
                    spdlog::warn("Timeout - closing connection");
                    pipe.close();
                    socket.close();
                    return;
                } 
                std::string message = receive_future.get();

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
                    pipe.close();
                    socket.close();
                    return;
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