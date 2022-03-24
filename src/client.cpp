#include <iostream>

#include "tls_client.h"


int main() {
    spdlog::set_level(spdlog::level::debug);

    
    try {
        asio::io_context io_context;

        TLS_Client client(io_context, "localhost", "4433");
        client.run();

    } catch (std::exception& e) {
        spdlog::error("Client - Exception: {}", e.what());
    }

    return 0;
}
