#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include <BigInt.hpp>

void split_message(std::string& message, std::vector<std::string>& parts) {
    std::stringstream ss(message);
    std::string part;
    while (std::getline(ss, part, '|')) {
        part = part.substr(part.find_first_of('_') + 1);
        parts.push_back(part);
    }
}

BigInt generate_random_number(BigInt min, BigInt max) {
    int randomData = open("/dev/urandom", O_RDONLY);
    return (randomData % (max - min + 1)) + min;
}