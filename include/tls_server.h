#pragma once

#include <asio.hpp>
#include <spdlog/spdlog.h>


class TLS_Server {
private: 
    asio::io_context& io_context;
    asio::ip::tcp::acceptor acceptor;


    void start_accept();


public:
    TLS_Server(asio::io_context& io_context);
};