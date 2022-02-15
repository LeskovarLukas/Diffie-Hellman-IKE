#include <iostream>
#include <cmath>
#include <json.hpp>
#include <fstream>
#include <sstream>
#include "BigInt.hpp"

using json = nlohmann::json;

void read_primes_json(std::string, json&, BigInt&, BigInt&);

int main() {
    BigInt G;
    BigInt P;
    json primes;
    read_primes_json("../modp_primes.json", primes, G, P);

    int a = 4; //Alice private key
    int b = 3; //Bob private key

    BigInt A = pow(G, a) % P; //Alice public key
    BigInt B = pow(G, b) % P; //Bob public key

    BigInt keyA = pow(B, a) % P; //Alice shared key
    BigInt keyB = pow(A, b) % P; //Bob shared key

    std::cout << (keyA == keyB) << std::endl;
    std::cout << "keyA: " << keyA << std::endl;
    std::cout << "keyB: " << keyB << std::endl;
    return 0;
}

void read_primes_json(std::string filename, json& j, BigInt& g, BigInt& p) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Error opening file" << std::endl;
        return;
    }
    file >> j;
    file.close();
    g = int(j["groups"][0]["g"]); 
    p = std::string(j["groups"][0]["p_dec"]);
}
