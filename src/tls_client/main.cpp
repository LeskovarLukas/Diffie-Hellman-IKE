#include <iostream>
#include <vector>

#include "BigInt/BigInt.hpp"
#include "spdlog/spdlog.h"

#include "pipe.h"
#include "utility.h"

void establish_secure_connection(Pipe&, BigInt&);


int main() {
    spdlog::set_level(spdlog::level::debug);

    try {
        State currentState = UNSECURED;
        Pipe pipe;
        BigInt G;
        BigInt P;
        BigInt S;
        BigInt c;
        BigInt C;
        BigInt key = -1;
        std::string clientCommunication{};
        std::string serverCommunication{};
    
        while (true) {  
            try {
                if (currentState == State::UNSECURED) {
                    spdlog::info("Initiating key exchange");
                    pipe << "TYPE_CLIENTHELLO";
                    currentState = ESTABLISHING;
                } else if (currentState == State::ESTABLISHING) {
                    std::string message;
                    pipe >> message;
                    std::vector<std::string> parts;
                    split_message(message, parts);

                    if (parts[0] == "SERVERHELLO") {
                        int group_id = std::stoi(parts[1]);
                        read_primes_json("../modp_primes.json", group_id, G, P);

                        c = generate_random_number(1, P);
                        C = pow(G, c.to_int()) % P;

                    } else if (parts[0] == "CERTIFICATE") {
                        S = BigInt(parts[1]);
                        key = pow(S, c.to_int()) % P;
                    } else if (parts[0] == "SERVERHELLODONE") {
                        pipe << "TYPE_CLIENTKEYEXCHANGE|C_" + C.to_string();

                        picosha2::hash256_hex_string("PRIMEGROUP_0|S_" + S.to_string() + "|C_" + C.to_string(), clientCommunication);
                        clientCommunication.resize(66);
                        spdlog::debug("Client communication: {}", clientCommunication);
                        pipe << "TYPE_CHANGECIPHERSPEC|" + send_message(key, clientCommunication);

                        pipe << "TYPE_FINISHED";
                    } else if (parts[0] == "CHANGECIPHERSPEC") {
                        serverCommunication = receive_message(key, std::stoul(parts[1]), parts[2]);
                        serverCommunication.resize(66);
                        spdlog::debug("Server communication: {}", serverCommunication);
                    } else if (parts[0] == "FINISHED") {
                        if (serverCommunication == clientCommunication) {
                            spdlog::info("Secure connection established");
                            currentState = State::SECURED;
                        } else {
                            spdlog::error("Secure connection failed");
                            currentState = State::UNSECURED;
                            pipe << "TYPE_ABORT";
                        }
                    } else if (parts[0] == "ABORT") {
                        spdlog::error("Secure connection failed");
                        currentState = State::UNSECURED;
                    } else {
                        spdlog::info("Received unknown message");
                        currentState = State::UNSECURED;
                        pipe << "TYPE_ABORT";
                    }
                } else if (currentState == State::SECURED) {
                    std::string input;
                    std::cout << "Enter message: ";
                    std::getline(std::cin, input);
                    pipe << "TYPE_DATA|" + send_message(key, input);
                }
            } catch (std::exception& e) {
                spdlog::warn(e.what());

                if (!pipe) {
                    spdlog::info("Trying to reconnect");
                    for (int i = 0; i < 10; i++) {
                        try {
                            pipe.try_reconnect();
                            key = -1;
                            spdlog::info("Reconnected");
                            break;
                        } catch (std::exception& e) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                            if (i == 9) {
                                throw std::runtime_error("Could not reconnect");
                            }
                        }
                    }
                }
            }
        }
    } catch (std::exception& e) {
        spdlog::error(e.what());
    }
    
    return 0;
}

void establish_secure_connection(Pipe& pipe, BigInt& K) {
    pipe << "TYPE_TLSDHE";

    std::string message;
    pipe >> message;

    std::vector<std::string> parameters;
    split_message(message, parameters);

    BigInt G = parameters[0];
    BigInt P = parameters[1];
    BigInt S = parameters[2];
    spdlog::debug("Received S: {}", S.to_string());
    BigInt c = generate_random_number(1, P);
    BigInt C = pow(G, c.to_int()) % P;
    spdlog::debug("Sending C: {}", C.to_string());

    pipe << "C_" + C.to_string();

    K = pow(S, c.to_int()) % P;
    spdlog::info("Key: {}", K.to_string());
}

