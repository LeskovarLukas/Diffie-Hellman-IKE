#include <iostream>
#include <vector>
#include <BigInt.hpp>
#include <spdlog/spdlog.h>
#include <picosha2.h>


#include "include/pipe.h"
#include "include/aes_utility.h"

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

    unsigned long size;
    std::string encrypted_message = Crypto_Utility::encrypt("Hello World", K.to_string(), size);
    spdlog::info("Encrypted message: {}", encrypted_message);
    
    std::string decrypted_message = Crypto_Utility::decrypt(encrypted_message, K.to_string(), size);
    spdlog::info("Decrypted message: {}", decrypted_message);
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