#pragma once

#include "session.h"
#include "messagebuilder.h"
#include "utility.h"


enum State {
    UNSECURED,
    ESTABLISHING,
    SECURED
};


class TLS_Handshake_Agent: public TLS_Observer, public std::enable_shared_from_this<TLS_Observer> {
private:
    std::shared_ptr<Session> session;
    State currentState{State::UNSECURED};

    // DHKE Parameters
    int primeGroup = 0;
    BigInt G = 0;
    BigInt P = 0;
    BigInt s = 0;
    BigInt S = 0;
    BigInt c = 0;
    BigInt C = 0; 
    BigInt key = 0;

    // Handshake protocol
    std::string localProtocol;
    std::string partnerProtocol;
    bool partnerEncrypted = false;  //for change cipher spec


    bool check_protocols() {
        if (localProtocol != partnerProtocol) {
            spdlog::error("TLS_Handshake_Agent - Protocols do not match");
            session->send(Messagebuilder::build_abort_message());
            return false;
        }
        return true;
    }


    // Handshake handles

    void handle_message(tls::MessageWrapper message) {    
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


    void receive_client_hello() {
        spdlog::info("Received client hello");

        // Send Client what prime group to use
        session->send(Messagebuilder::build_server_hello_message(primeGroup));

        // Calculate server public and private key
        Utility::read_primes_json("../modp_primes.json", primeGroup, G, P);

        s = Utility::generate_random_number(1, P);
        S = pow(G, s.to_int()) % P;

        // Send public key to client
        session->send(Messagebuilder::build_certificate_message(S.to_string()));

        // Server Done
        session->send(Messagebuilder::build_server_hello_done_message());
        currentState = State::ESTABLISHING;
    }


    void receive_server_hello(tls::MessageWrapper message) {
        // Calculate client public and private key
        primeGroup = message.mutable_server_hello()->prime_group();
        Utility::read_primes_json("../modp_primes.json", primeGroup, G, P);

        c = Utility::generate_random_number(1, P);
        C = pow(G, c.to_int()) % P;
        spdlog::debug("TLS_Handshake_Agent::handle_message() - Calculated client public key: {}", C.to_string());
    }


    void receive_certificate(tls::MessageWrapper message) {
        // Receive Server public key
        S = BigInt(message.mutable_certificate()->public_key());
        spdlog::debug("TLS_Handshake_Agent::handle_message() - Received server public key: {}", S.to_string());

        // calculate shared secret key
        key = pow(S, c.to_int()) % P;
        spdlog::debug("Master Key: {}", key.to_string());
    }


    void receive_server_hello_done() {
        // Send client public key
        session->send(Messagebuilder::build_client_key_exchange_message(C.to_string()));


        // Start encrypted communication
        session->send(Messagebuilder::build_change_cipher_spec_message());

        // Create client protocol
        picosha2::hash256_hex_string(
            "PRIMEGROUP_0|S_" + S.to_string() + "|C_" + C.to_string()
            , localProtocol
        );
        localProtocol.resize(66);

        // Client finished
        unsigned long size;
        std::string encrypted_protocol = Utility::send_message(key, size, localProtocol);
        session->send(Messagebuilder::build_finished_message(tls::FinishedType::CLIENT_FINISHED, size, encrypted_protocol));
    }


    void receive_client_key_exchange(tls::MessageWrapper message) {
        // Receive client public key
        C = BigInt(message.mutable_client_key_exchange()->public_key());
        spdlog::debug("TLS_Handshake_Agent::handle_message() - Received client public key: {}", C.to_string());

        // Calculate shared secret key
        key = pow(C, s.to_int()) % P;
        spdlog::debug("Master Key: {}", key.to_string());
    }


    void receive_finished(tls::MessageWrapper message) {
        // Receive partner protocol (client and server)
        partnerProtocol = Utility::receive_message(key, message.mutable_finished()->size(), message.mutable_finished()->protocol());
        partnerProtocol.resize(66);

        if (message.mutable_finished()->party() == tls::FinishedType::CLIENT_FINISHED) {         // Server receives client finished
            // Start encrypted communication
            session->send(Messagebuilder::build_change_cipher_spec_message());

            // Create server protocol
            picosha2::hash256_hex_string(
                "PRIMEGROUP_0|S_" + S.to_string() + "|C_" + C.to_string()
                , localProtocol
            );
            localProtocol.resize(66);


            if (check_protocols()) {
                unsigned long size;
                std::string encrypted_protocol = Utility::send_message(key, size, localProtocol);
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

public:
    TLS_Handshake_Agent(std::shared_ptr<Session> session) : session(session) {
        currentState = State::UNSECURED;
    }


    void notify(tls::MessageWrapper message, unsigned int session_id) {
        spdlog::debug("TLS_Handshake_Agent::notify() - Received message from Session {}", session_id);
        if (currentState == State::UNSECURED || currentState == State::ESTABLISHING) {
            handle_message(message);
        }
    }


    void initiate_handshake() {
        if (currentState == SECURED) {
            spdlog::warn("TLS_Handshake_Agent::initiate_handshake() called when TLS connection is already established");
        }
        spdlog::info("Initiating key exchange");
        session->send(Messagebuilder::build_client_hello_message());
        currentState = ESTABLISHING;
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

    BigInt get_key() const {
        return key;
    }                                                                                                      
};