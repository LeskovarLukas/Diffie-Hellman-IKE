#include <iostream>
#include <vector>
#include <BigInt.hpp>
#include <spdlog/spdlog.h>


#include "pipe.h"
#include "utility.h"

void establish_secure_connection(Pipe&, BigInt&);

int main() {
    spdlog::set_level(spdlog::level::debug);

    try {
        Pipe pipe;
        BigInt key = -1;
    
        while (true) {  
            try {
                if (key == -1) {
                    spdlog::info("Initiating key exchange");
                    establish_secure_connection(pipe, key);
                } else {
                    std::string input;
                    std::cout << "Enter message: ";
                    std::getline(std::cin, input);
                    pipe << send_message(key, input);
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
    pipe << "TLS_DHE";

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

