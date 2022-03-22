#pragma once

#include <asio.hpp>
#include <vector>
#include <thread>
#include <future>

#include "tls_util.h"

class TLS_Server {
private:
    std::vector<std::thread> threads;
    std::vector<asio::ip::tcp::socket*> sockets;
    asio::io_context io_context;
    asio::ip::tcp::endpoint endpoint;
    asio::ip::tcp::acceptor acceptor;


    static void handle_socket(asio::ip::tcp::socket& socket) {
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

public:
    TLS_Server(int port): 
        endpoint{asio::ip::tcp::v4(), (asio::ip::port_type) port}, 
        acceptor{io_context, endpoint} {
    }

    ~TLS_Server() {
        for (auto& thread: threads) {
            thread.join();
        }

        for (auto& socket: sockets) {
            delete socket;
        }
    }

    void listen_for_connections() {
        try {
            spdlog::info("Waiting for connections");
            acceptor.listen();

            while (true) {
                asio::ip::tcp::socket* socket = new asio::ip::tcp::socket(io_context);
                sockets.push_back(socket);
                acceptor.accept(*socket);
                spdlog::info("Connection accepted");

                threads.push_back(std::thread(TLS_Server::handle_socket, std::ref(*socket)));
            }
        } catch (std::exception& e) {
            spdlog::error(e.what());
        }
        acceptor.close();
    }
};