/*
author: Leskovar Lukas
matnr: i17057
file: tls_server.cpp
desc: See tls_server.h
date: 2022-04-06
class: 5b
catnr: 10
*/


#include "tls_server.h"

#include <spdlog/spdlog.h>


void TLS_Server::start_accept() {
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

                auto new_ping_agent = std::make_shared<Ping_Agent>(new_session, timeout);
                new_session->subscribe(new_ping_agent);
                ping_agents.push_back(new_ping_agent);
                new_ping_agent->run();


                start_accept();
            } else {
                spdlog::error("Server - Error accepting connection: {}", ec.message());
            }
        }
    );
}


TLS_Server::TLS_Server(asio::io_context& io_context, int port, unsigned int timeout): 
    io_context(io_context), 
    acceptor(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
    timeout(timeout) {

    start_accept();
}


void TLS_Server::notify(tls::Message_Wrapper message, unsigned int session_id) {
    spdlog::debug("Server Session {} - Received message type {}", session_id, message.type());

    if (message.type() == tls::Message_Type::DATA) {
        std::shared_ptr<TLS_Handshake_Agent> handshake_agent = handshake_agents.at(session_id);

        if (handshake_agent->is_secure()) {
            std::string key = handshake_agent->get_key();
            unsigned long size = message.application_data().size();
            std::string decrypted_message = TLS_Handshake_Agent::receive_message(key, size, message.application_data().data());
            std::cout << "Session " << session_id << " > " << decrypted_message << std::endl;

            send(session_id, "Pong: " + decrypted_message);
        } else {
            spdlog::warn("Server - Received unsecure message");
            std::cout << "Session " << session_id << " > " << message.application_data().data() << std::endl;

            send(session_id, "Pong: " + message.application_data().data());
        }
    } else if (message.type() == tls::Message_Type::CLOSE) {
        spdlog::info("Server Session {} - Received close message", session_id);
        sessions.at(session_id)->close();
    }
}


void TLS_Server::send(unsigned int session_id, std::string input) {
    spdlog::debug("Server Session {} - Sending message", session_id);

    std::shared_ptr<Session> session = sessions.at(session_id);
    std::shared_ptr<TLS_Handshake_Agent> handshake_agent = handshake_agents.at(session_id);

    if (handshake_agent->is_secure()) {
        std::string key = handshake_agent->get_key();
        unsigned long size;
        std::string encrypted_message = TLS_Handshake_Agent::send_message(key, size, input);
        tls::Message_Wrapper message = Messagebuilder::build_application_message(size, encrypted_message);
        session->send(message);
    } else {
        tls::Message_Wrapper message = Messagebuilder::build_application_message(input.size(), input);
        session->send(message);
    }
}