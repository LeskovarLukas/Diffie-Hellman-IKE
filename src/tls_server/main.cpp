#include <iostream>
#include <asio.hpp>
#include <BigInt.hpp>
#include <spdlog/spdlog.h>
#include <json.hpp>
#include <fstream>


#include "include/pipe.h"

using json = nlohmann::json;


void read_primes_json(std::string, json&, BigInt&, BigInt&);

int main() {
    asio::io_context io_context;
    asio::ip::tcp::endpoint endpoint{asio::ip::tcp::v4(), 4433};
    spdlog::set_level(spdlog::level::debug);

    std::string message;


    try {
        asio::ip::tcp::acceptor acceptor{io_context, endpoint};
        asio::ip::tcp::socket socket{io_context};

        spdlog::info("Waiting for connections");
        acceptor.listen();

        while (true) {
            acceptor.accept(socket);
            spdlog::info("Connection accepted");
            Pipe pipe{std::move(socket)};

            pipe >> message;
            spdlog::debug("Message: {}", message);
            
            if (message == "TLS_DHE") {
                BigInt G;
                BigInt P;
                json primes;
                read_primes_json("../modp_primes.json", primes, G, P);

                int s = 3;
                BigInt S = pow(G, s) % P;
                spdlog::debug("S: {}", S.to_string());

                pipe << "G_" + G.to_string() + "|P_" + P.to_string() + "|S_" + S.to_string();

                pipe >> message;
                spdlog::debug("Message: {}", message);
                BigInt C = BigInt(message.substr(2));

                BigInt K = pow(C, s) % P;
                spdlog::info("Key: {}", K.to_string());
            }
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}

void read_primes_json(std::string filename, json& j, BigInt& g, BigInt& p) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Error opening file" << std::endl;
        return;
    }
    file >> j;
    file.close();
    g = int(j["groups"][0]["g"]); 
    p = std::string(j["groups"][0]["p_dec"]);
}
