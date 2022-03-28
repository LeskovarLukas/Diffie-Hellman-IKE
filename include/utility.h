#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <json.hpp>
#include <spdlog/spdlog.h>

#include "PicoSHA2/picosha2.h"
#include "BigInt/BigInt.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "plusaes/plusaes.hpp"
#pragma GCC diagnostic pop


class Utility {
private:
    inline static const unsigned char iv[16] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
    };


public:

    /*
    General Utility Functions
    */


    static BigInt generate_random_number(BigInt min, BigInt max) {
        int randomData = open("/dev/urandom", O_RDONLY);
        return (randomData % (max - min + 1)) + min;
    }


    static void read_primes_json(std::string filename, int id, BigInt& g, BigInt& p) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cout << "Error opening file" << std::endl;
            return;                      
        }
        nlohmann::json primes;
        file >> primes;
        file.close();
        g = int(primes["groups"][id]["g"]); 
        p = std::string(primes["groups"][id]["p_dec"]);
    }


    /*
    Encrytion Utility Functions
    */

    static std::string encrypt(const std::string& message, unsigned long& size, const std::string& key_string) {
        std::vector<unsigned char> key(32);
        picosha2::hash256(key_string.begin(), key_string.end(), key);

        size = plusaes::get_padded_encrypted_size(message.size());
        std::vector<unsigned char> encrypted(size);

        plusaes::encrypt_cbc(
            (unsigned char*)message.data(), message.size(), 
            &key[0], key.size(), 
            &iv, 
            &encrypted[0], encrypted.size(), 
            true
        );

        return std::string(encrypted.begin(), encrypted.end());    
    }


    static std::string decrypt(const std::string& message, unsigned long& size, const std::string& key_string) {
        std::vector<unsigned char> key(32);
        picosha2::hash256(key_string.begin(), key_string.end(), key);

        unsigned long padded_size = 0;
        std::vector<unsigned char> encrypted(message.begin(), message.end());
        std::vector<unsigned char> decrypted(size);

        plusaes::decrypt_cbc(
            &encrypted[0], encrypted.size(), 
            &key[0], key.size(), 
            &iv, 
            &decrypted[0], decrypted.size(), 
            &padded_size
        );

        return std::string(decrypted.begin(), decrypted.end());    
    }


    /*
    Encoding Utility Functions
    https://gist.github.com/tomykaira/f0fd86b6c73063283afe550bc5d77594
    */

    static std::string encode_base64(const std::string& message) {
        static constexpr char sEncodingTable[] = {
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
            'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
            'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
            'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
            'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
            'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
            'w', 'x', 'y', 'z', '0', '1', '2', '3',
            '4', '5', '6', '7', '8', '9', '+', '/'
        };

        size_t in_len = message.size();
        size_t out_len = 4 * ((in_len + 2) / 3);
        std::string ret(out_len, '\0');
        size_t i;
        char *p = const_cast<char*>(ret.c_str());

        for (i = 0; i < in_len - 2; i += 3) {
            *p++ = sEncodingTable[(message[i] >> 2) & 0x3F];
            *p++ = sEncodingTable[((message[i] & 0x3) << 4) | ((int) (message[i + 1] & 0xF0) >> 4)];
            *p++ = sEncodingTable[((message[i + 1] & 0xF) << 2) | ((int) (message[i + 2] & 0xC0) >> 6)];
            *p++ = sEncodingTable[message[i + 2] & 0x3F];
        }
        if (i < in_len) {
            *p++ = sEncodingTable[(message[i] >> 2) & 0x3F];
            if (i == (in_len - 1)) {
            *p++ = sEncodingTable[((message[i] & 0x3) << 4)];
            *p++ = '=';
            }
            else {
            *p++ = sEncodingTable[((message[i] & 0x3) << 4) | ((int) (message[i + 1] & 0xF0) >> 4)];
            *p++ = sEncodingTable[((message[i + 1] & 0xF) << 2)];
            }
            *p++ = '=';
        }

        return ret;
    }

    static std::string decode_base64(const std::string& message) {
        static constexpr unsigned char kDecodingTable[] = {
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
            52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
            64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
            15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
            64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
            41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
        };

        size_t in_len = message.size();
        if (in_len % 4 != 0) return "Input data size is not a multiple of 4";

        size_t out_len = in_len / 4 * 3;
        if (message[in_len - 1] == '=') out_len--;
        if (message[in_len - 2] == '=') out_len--;

        std::string out(out_len, '\0');

        for (size_t i = 0, j = 0; i < in_len;) {
            uint32_t a = message[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<int>(message[i++])];
            uint32_t b = message[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<int>(message[i++])];
            uint32_t c = message[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<int>(message[i++])];
            uint32_t d = message[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<int>(message[i++])];

            uint32_t triple = (a << 3 * 6) + (b << 2 * 6) + (c << 1 * 6) + (d << 0 * 6);

            if (j < out_len) out[j++] = (triple >> 2 * 8) & 0xFF;
            if (j < out_len) out[j++] = (triple >> 1 * 8) & 0xFF;
            if (j < out_len) out[j++] = (triple >> 0 * 8) & 0xFF;
        }

        return out;
    }


    /*
    Messaging Functions
    */

    static std::string send_message(BigInt& key, unsigned long& size, std::string message) {
        if (message == "") {
            throw std::runtime_error("Message empty!");
        }
        size = 0;
        std::string encrypted = Utility::encrypt(message, size, key.to_string());
        spdlog::debug("Encrypted message: {}", encrypted);

        return Utility::encode_base64(encrypted);
    }


    static std::string receive_message(BigInt& key, unsigned long size, std::string message) {
        if (message == "") {
            throw std::runtime_error("Message empty!");
        }

        std::string encrypted = Utility::decode_base64(message);
        spdlog::debug("Encrypted message: {}", encrypted);
        return Utility::decrypt(encrypted, size, key.to_string());
    }
};