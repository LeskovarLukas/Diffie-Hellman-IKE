#pragma once


#include "session.h"
#include "tls_handshake_agent.h"


class TLS_Client: public TLS_Observer, public std::enable_shared_from_this<TLS_Observer> {
private:
    asio::io_context& io_context;
    asio::ip::tcp::resolver resolver;
    asio::ip::tcp::socket socket;
    asio::ip::tcp::resolver::results_type endpoints;
    std::shared_ptr<Session> session;
    std::shared_ptr<TLS_Handshake_Agent> handshake_agent;


public: 
    TLS_Client(asio::io_context& io_context, std::string host, std::string port);


    void run();


    void notify(tls::Message_Wrapper message, unsigned int session_id);
};