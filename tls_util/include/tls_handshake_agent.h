/*
author: Leskovar Lukas
matnr: i17057
file: tls_handshake_agent.h
desc: This module is responsible for handling handshake messages.
        It also contains utility for encryption and decryption.
date: 2022-04-06
class: 5b
catnr: 10
*/


#pragma once

#include "session.h"

#include "messagebuilder.h"

class BigInt;


enum State {
    UNSECURED,
    ESTABLISHING,
    SECURED
};


class TLS_Handshake_Agent: public TLS_Observer, public std::enable_shared_from_this<TLS_Observer> {
private:
    std::shared_ptr<Session> session;
    State current_state{State::UNSECURED};

    // DHKE Parameters
    int prime_Group = 0;
    std::shared_ptr<BigInt> G;
    std::shared_ptr<BigInt> P;
    std::shared_ptr<BigInt> s;
    std::shared_ptr<BigInt> S;
    std::shared_ptr<BigInt> c;
    std::shared_ptr<BigInt> C;
    std::shared_ptr<BigInt> key;

    // Handshake protocol
    std::string local_protocol;
    std::string partner_protocol;
    bool partner_encrypted = false;  //for change cipher spec


    // encryption
    static const unsigned char iv[16];


    bool check_protocols();


    // Handshake handles

    void handle_message(tls::Message_Wrapper message);


    void receive_client_hello();


    void receive_server_hello(tls::Message_Wrapper message);


    void receive_certificate(tls::Message_Wrapper message);


    void receive_server_hello_done();


    void receive_client_key_exchange(tls::Message_Wrapper message);


    void receive_finished(tls::Message_Wrapper message);

public:
    TLS_Handshake_Agent(std::shared_ptr<Session> session);


    void notify(tls::Message_Wrapper message, unsigned int session_id);

    void initiate_handshake();


    bool is_secure() const;

    bool is_establishing() const;

    void reconnect();    

    std::string get_key() const;           


    /*
    Static Utility
    */

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