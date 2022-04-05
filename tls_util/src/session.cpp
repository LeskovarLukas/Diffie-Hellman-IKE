#include <spdlog/spdlog.h>

#include "session.h"


void Session::listen_for_messages() {
    spdlog::debug("Session - Listening for messages");
    try {
        while (pipe) {
            tls::Message_Wrapper message;
            pipe.receive(message);

            spdlog::debug("Session - Received message");
            notify(message);

        }
    } catch (std::exception& e) {
        spdlog::error("Session - Exception: {}", e.what());
    }
}


void Session::notify(tls::Message_Wrapper message) {
    for (auto observer : observers) {
        observer->notify(message, session_id);
    }
}


Session::Session(asio::ip::tcp::socket socket, unsigned int session_id): 
    pipe(Pipe(std::move(socket))), session_id(session_id) {
        
    spdlog::debug("Session - Creating session");
}

Session::~Session() {
    spdlog::debug("Session - Destroying session");
    listen_thread.join();
}


void Session::start() {
    spdlog::debug("Session - Starting session");

    listen_thread = std::thread([this] {
        listen_for_messages();
    });
    listen_thread.detach();
}


void Session::send(tls::Message_Wrapper message) {
    spdlog::debug("Session - Sending message");
    pipe.send(message);
}


void Session::subscribe(TLS_Observer_ptr observer) {
    observers.push_back(observer);
    spdlog::debug("Session - New observer");
}

void Session::unsubscribe(TLS_Observer_ptr observer) {
    observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
    spdlog::debug("Session - Observer removed");
}


unsigned int Session::get_session_id() const {
    return session_id;
}