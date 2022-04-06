#include "tls_client.h"

#include "CLI11.hpp"
#include <spdlog/spdlog.h>

#include <iostream>


int main(int argc, char* argv[]) {
    CLI::App app{"tls_client"};

    std::string host = "localhost";
    std::string port = "4433";
    spdlog::level::level_enum log_level = spdlog::level::info;
    std::map<std::string, spdlog::level::level_enum> log_level_map = {
        {"trace", spdlog::level::trace},
        {"debug", spdlog::level::debug},
        {"info", spdlog::level::info},
        {"warn", spdlog::level::warn},
        {"error", spdlog::level::err},
        {"critical", spdlog::level::critical}
    };


    app.add_option("-n,--hostname", host, "Hostname");
    app.add_option("-p,--port", port, "Port");
    app.add_option("-l,--log-level", log_level, "Log level")->transform(CLI::CheckedTransformer(log_level_map, CLI::ignore_case));

    CLI11_PARSE(app, argc, argv);

    spdlog::set_level(log_level);

    
    try {
        asio::io_context io_context;

        std::shared_ptr<TLS_Client> client = std::make_shared<TLS_Client>(io_context, host, port);
        client->run();

    } catch (std::exception& e) {
        spdlog::error("Client - {}", e.what());
    }

    return 0;
}
