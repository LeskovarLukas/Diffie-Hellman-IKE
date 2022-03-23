#pragma once

#include <asio.hpp>
#include <spdlog/spdlog.h>

#include "pipe.hpp"


class session {
private:
    Pipe pipe;

    std::thread listen_thread;


    void listen_for_messages() {
        spdlog::debug("Session - Listening for messages");
        try {
            while (true) {
                std::string message;
                pipe.receive(message);

                spdlog::info("Session - Received message: {}", message);
            }
        } catch (std::exception& e) {
            spdlog::error("Session - Exception: {}", e.what());
        }
    }

public:
    session(asio::ip::tcp::socket socket): 
        pipe(std::move(socket)) {
        
        spdlog::debug("Session - Creating session");
    }

    void start() {
        spdlog::debug("Session - Starting session");

        listen_thread = std::thread([this] {
            listen_for_messages();
        });
        listen_thread.detach();
    }

    void send(const std::string& message) {
        spdlog::debug("Session - Sending message");
        pipe.send(message);
    }
};