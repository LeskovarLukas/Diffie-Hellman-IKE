#pragma once 

// I hate to do this, but I dont know any better
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#include <plusaes/plusaes.hpp>
#pragma GCC diagnostic pop

#include <picosha2.h>


class Crypto_Utility
{
private:
    unsigned char iv[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
    std::vector<unsigned char> key = std::vector<unsigned char>(32);
    unsigned long size;

public:
    Crypto_Utility(const std::string& key_string) {
        picosha2::hash256(key_string.begin(), key_string.end(), this->key);
    }

    std::string encrypt(const std::string& message) {
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

    std::string decrypt(const std::string& message) {
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

    unsigned long get_size() const {
        return size;
    }

    void set_size(unsigned long size) {
        this->size = size;
    }

    static BigInt generate_random_number(BigInt min, BigInt max) {
        int randomData = open("/dev/urandom", O_RDONLY);
        return (randomData % (max - min + 1)) + min;
    }
};

