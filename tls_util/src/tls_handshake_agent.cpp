#include <spdlog/spdlog.h>
#include <vector>
#include <fstream>
#include <json.hpp>

#include "tls_handshake_agent.h"
#include "PicoSHA2/picosha2.h"
#include "BigInt/BigInt.hpp"        

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "plusaes/plusaes.hpp"
#pragma GCC diagnostic pop



bool TLS_Handshake_Agent::check_protocols() {
    if (localProtocol != partnerProtocol) {
        spdlog::error("TLS_Handshake_Agent - Protocols do not match");
        session->send(Messagebuilder::build_abort_message());
        return false;
    }
    return true;
}


// Handshake handles

void TLS_Handshake_Agent::handle_message(tls::MessageWrapper message) {    
    tls::MessageType messageType = message.type();

    spdlog::debug("TLS_Handshake_Agent - Received message {}", messageType);

    if (messageType == tls::MessageType::CLIENT_HELLO) {
        receive_client_hello();

    } else if (messageType == tls::MessageType::SERVER_HELLO) {
        receive_server_hello(message);

    } else if (messageType == tls::MessageType::CERTIFICATE) {
        receive_certificate(message);

    } else if (messageType == tls::MessageType::SERVER_HELLO_DONE) {
        receive_server_hello_done();

    } else if (messageType == tls::MessageType::CLIENT_KEY_EXCHANGE) {
        receive_client_key_exchange(message);

    } else if (messageType == tls::MessageType::CHANGE_CIPHER_SPEC) {
        partnerEncrypted = true;

    } else if (messageType == tls::MessageType::FINISHED) {
        if (!partnerEncrypted) {
            session->send(Messagebuilder::build_abort_message());
            throw std::runtime_error("TLS_Handshake_Agent::handle_message() - Partner did not send ChangeCipherSpec");
        }
        receive_finished(message);
        
    } else if (messageType == tls::MessageType::ABORT) {
        currentState = State::UNSECURED;
        throw new std::runtime_error("TLS_Handshake_Agent::handle_message() - TLS connection aborted");
    } else {
        spdlog::error("Unknown message type: {}", messageType);
    }
}


void TLS_Handshake_Agent::receive_client_hello() {
    spdlog::info("Received client hello");

    // Send Client what prime group to use
    session->send(Messagebuilder::build_server_hello_message(primeGroup));

    // Calculate server public and private key
    G = std::make_shared<BigInt>(0);
    P = std::make_shared<BigInt>(0);
    TLS_Handshake_Agent::read_primes_json("../modp_primes.json", primeGroup, *G, *P);

    s = std::make_shared<BigInt>(TLS_Handshake_Agent::generate_random_number(1, *P));
    S = std::make_shared<BigInt>(pow(*G, s->to_int()) % *P);

    // Send public key to client
    session->send(Messagebuilder::build_certificate_message(S->to_string()));

    // Server Done
    session->send(Messagebuilder::build_server_hello_done_message());
    currentState = State::ESTABLISHING;
}


void TLS_Handshake_Agent::receive_server_hello(tls::MessageWrapper message) {
    // Calculate client public and private key
    primeGroup = message.mutable_server_hello()->prime_group();
    G = std::make_shared<BigInt>(0);
    P = std::make_shared<BigInt>(0);
    TLS_Handshake_Agent::read_primes_json("../modp_primes.json", primeGroup, *G, *P);

    c = std::make_shared<BigInt>(TLS_Handshake_Agent::generate_random_number(1, *P));
    C = std::make_shared<BigInt>(pow(*G, c->to_int()) % *P);
    spdlog::debug("TLS_Handshake_Agent::handle_message() - Calculated client public key: {}", C->to_string());
}


void TLS_Handshake_Agent::receive_certificate(tls::MessageWrapper message) {
    // Receive Server public key
    S = std::make_shared<BigInt>(BigInt(message.mutable_certificate()->public_key()));
    spdlog::debug("TLS_Handshake_Agent::handle_message() - Received server public key: {}", S->to_string());

    // calculate shared secret key
    key = std::make_shared<BigInt>(pow(*S, c->to_int()) % *P);
    spdlog::debug("Master Key: {}", key->to_string());
}


void TLS_Handshake_Agent::receive_server_hello_done() {
    // Send client public key
    session->send(Messagebuilder::build_client_key_exchange_message(C->to_string()));


    // Start encrypted communication
    session->send(Messagebuilder::build_change_cipher_spec_message());

    // Create client protocol
    picosha2::hash256_hex_string(
        "PRIMEGROUP_0|S_" + S->to_string() + "|C_" + C->to_string()
        , localProtocol
    );
    localProtocol.resize(66);

    // Client finished
    unsigned long size;
    std::string encrypted_protocol = TLS_Handshake_Agent::send_message(key->to_string(), size, localProtocol);
    session->send(Messagebuilder::build_finished_message(tls::FinishedType::CLIENT_FINISHED, size, encrypted_protocol));
}


