#pragma once

#include "pipe.h"
#include "tls_observer.h"


class Session {
private:
    Pipe pipe;
    unsigned int session_id;
    std::vector<std::shared_ptr<TLS_Observer>> observers;

    std::thread listen_thread;


    void listen_for_messages() {
        spdlog::debug("Session - Listening for messages");
        try {
            while (pipe) {
                tls::MessageWrapper message;
                pipe.receive(message);

                spdlog::debug("Session - Received message");
                notify(message);

            }
        } catch (std::exception& e) {
            spdlog::error("Session - Exception: {}", e.what());
        }
    }

    void notify(tls::MessageWrapper message) {
        for (auto observer : observers) {
            observer->notify(message, session_id);
        }
    }

public:
    Session(asio::ip::tcp::socket socket, unsigned int session_id): 
        pipe(Pipe(std::move(socket))), session_id(session_id) {
            
        spdlog::debug("Session - Creating session");
    }

    ~Session() {
        spdlog::debug("Session - Destroying session");
        listen_thread.join();
    }

    void start() {
        spdlog::debug("Session - Starting session");

        listen_thread = std::thread([this] {
            listen_for_messages();
        });
        listen_thread.detach();
    }

    void send(tls::MessageWrapper message) {
        spdlog::debug("Session - Sending message");
        pipe.send(message);
    }


    void subscribe(TLS_Observer_ptr observer) {
        observers.push_back(observer);
        spdlog::debug("Session - New observer");
    }

    void unsubscribe(TLS_Observer_ptr observer) {
        observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
        spdlog::debug("Session - Observer removed");
    }


    unsigned int get_session_id() const {
        return session_id;
    }
};