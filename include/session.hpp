#pragma once

#include <asio.hpp>
#include <spdlog/spdlog.h>

#include "pipe.hpp"
#include "tls_observer.hpp"


class Session {
private:
    Pipe pipe;
    std::vector<std::shared_ptr<TLS_Observer>> observers;

    std::thread listen_thread;


    void listen_for_messages() {
        spdlog::debug("Session - Listening for messages");
        try {
            while (true) {
                std::string message;
                pipe.receive(message);

                spdlog::info("Session - Received message: {}", message);
                notify(message);

            }
        } catch (std::exception& e) {
            spdlog::error("Session - Exception: {}", e.what());
        }
    }

    void notify(std::string message) {
        for (auto observer : observers) {
            observer->notify(message);
        }
    }

public:
    Session(asio::ip::tcp::socket socket): 
        pipe(Pipe(std::move(socket))) {
            
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


    void subscribe(TLS_Observer_ptr observer) {
        observers.push_back(observer);
    }

    void unsubscribe(TLS_Observer_ptr observer) {
        observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
    }
};