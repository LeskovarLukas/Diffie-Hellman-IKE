#pragma once

#include "pipe.h"
#include "tls_observer.h"


class Session {
private:
    Pipe pipe;
    unsigned int session_id;
    std::vector<std::shared_ptr<TLS_Observer>> observers;

    std::thread listen_thread;


    void listen_for_messages();

    void notify(tls::Message_Wrapper message);

public:
    Session(asio::ip::tcp::socket socket, unsigned int session_id);

    ~Session();

    void start();

    void send(tls::Message_Wrapper message);


    void subscribe(TLS_Observer_ptr observer);

    void unsubscribe(TLS_Observer_ptr observer);


    unsigned int get_session_id() const;
};