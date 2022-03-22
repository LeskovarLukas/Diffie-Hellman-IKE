#include <iostream>
#include <fstream>
#include <spdlog/spdlog.h>
#include <future>

#include "CLI11.hpp"

#include "tls_server.h"


void handle_socket(asio::ip::tcp::socket&);

int main(int argc, char* argv[]) {
    CLI::App app{"tls_server"};

    int port = 4433;
    spdlog::level::level_enum log_level = spdlog::level::info;
    std::map<std::string, spdlog::level::level_enum> log_level_map = {
        {"trace", spdlog::level::trace},
        {"debug", spdlog::level::debug},
        {"info", spdlog::level::info},
        {"warn", spdlog::level::warn},
        {"error", spdlog::level::err},
        {"critical", spdlog::level::critical}
    };

    app.add_option("-p,--port", port, "Port");
    app.add_option("-l,--log-level", log_level, "Log level")->transform(CLI::CheckedTransformer(log_level_map, CLI::ignore_case));


    CLI11_PARSE(app, argc, argv);


    spdlog::set_level(log_level);

    try {
        TLS_Server server(port);
        server.listen_for_connections();
        
    } catch (std::exception& e) {
        spdlog::error(e.what());
    }

    return 0;
}
