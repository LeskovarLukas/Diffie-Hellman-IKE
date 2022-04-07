/*
author: Leskovar Lukas
matnr: i17057
file: tls_server.h
desc: This module runs a server asynchronous listening for incoming connections.
        Once a connection is established it replies to messages
date: 2022-04-06
class: 5b
catnr: 10
*/


#pragma once

#include "session.h"
#include "tls_handshake_agent.h"
#include "ping_agent.h"


class TLS_Server: public TLS_Observer, public std::enable_shared_from_this<TLS_Observer> {
private: 
    asio::io_context& io_context;
    asio::ip::tcp::acceptor acceptor;
    std::vector<std::shared_ptr<Session>> sessions;
    std::vector<std::shared_ptr<TLS_Handshake_Agent>> handshake_agents;
    std::vector<std::shared_ptr<Ping_Agent>> ping_agents;
    unsigned int timeout;

    void start_accept();

public:
    TLS_Server(asio::io_context& io_context, int port, unsigned int timeout);


    void notify(tls::Message_Wrapper message, unsigned int session_id);


    void send(unsigned int session_id, std::string input);
};