#pragma once

#include <spdlog/spdlog.h>

#include "pipe.h"
#include "utility.h"


enum State {
    UNSECURED,
    ESTABLISHING,
    SECURED
};


class TLS_Util {
private:
    Pipe& pipe;
    State currentState{State::UNSECURED};

    // DHKE Parameters
    int primeGroup = 0;
    BigInt G;
    BigInt P;
    BigInt s;
    BigInt S;
    BigInt c;
    BigInt C; 
    BigInt& key;

    // Handshake protocol
    std::string localProtocol;
    std::string partnerProtocol;
    bool partnerEncrypted = false;  //for change cipher spec

    bool check_protocols() {
        if (localProtocol != partnerProtocol) {
            pipe << "ABORT";
            return false;
        }
        return true;
    }

public:
    TLS_Util(Pipe& pipe, BigInt& key): pipe(pipe), key(key) {}

    void initiate_handshake() {
        if (currentState == SECURED) {
            spdlog::warn("TLS_Util::initiate_handshake() called when TLS connection is already established");
        }
        spdlog::info("Initiating key exchange");
        pipe << "TYPE_CLIENTHELLO";
        currentState = ESTABLISHING;
    }

    void handle_message(std::vector<std::string>& message_parts) {        
        if (currentState == State::UNSECURED || currentState == State::ESTABLISHING) {
            std::string messageType = message_parts[0];

            if (messageType == "CLIENTHELLO") {
                spdlog::info("Received client hello");

                // Send Client what prime group to use
                pipe << "TYPE_SERVERHELLO|PRIMEGROUP_" + std::to_string(primeGroup);

                // Calculate server public and private key
                Utility::read_primes_json("../modp_primes.json", primeGroup, G, P);

                s = Utility::generate_random_number(1, P);
                S = pow(G, s.to_int()) % P;

                // Send public key to client
                pipe << "TYPE_CERTIFICATE|S_" + S.to_string();

                // Server Done
                pipe << "TYPE_SERVERHELLODONE";
                currentState = State::ESTABLISHING;

            } else if (messageType == "SERVERHELLO") {
                // Calculate client public and private key
                primeGroup = std::stoi(message_parts[1]);
                Utility::read_primes_json("../modp_primes.json", primeGroup, G, P);

                c = Utility::generate_random_number(1, P);
                C = pow(G, c.to_int()) % P;

            } else if (messageType == "CERTIFICATE") {
                // Receive Server public key
                S = BigInt(message_parts[1]);

                // calculate shared secret key
                key = pow(S, c.to_int()) % P;
                spdlog::debug("Master Key: {}", key.to_string());

            } else if (messageType == "SERVERHELLODONE") {
                // Send client public key
                pipe << "TYPE_CLIENTKEYEXCHANGE|C_" + C.to_string();


                // Start encrypted communication
                pipe << "TYPE_CHANGECIPHERSPEC";

                // Create client protocol
                picosha2::hash256_hex_string(
                    "PRIMEGROUP_0|S_" + S.to_string() + "|C_" + C.to_string()
                    , localProtocol
                );
                localProtocol.resize(66);

                // Client finished
                pipe << "TYPE_FINISHED|"  + Utility::send_message(key, localProtocol) + "|PARTY_CLIENT";
            } else if (messageType == "CLIENTKEYEXCHANGE") {
                // Receive client public key 
                C = BigInt(message_parts[1]);

                // calculate shared secret key
                key = pow(C, s.to_int()) % P;
                spdlog::debug("Master Key: {}", key.to_string());

            } else if (messageType == "CHANGECIPHERSPEC") {
                partnerEncrypted = true;

            } else if (messageType == "FINISHED") {
                if (!partnerEncrypted) {
                    pipe << "TYPE_ABORT";
                    throw std::runtime_error("TLS_Util::handle_message() - Partner did not send ChangeCipherSpec");
                }

                // Receive partner protocol (client and server)
                partnerProtocol = Utility::receive_message(key, std::stoul(message_parts[1]), message_parts[2]);
                partnerProtocol.resize(66);

                if (message_parts[3] == "CLIENT") {         // Server receives client finished
                    // Start encrypted communication
                    pipe << "TYPE_CHANGECIPHERSPEC";

                    // Create server protocol
                    picosha2::hash256_hex_string(
                        "PRIMEGROUP_0|S_" + S.to_string() + "|C_" + C.to_string()
                        , localProtocol
                    );
                    localProtocol.resize(66);

                    if (check_protocols()) {
                        pipe << "TYPE_FINISHED|" + Utility::send_message(key, localProtocol) + "|PARTY_SERVER";
                        currentState = State::SECURED;
                        spdlog::info("TLS_Util::handle_message() - TLS connection established");
                    } else {
                        currentState = State::UNSECURED;
                        throw new std::runtime_error("TLS_Util::handle_message() - Protocols do not match");
                    }
                    
                } else if (message_parts[3] == "SERVER") {      // Client receives server finished
                    if (check_protocols()) {
                        currentState = State::SECURED;
                        spdlog::info("TLS_Util::handle_message() - TLS connection established");
                    } else {
                        currentState = State::UNSECURED;
                        throw new std::runtime_error("TLS_Util::handle_message() - Protocols do not match");
                    }
                }
            } else if (messageType == "ABORT") {
                currentState = State::UNSECURED;
                throw new std::runtime_error("TLS_Util::handle_message() - TLS connection aborted");
            } else {
                spdlog::error("Unknown message type: {}", messageType);
            }
        }
    }




    bool is_secure() const {
        return currentState == SECURED;
    }

    bool is_establishing() const {
        return currentState == ESTABLISHING;
    }

    void reconnect() {
        currentState = UNSECURED;
    }
};