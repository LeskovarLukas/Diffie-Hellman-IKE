#include <iostream>
#include <vector>
#include <BigInt.hpp>
#include <spdlog/spdlog.h>


#include "include/pipe.h"

void split_message(std::string, std::vector<std::string>&);

int main() {
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Initiating key exchange");
    Pipe pipe;
    pipe << "TLS_DHE";

    std::string message;
    pipe >> message;
    spdlog::debug("Message: {}" + message);

    std::vector<std::string> parameters;
    split_message(message, parameters);

    BigInt G = parameters[0];
    BigInt P = parameters[1];
    BigInt S = parameters[2];
    int c = 5;
    BigInt C = pow(G, c) % P;
    spdlog::debug("C: {}", C.to_string());

    pipe << "C_" + C.to_string();

    BigInt K = pow(S, c) % P;
    spdlog::info("Key: {}", K.to_string());

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