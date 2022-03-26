#include <iostream>
#include "tls_client.h"
#include "messagebuilder.h"

TLS_Client::TLS_Client(asio::io_context& io_context, std::string host, std::string port):
    io_context(io_context),
    resolver(io_context),
    socket(io_context) {

    endpoints = resolver.resolve(host, port);
    asio::connect(socket, endpoints);
    session = std::make_shared<Session>(std::move(socket));
    session->start();
    spdlog::info("Client - Connected to {}:{}", host, port);
}


void TLS_Client::run() {
    session->subscribe(shared_from_this());

    while (true) {
        std::string input;
        std::getline(std::cin, input);

        tls::MessageWrapper message = Messagebuilder::build_application_message(input.size(), input);
        session->send(message);

        if (input == "quit") {
            break;
        }
    }
}


void TLS_Client::notify(tls::MessageWrapper message) {
    spdlog::debug("Client - Received message {}", message.type());

    if (message.type() == tls::MessageType::DATA) {
        tls::MessageWrapper::ApplicationData application_data = message.application_data();
        spdlog::info("Client - Received message: {}", application_data.data());
    }
}