void TLS_Handshake_Agent::receive_client_key_exchange(tls::MessageWrapper message) {
    // Receive client public key
    C = std::make_shared<BigInt>(BigInt(message.mutable_client_key_exchange()->public_key()));
    spdlog::debug("TLS_Handshake_Agent::handle_message() - Received client public key: {}", C->to_string());

    // Calculate shared secret key
    key = std::make_shared<BigInt>(pow(*C, s->to_int()) % *P);
    spdlog::debug("Master Key: {}", key->to_string());
}


void TLS_Handshake_Agent::receive_finished(tls::MessageWrapper message) {
    // Receive partner protocol (client and server)
    partnerProtocol = TLS_Handshake_Agent::receive_message(key->to_string(), message.mutable_finished()->size(), message.mutable_finished()->protocol());
    partnerProtocol.resize(66);

    if (message.mutable_finished()->party() == tls::FinishedType::CLIENT_FINISHED) {         // Server receives client finished
        // Start encrypted communication
        session->send(Messagebuilder::build_change_cipher_spec_message());

        // Create server protocol
        picosha2::hash256_hex_string(
            "PRIMEGROUP_0|S_" + S->to_string() + "|C_" + C->to_string()
            , localProtocol
        );
        localProtocol.resize(66);


        if (check_protocols()) {
            unsigned long size;
            std::string encrypted_protocol = TLS_Handshake_Agent::send_message(key->to_string(), size, localProtocol);
            session->send(Messagebuilder::build_finished_message(tls::FinishedType::SERVER_FINISHED, size, encrypted_protocol));
            spdlog::info("TLS_Handshake_Agent::handle_message() - TLS connection established");
            currentState = State::SECURED;
        } else {
            currentState = State::UNSECURED;
            throw new std::runtime_error("TLS_Handshake_Agent::handle_message() - Protocols do not match");
        }
        
    } else if (message.mutable_finished()->party() == tls::FinishedType::SERVER_FINISHED) {      // Client receives server finished
        if (check_protocols()) {
            spdlog::info("TLS_Handshake_Agent::handle_message() - TLS connection established");
            currentState = State::SECURED;
        } else {
            currentState = State::UNSECURED;
            throw new std::runtime_error("TLS_Handshake_Agent::handle_message() - Protocols do not match");
        }
    }
}


// Public functions

TLS_Handshake_Agent::TLS_Handshake_Agent(std::shared_ptr<Session> session) : session(session) {
    currentState = State::UNSECURED;
}


void TLS_Handshake_Agent::notify(tls::MessageWrapper message, unsigned int session_id) {
    spdlog::debug("TLS_Handshake_Agent::notify() - Received message from Session {}", session_id);
    if (currentState == State::UNSECURED || currentState == State::ESTABLISHING) {
        handle_message(message);
    }
}


void TLS_Handshake_Agent::initiate_handshake() {
    if (currentState == SECURED) {
        spdlog::warn("TLS_Handshake_Agent::initiate_handshake() called when TLS connection is already established");
    }
    spdlog::info("Initiating key exchange");
    session->send(Messagebuilder::build_client_hello_message());
    currentState = ESTABLISHING;
}


bool TLS_Handshake_Agent::is_secure() const {
    return currentState == SECURED;
}

bool TLS_Handshake_Agent::is_establishing() const {
    return currentState == ESTABLISHING;
}

void TLS_Handshake_Agent::reconnect() {
    currentState = UNSECURED;
}     

std::string TLS_Handshake_Agent::get_key() const {
    return key->to_string();
}


/*
Static Utility
*/

/*
General Utility Functions
*/

BigInt TLS_Handshake_Agent::generate_random_number(BigInt min, BigInt max) {
    int randomData = open("/dev/urandom", O_RDONLY);
    return (randomData % (max - min + 1)) + min;
}


void TLS_Handshake_Agent::read_primes_json(std::string filename, int id, BigInt& g, BigInt& p) {
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

const unsigned char TLS_Handshake_Agent::iv[16] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
};


std::string TLS_Handshake_Agent::encrypt(const std::string& message, unsigned long& size, const std::string& key_string) {
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


std::string TLS_Handshake_Agent::decrypt(const std::string& message, unsigned long& size, const std::string& key_string) {
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

std::string TLS_Handshake_Agent::encode_base64(const std::string& message) {
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

std::string TLS_Handshake_Agent::decode_base64(const std::string& message) {
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

std::string TLS_Handshake_Agent::send_message(std::string key, unsigned long& size, std::string message) {
    if (message == "") {
        throw std::runtime_error("Message empty!");
    }
    size = 0;
    std::string encrypted = TLS_Handshake_Agent::encrypt(message, size, key);
    spdlog::debug("Encrypted message: {}", encrypted);

    return TLS_Handshake_Agent::encode_base64(encrypted);
}


std::string TLS_Handshake_Agent::receive_message(std::string key, unsigned long size, std::string message) {
    if (message == "") {
        throw std::runtime_error("Message empty!");
    }

    std::string encrypted = TLS_Handshake_Agent::decode_base64(message);
    spdlog::debug("Encrypted message: {}", encrypted);
    return TLS_Handshake_Agent::decrypt(encrypted, size, key);
}