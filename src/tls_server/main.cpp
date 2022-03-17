#include <iostream>
#include <asio.hpp>
#include <fstream>
#include <spdlog/spdlog.h>

#include "BigInt/BigInt.hpp"

#include "pipe.h"
#include "utility.h"
#include "tls_util.h"

void establish_secure_connection(Pipe&, BigInt&);

void handle_socket(asio::ip::tcp::socket&);

int main() {
    asio::io_context io_context;
    asio::ip::tcp::endpoint endpoint{asio::ip::tcp::v4(), 4433};
    asio::ip::tcp::acceptor acceptor{io_context, endpoint};
    asio::ip::tcp::socket socket{io_context};
    
    spdlog::set_level(spdlog::level::debug);
 
    try {
        spdlog::info("Waiting for connections");
        acceptor.listen();

        while (true) {
            acceptor.accept(socket);
            spdlog::info("Connection accepted");
            
            handle_socket(socket);
        }
    } catch (std::exception& e) {
        spdlog::error(e.what());
    }
    acceptor.close();
    return 0;
}


void establish_secure_connection(Pipe& pipe, BigInt& K) {
    BigInt G;
    BigInt P;
    read_primes_json("../modp_primes.json", 0, G, P);

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

void handle_socket(asio::ip::tcp::socket& socket) {
    std::string message;

    try {
        Pipe pipe{std::move(socket)};
        TLS_Util tls_util{pipe};

        while (true) {
            try {
                pipe >> message;

                if (message == "") {
                    spdlog::warn("Received empty message");
                    continue;
                }

                std::vector<std::string> message_parts;
                split_message(message, message_parts);
                
                if (message_parts[0] == "DATA" && tls_util.is_secure()) {
                    BigInt key = tls_util.get_key();
                    message = receive_message(key, std::stoul(message_parts[1]), message_parts[2]);
                    spdlog::info("Received Message: {}", message);
                } else if (message_parts[0] == "CLOSE") {
                    break;
                } else {
                    tls_util.handle_message(message_parts);
                }
            } catch (std::exception& e) {
                if (!pipe) 
                    throw std::runtime_error("Pipe connection lost");
            }
        }
    } catch (std::exception& e) {
        spdlog::error(e.what());
    }
    spdlog::info("Closing client connection");
}