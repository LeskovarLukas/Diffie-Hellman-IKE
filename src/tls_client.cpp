#include <iostream>
#include "tls_client.h"

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
    while (true) {
        std::string message;
        std::getline(std::cin, message);

        session->send(message);

        if (message == "quit") {
            break;
        }
    }
}

