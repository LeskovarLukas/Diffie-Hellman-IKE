#pragma once

#include <spdlog/spdlog.h>

#include "session.h"
#include "tls_handshake_agent.h"
#include "utility.h"


class TLS_Server: public TLS_Observer, public std::enable_shared_from_this<TLS_Observer> {
private: 
    asio::io_context& io_context;
    asio::ip::tcp::acceptor acceptor;
    std::vector<std::shared_ptr<Session>> sessions;
    std::vector<std::shared_ptr<TLS_Handshake_Agent>> handshake_agents;

    void start_accept() {
        spdlog::info("Server - Starting accept");

        acceptor.async_accept(
            [this](const std::error_code& ec, asio::ip::tcp::socket socket) {
                if (!ec) {
                    spdlog::info("Server - Accepted connection");
                    auto new_session = std::make_shared<Session>(std::move(socket), sessions.size());
                    new_session->subscribe(shared_from_this());
                    new_session->start();
                    sessions.push_back(new_session);

                    auto new_handshake_agent = std::make_shared<TLS_Handshake_Agent>(new_session);
                    new_session->subscribe(new_handshake_agent);
                    handshake_agents.push_back(new_handshake_agent);


                    start_accept();
                } else {
                    spdlog::error("Server - Error accepting connection: {}", ec.message());
                }
            }
        );
    }

public:
    TLS_Server(asio::io_context& io_context, int port): 
        io_context(io_context), 
        acceptor(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {

        start_accept();
    }


    void notify(tls::MessageWrapper message, unsigned int session_id) {
        spdlog::debug("Server Session {} - Received message type {}", session_id, message.type());

        if (message.type() == tls::MessageType::DATA) {
            std::shared_ptr<TLS_Handshake_Agent> handshake_agent = handshake_agents.at(session_id);

            if (handshake_agent->is_secure()) {
                std::string key = handshake_agent->get_key();
                unsigned long size = message.application_data().size();
                std::string decrypted_message = Utility::receive_message(key, size, message.application_data().data());
                std::cout << "Session " << session_id << " > " << decrypted_message << std::endl;

                send(session_id, "Pong: " + decrypted_message);
            } else {
                spdlog::warn("Server - Received unsecure message");
                std::cout << "Session " << session_id << " > " << message.application_data().data() << std::endl;

                send(session_id, "Pong: " + message.application_data().data());
            }
        }
    }


    void send(unsigned int session_id, std::string input) {
        spdlog::debug("Server Session {} - Sending message", session_id);

        std::shared_ptr<Session> session = sessions.at(session_id);
        std::shared_ptr<TLS_Handshake_Agent> handshake_agent = handshake_agents.at(session_id);

        if (handshake_agent->is_secure()) {
            std::string key = handshake_agent->get_key();
            unsigned long size;
            std::string encrypted_message = Utility::send_message(key, size, input);
            tls::MessageWrapper message = Messagebuilder::build_application_message(size, encrypted_message);
            session->send(message);
        } else {
            tls::MessageWrapper message = Messagebuilder::build_application_message(input.size(), input);
            session->send(message);
        }
    }
};