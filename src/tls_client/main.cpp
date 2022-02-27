#include <iostream>
#include <vector>
#include <BigInt.hpp>
#include <spdlog/spdlog.h>


#include "include/pipe.h"
#include "include/crypto_utility.h"
#include "utility.h"

int main() {
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Initiating key exchange");
    Pipe pipe;
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

    BigInt K = pow(S, c.to_int()) % P;
    spdlog::info("Key: {}", K.to_string());

    Crypto_Utility crypto_utility(K.to_string());
    std::string encrypted = crypto_utility.encrypt("Hello world!Hello world!Hello world!");

    pipe << "SIZE_" + std::to_string(crypto_utility.get_size()) + 
    "|" + "MSG_" + encrypted;
    
    return 0;
}