#include <iostream>
#include <asio.hpp>
#include <BigInt.hpp>
#include <spdlog/spdlog.h>
#include <json.hpp>
#include <fstream>


#include "include/pipe.h"
#include "utility.h"

using json = nlohmann::json;


void read_primes_json(std::string, json&, BigInt&, BigInt&);

void establish_secure_connection(Pipe&, BigInt&);


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
            BigInt key;

            pipe >> message;
            spdlog::debug("Message: {}", message);
            
            if (message == "TLS_DHE") {
                establish_secure_connection(pipe, key);

                pipe >> message;
                spdlog::debug("Message: {}", message);

                std::vector<std::string> parts;
                split_message(message, parts);

                std::string encrypted = decode_base64(parts[1]);
                spdlog::debug("Encrypted: {}", encrypted);

                unsigned long size = std::stoul(parts[0]);
                std::string decrypted = decrypt(encrypted, size, key.to_string());
                spdlog::info("Decrypted message: {}", decrypted);
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


void establish_secure_connection(Pipe& pipe, BigInt& K) {
    BigInt G;
    BigInt P;
    json primes;
    read_primes_json("../modp_primes.json", primes, G, P);

    BigInt s = generate_random_number(1, P);
    BigInt S = pow(G, s.to_int()) % P;
    spdlog::debug("Sending S: {}", S.to_string());

    pipe << "G_" + G.to_string() + "|P_" + P.to_string() + "|S_" + S.to_string();

    std::string message;
    pipe >> message;
    BigInt C = BigInt(message.substr(2));
    spdlog::debug("Received C: {}", C.to_string());

    K = pow(C, s.to_int()) % P;
    spdlog::info("Key: {}", K.to_string());
}