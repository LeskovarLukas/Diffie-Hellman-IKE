#include <iostream>
#include <vector>
#include <BigInt.hpp>
#include <spdlog/spdlog.h>


#include "include/pipe.h"
#include "utility.h"

void establish_secure_connection(Pipe&, BigInt&);

int main() {
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Initiating key exchange");
    Pipe pipe;
    BigInt key;

    establish_secure_connection(pipe, key);

    std::string message = "Hello World";
    unsigned long size = 0;

    std::string encrypted = encrypt(message, size, key.to_string());
    spdlog::debug("Encrypted: {}", encrypted);

    encrypted = encode_base64(encrypted);

    pipe << "SIZE_" + std::to_string(size) + "|" + "MSG_" + encrypted;

    
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