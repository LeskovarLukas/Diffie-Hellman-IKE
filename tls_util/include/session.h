/*
author: Leskovar Lukas
matnr: i17057
file: session.h
desc: This module provides a session asynchronously listening for incoming messages 
        and sending messages.
date: 2022-04-06
class: 5b
catnr: 10
*/


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

    operator bool() {
        return pipe.is_open();
    }

    void start();

    void close();

    void send(tls::Message_Wrapper message);


    void subscribe(TLS_Observer_ptr observer);

    void unsubscribe(TLS_Observer_ptr observer);


    unsigned int get_session_id() const;

    bool is_open();


    void set_delay(unsigned int delay);
};