#pragma once

#include <spdlog/spdlog.h>
#include <iostream>

#include "session.h"
#include "tls_handshake_agent.h"
#include "messagebuilder.h"
#include "utility.h"


class TLS_Client: public TLS_Observer, public std::enable_shared_from_this<TLS_Observer> {
private:
    asio::io_context& io_context;
    asio::ip::tcp::resolver resolver;
    asio::ip::tcp::socket socket;
    asio::ip::tcp::resolver::results_type endpoints;
    std::shared_ptr<Session> session;
    std::shared_ptr<TLS_Handshake_Agent> handshake_agent;


public: 
    TLS_Client(asio::io_context& io_context, std::string host, std::string port):
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


    void run() {
        session->subscribe(shared_from_this());

        handshake_agent->initiate_handshake();

        while (session) {
            if (!handshake_agent->is_establishing()) {
                std::string input;
                std::getline(std::cin, input);

                tls::MessageWrapper message;
                if (handshake_agent->is_secure()) {
                    std::string key = handshake_agent->get_key();
                    unsigned long size;
                    std::string encrypted_message = Utility::send_message(key, size, input);
                    message = Messagebuilder::build_application_message(size, encrypted_message);
                } else {
                    message = Messagebuilder::build_application_message(input.size(), input);
                }

                session->send(message);

                if (input == "quit") {
                    break;
                }
            }
        }
    }


    void notify(tls::MessageWrapper message, unsigned int session_id) {
        spdlog::debug("Client Session {} - Received message type {}", session_id, message.type());

        if (message.type() == tls::MessageType::DATA) {
            if (handshake_agent->is_secure()) {
                std::string key = handshake_agent->get_key();
                unsigned long size = message.application_data().size();
                std::string decrypted_message = Utility::receive_message(key, size, message.application_data().data());
                std::cout << "> " << decrypted_message << std::endl;
            } else {
                spdlog::warn("Client - Received unsecure message");
                std::cout << "> " << message.application_data().data() << std::endl;
            }
        }
    } 
};