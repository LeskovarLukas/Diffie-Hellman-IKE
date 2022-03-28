#pragma once

#include <string>
#include "BigInt/BigInt.hpp"

class Utility {
private:
    static const unsigned char iv[16];


public:

    /*
    General Utility Functions
    */

    static BigInt generate_random_number(BigInt min, BigInt max);


    static void read_primes_json(std::string filename, int id, BigInt& g, BigInt& p);


    /*
    Encrytion Utility Functions
    */

    static std::string encrypt(const std::string& message, unsigned long& size, const std::string& key_string);


    static std::string decrypt(const std::string& message, unsigned long& size, const std::string& key_string);


    /*
    Encoding Utility Functions
    https://gist.github.com/tomykaira/f0fd86b6c73063283afe550bc5d77594
    */

    static std::string encode_base64(const std::string& message);

    static std::string decode_base64(const std::string& message);


    /*
    Messaging Functions
    */

    static std::string send_message(std::string key, unsigned long& size, std::string message);


    static std::string receive_message(std::string key, unsigned long size, std::string message);
};