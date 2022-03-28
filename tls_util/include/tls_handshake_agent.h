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
    State currentState{State::UNSECURED};

    // DHKE Parameters
    int primeGroup = 0;
    std::shared_ptr<BigInt> G;
    std::shared_ptr<BigInt> P;
    std::shared_ptr<BigInt> s;
    std::shared_ptr<BigInt> S;
    std::shared_ptr<BigInt> c;
    std::shared_ptr<BigInt> C;
    std::shared_ptr<BigInt> key;

    // Handshake protocol
    std::string localProtocol;
    std::string partnerProtocol;
    bool partnerEncrypted = false;  //for change cipher spec


    bool check_protocols();


    // Handshake handles

    void handle_message(tls::MessageWrapper message);


    void receive_client_hello();


    void receive_server_hello(tls::MessageWrapper message);


    void receive_certificate(tls::MessageWrapper message);


    void receive_server_hello_done();


    void receive_client_key_exchange(tls::MessageWrapper message);


    void receive_finished(tls::MessageWrapper message);

public:
    TLS_Handshake_Agent(std::shared_ptr<Session> session);


    void notify(tls::MessageWrapper message, unsigned int session_id);

    void initiate_handshake();


    bool is_secure() const;

    bool is_establishing() const;

    void reconnect();    

    std::string get_key() const;                                                                                                   
};