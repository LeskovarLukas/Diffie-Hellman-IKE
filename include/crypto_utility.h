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
    static const unsigned char iv[16];

public:
    static std::string encrypt(const std::string& message, const std::string& key, unsigned long& encrypted_size) {
        std::vector<unsigned char> key_bytes(32);
        picosha2::hash256(key.begin(), key.end(), key_bytes);

        encrypted_size = plusaes::get_padded_encrypted_size(message.size());
        std::vector<unsigned char> encrypted(encrypted_size);
        plusaes::encrypt_cbc(
            (unsigned char*)message.data(), message.size(), 
            &key_bytes[0], key_bytes.size(), 
            &iv, 
            &encrypted[0], encrypted.size(), 
            true
        );

        return std::string(encrypted.begin(), encrypted.end());    
    }

    static std::string decrypt(const std::string& message, const std::string& key, unsigned long& encrypted_size) {
        std::vector<unsigned char> key_bytes(32);
        picosha2::hash256(key.begin(), key.end(), key_bytes);

        unsigned long padded_size = 0;
        std::vector<unsigned char> encrypted(message.begin(), message.end());
        std::vector<unsigned char> decrypted(encrypted_size);
        plusaes::decrypt_cbc(
            &encrypted[0], encrypted.size(), 
            &key_bytes[0], key_bytes.size(), 
            &iv, 
            &decrypted[0], decrypted.size(), 
            &padded_size
        );

        return std::string(decrypted.begin(), decrypted.end());    
    }
};

const unsigned char Crypto_Utility::iv[16] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 
    0x08, 0x09,0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
};