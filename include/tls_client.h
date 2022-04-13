/*
author: Leskovar Lukas
matnr: i17057
file: tls_client.h
desc: This modules creates a client and connects to a server.
        Once the connection is established it sends a message to the server.
date: 2022-04-06
class: 5b
catnr: 10
*/


#pragma once

#include "session.h"
#include "tls_handshake_agent.h"
#include "ping_agent.h"


class TLS_Client: public TLS_Observer, public std::enable_shared_from_this<TLS_Observer> {
private:
    asio::io_context& io_context;
    asio::ip::tcp::resolver resolver;
    asio::ip::tcp::socket socket;
    asio::ip::tcp::resolver::results_type endpoints;
    std::shared_ptr<Session> session;
    std::shared_ptr<TLS_Handshake_Agent> handshake_agent;
    std::shared_ptr<Ping_Agent> ping_agent;


public: 
    TLS_Client(asio::io_context& io_context, std::string host, std::string port);

    ~TLS_Client();


    void run();


    void notify(tls::Message_Wrapper message, unsigned int session_id);


    void set_delay(unsigned int delay);
};