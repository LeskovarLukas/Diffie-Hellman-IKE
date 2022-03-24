#include <iostream>

#include "tls_client.h"


int main() {
    spdlog::set_level(spdlog::level::debug);

    
    try {
        asio::io_context io_context;

        std::shared_ptr<TLS_Client> client = std::make_shared<TLS_Client>(io_context, "localhost", "4433");
        client->run();

    } catch (std::exception& e) {
        spdlog::error("Client - Exception: {}", e.what());
    }

    return 0;
}
