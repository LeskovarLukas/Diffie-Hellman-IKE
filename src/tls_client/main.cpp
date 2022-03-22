#include <iostream>
#include <vector>

#include <spdlog/spdlog.h>
#include "CLI11.hpp"

#include "tls_client.h"


int main(int argc, char* argv[]) {
    CLI::App app{"tls_client"};

    std::string host = "localhost";
    int port = 4433;
    int delay = 0;
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
    app.add_option("-d,--delay", delay, "Delay");
    app.add_option("-l,--log-level", log_level, "Log level")->transform(CLI::CheckedTransformer(log_level_map, CLI::ignore_case));


    CLI11_PARSE(app, argc, argv);

    spdlog::set_level(log_level);


    try {
        Pipe pipe(host, std::to_string(port));
        pipe.set_delay(std::chrono::milliseconds(delay));
        
        TLS_Client client(pipe);
        client.run();

    } catch (std::exception& e) {
        spdlog::error(e.what());
    }
    
    return 0;
}
