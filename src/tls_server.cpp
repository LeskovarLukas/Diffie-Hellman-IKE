#include "tls_server.h"
#include "session.hpp"


void TLS_Server::start_accept() {
    spdlog::info("Server - Starting accept");

    acceptor.async_accept(
        [this](const std::error_code& ec, asio::ip::tcp::socket socket) {
            if (!ec) {
                spdlog::info("Server - Accepted connection");
                auto new_session= std::make_shared<session>(std::move(socket));
                new_session->start();

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