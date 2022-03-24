#include <iostream>

#include "tls_server.h"


int main() {
    spdlog::set_level(spdlog::level::debug);

    try {
        asio::io_context io_context(1);

        std::shared_ptr<TLS_Server> server_ptr = std::make_shared<TLS_Server>(io_context);

        io_context.run();

    } catch (std::exception& e) {
        spdlog::error("Server - Exception: {}", e.what());
    }

    return 0;
}
