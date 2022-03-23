#include <iostream>
#include <asio.hpp>
#include <spdlog/spdlog.h>

#include "session.hpp"
#include "tls_server.h"


int main() {
    spdlog::set_level(spdlog::level::debug);

    try {
        asio::io_context io_context(1);

        TLS_Server server(io_context);

        io_context.run();

    } catch (std::exception& e) {
        spdlog::error("Server - Exception: {}", e.what());
    }

    return 0;
}
