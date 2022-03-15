#include <iostream>
#include <asio.hpp>
#include <fstream>
#include <spdlog/spdlog.h>

#include "BigInt/BigInt.hpp"

#include "pipe.h"
#include "utility.h"

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
    BigInt G;
    BigInt P;
    BigInt s;
    BigInt S;
    BigInt C;
    BigInt key = -1;
    std::string serverCommunication = {};
    std::string clientCommunication = {};
    std::string message;

    try {
        State currentState = State::UNSECURED;
        Pipe pipe{std::move(socket)};

        while (true) {
            try {
                pipe >> message;
                if (message != "") {
                    std::vector<std::string> parts;
                    split_message(message, parts);

                    if (currentState == State::UNSECURED || currentState == ESTABLISHING) {
                        if (parts[0] == "CLIENTHELLO") {
                            spdlog::info("Establishing secure connection");
                            pipe << "TYPE_SERVERHELLO|PRIMEGROUP_0";
                            read_primes_json("../modp_primes.json", 0, G, P);

                            s = generate_random_number(1, P);
                            S = pow(G, s.to_int()) % P;

                            pipe << "TYPE_CERTIFICATE|S_" + S.to_string();

                            pipe << "TYPE_SERVERHELLODONE";
                            currentState = State::ESTABLISHING;
                        } else if (parts[0] == "CLIENTKEYEXCHANGE") {
                            C = BigInt(parts[1]);
                            key = pow(C, s.to_int()) % P;
                        } else if (parts[0] == "CHANGECIPHERSPEC") {
                            clientCommunication = receive_message(key, std::stoul(parts[1]), parts[2]);
                            clientCommunication.resize(66);
                            spdlog::debug("Client Communication: {}", clientCommunication);
                        } else if (parts[0] == "FINISHED") {
                            picosha2::hash256_hex_string("PRIMEGROUP_0|S_" + S.to_string() + "|C_" + C.to_string(), serverCommunication);
                            serverCommunication.resize(66);
                            
                            spdlog::debug("Server Communication: {}", serverCommunication);
                            pipe << "TYPE_CHANGECIPHERSPEC|" + send_message(key, serverCommunication);

                            if (serverCommunication == clientCommunication) {
                                spdlog::info("Secure connection established");
                                currentState = State::SECURED;
                                pipe << "TYPE_FINISHED";
                            } else {
                                spdlog::error("Secure connection failed");
                                currentState = State::UNSECURED;
                                pipe << "TYPE_ABORT";
                            }
                        } else if (parts[0] == "ABORT") {
                            spdlog::error("Secure connection failed");
                            currentState = State::UNSECURED;
                        } else {
                            spdlog::error("Received unknown message");
                            currentState = State::UNSECURED;
                            pipe << "TYPE_ABORT";
                        }
                    } else if (currentState == State::SECURED) {
                        if (parts[0] == "DATA") {
                            std::string information = receive_message(key, std::stoul(parts[1]), parts[2]);
                            spdlog::info("Received information: {}", information);
                        }
                    }
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