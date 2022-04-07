/*
author: Leskovar Lukas
matnr: i17057
file: server.cpp
desc: This file contains the main function of the server. It provides a CLI and runs the tls server.
date: 2022-04-06
class: 5b
catnr: 10
*/


#include "tls_server.h"

#include "CLI11.hpp"
#include <spdlog/spdlog.h>

#include <iostream>


int main(int argc, char* argv[]) {
    CLI::App app{"tls_server"};

    int port = 4433;
    unsigned int timeout = 10000;
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
    app.add_option("-t,--timeout", timeout, "Timeout");
    app.add_option("-l,--log-level", log_level, "Log level")->transform(CLI::CheckedTransformer(log_level_map, CLI::ignore_case));


    CLI11_PARSE(app, argc, argv);


    spdlog::set_level(log_level);

    try {
        asio::io_context io_context(1);

        std::shared_ptr<TLS_Server> server_ptr = std::make_shared<TLS_Server>(io_context, port, timeout);

        io_context.run();

    } catch (std::exception& e) {
        spdlog::error("Server - {}", e.what());
    }

    return 0;
}
