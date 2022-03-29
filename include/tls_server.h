#pragma once

#include "session.h"
#include "tls_handshake_agent.h"


class TLS_Server: public TLS_Observer, public std::enable_shared_from_this<TLS_Observer> {
private: 
    asio::io_context& io_context;
    asio::ip::tcp::acceptor acceptor;
    std::vector<std::shared_ptr<Session>> sessions;
    std::vector<std::shared_ptr<TLS_Handshake_Agent>> handshake_agents;

    void start_accept();

public:
    TLS_Server(asio::io_context& io_context, int port);


    void notify(tls::MessageWrapper message, unsigned int session_id);


    void send(unsigned int session_id, std::string input);
};