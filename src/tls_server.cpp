#include "tls_server.h"


void TLS_Server::start_accept() {
    spdlog::info("Server - Starting accept");

    acceptor.async_accept(
        [this](const std::error_code& ec, asio::ip::tcp::socket socket) {
            if (!ec) {
                spdlog::info("Server - Accepted connection");
                auto new_session = std::make_shared<Session>(std::move(socket));
                new_session->subscribe(shared_from_this());
                new_session->start();
                sessions.push_back(new_session);

                start_accept();
            } else {
                spdlog::error("Server - Error accepting connection: {}", ec.message());
            }
        }
    );
}

TLS_Server::TLS_Server(asio::io_context& io_context): 
    io_context(io_context), 
    acceptor(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 4433)) {

    start_accept();
}


void TLS_Server::notify(std::string message) {
    spdlog::info("Server - Received message: {}", message);
}