#pragma once


#include "session.hpp"


class TLS_Server {
private: 
    asio::io_context& io_context;
    asio::ip::tcp::acceptor acceptor;
    std::vector<std::shared_ptr<Session>> sessions;


    void start_accept();


public:
    TLS_Server(asio::io_context& io_context);
};