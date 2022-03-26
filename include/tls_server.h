#pragma once


#include "session.h"
#include "tls_observer.h"


class TLS_Server: public TLS_Observer, public std::enable_shared_from_this<TLS_Observer> {
private: 
    asio::io_context& io_context;
    asio::ip::tcp::acceptor acceptor;
    std::vector<std::shared_ptr<Session>> sessions;


    void start_accept();


public:
    TLS_Server(asio::io_context& io_context);

    void notify(tls::MessageWrapper message);
};