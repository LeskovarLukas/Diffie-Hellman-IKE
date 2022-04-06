#include "session.h"

#include <spdlog/spdlog.h>


void Session::listen_for_messages() {
    spdlog::debug("Session {} - Listening for messages", session_id);
    try {
        while (pipe) {
            tls::Message_Wrapper message;
            pipe.receive(message);

            spdlog::debug("Session {} - Received message", session_id);
            notify(message);

        }
    } catch (std::exception& e) {
        spdlog::error("Session {} - Error: {}", session_id, e.what());
    }
}


void Session::notify(tls::Message_Wrapper message) {
    for (auto observer : observers) {
        observer->notify(message, session_id);
    }
}


Session::Session(asio::ip::tcp::socket socket, unsigned int session_id): 
    pipe(Pipe(std::move(socket))), session_id(session_id) {
        
    spdlog::debug("Session {} - Creating session", session_id);
}

Session::~Session() {
    spdlog::debug("Session {} - Destroying session", session_id);
    listen_thread.join();
}


void Session::start() {
    spdlog::debug("Session {} - Starting session", session_id);

    listen_thread = std::thread([this] {
        listen_for_messages();
    });
    listen_thread.detach();
}


void Session::close() {
    spdlog::debug("Session {} - Closing session", session_id);
    pipe.close();
}


void Session::send(tls::Message_Wrapper message) {
    spdlog::debug("Session {} - Sending message", session_id);
    pipe.send(message);
}


void Session::subscribe(TLS_Observer_ptr observer) {
    observers.push_back(observer);
    spdlog::debug("Session {} - New observer", session_id);
}

void Session::unsubscribe(TLS_Observer_ptr observer) {
    observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
    spdlog::debug("Session {} - Observer removed", session_id);
}


unsigned int Session::get_session_id() const {
    return session_id;
}