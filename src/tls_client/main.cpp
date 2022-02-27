#include <iostream>
#include <vector>
#include <BigInt.hpp>
#include <spdlog/spdlog.h>


#include "include/pipe.h"
#include "include/crypto_utility.h"

void split_message(std::string, std::vector<std::string>&);

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
    int c = 2;
    BigInt C = pow(G, c) % P;
    spdlog::debug("Sending C: {}", C.to_string());

    pipe << "C_" + C.to_string();

    BigInt K = pow(S, c) % P;
    spdlog::info("Key: {}", K.to_string());

    Crypto_Utility crypto_utility(K.to_string());
    std::string encrypted = crypto_utility.encrypt("Hello world!Hello world!Hello world!");

    pipe << "SIZE_" + std::to_string(crypto_utility.get_size()) + 
    "|" + "MSG_" + encrypted +
    "|" + "IV_" + crypto_utility.get_iv();
    
    return 0;
}

void split_message(std::string message, std::vector<std::string>& parts) {
    std::stringstream ss(message);
    std::string part;
    while (std::getline(ss, part, '|')) {
        part = part.substr(2);
        parts.push_back(part);
    }
}