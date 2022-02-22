#include <iostream>
#include <vector>
#include <BigInt.hpp>
#include <spdlog/spdlog.h>
#include <picosha2.h>


#include "include/pipe.h"

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

    std::string hash_hex_str = picosha2::hash256_hex_string(K.to_string());
    spdlog::info("Hash: {}", hash_hex_str);

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