/*
author: Leskovar Lukas
matnr: i17057
file: tls_client.cpp
desc: See tls_client.h
date: 2022-04-06
class: 5b
catnr: 10
*/


#include "tls_client.h"

#include <spdlog/spdlog.h>

#include <iostream>


TLS_Client::TLS_Client(asio::io_context& io_context, std::string host, std::string port):
    io_context(io_context),
    resolver(io_context),
    socket(io_context) {

    endpoints = resolver.resolve(host, port);
    asio::connect(socket, endpoints);
    session = std::make_shared<Session>(std::move(socket), 0);
    session->start();
    spdlog::info("Client - Connected to {}:{}", host, port);

    handshake_agent = std::make_shared<TLS_Handshake_Agent>(session);
    session->subscribe(handshake_agent);
}


void TLS_Client::run() {
    session->subscribe(shared_from_this());

    handshake_agent->initiate_handshake();

    while (session) {
        if (!handshake_agent->is_establishing()) {
            std::string input;
            std::getline(std::cin, input);

            tls::Message_Wrapper message;

            if (input == "quit") {
                message = Messagebuilder::build_close_message();
                session->send(message);
                break;
            }

            if (handshake_agent->is_secure()) {
                std::string key = handshake_agent->get_key();
                unsigned long size;
                std::string encrypted_message = TLS_Handshake_Agent::send_message(key, size, input);
                message = Messagebuilder::build_application_message(size, encrypted_message);
            } else {
                message = Messagebuilder::build_application_message(input.size(), input);
            }
            
            session->send(message);
        }
    }
}


void TLS_Client::notify(tls::Message_Wrapper message, unsigned int session_id) {
    spdlog::debug("Client Session {} - Received message type {}", session_id, message.type());

    if (message.type() == tls::Message_Type::DATA) {
        if (handshake_agent->is_secure()) {
            std::string key = handshake_agent->get_key();
            unsigned long size = message.application_data().size();
            std::string decrypted_message = TLS_Handshake_Agent::receive_message(key, size, message.application_data().data());
            std::cout << "> " << decrypted_message << std::endl;
        } else {
            spdlog::warn("Client - Received unsecure message");
            std::cout << "> " << message.application_data().data() << std::endl;
        }
    } else if (message.type() == tls::Message_Type::CLOSE) {
        spdlog::info("Client - Received close message");
        session->close();
    }
} 


void TLS_Client::set_delay(unsigned int delay) {
    session->set_delay(delay);
}