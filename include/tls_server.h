#pragma once

#include <asio.hpp>
#include <spdlog/spdlog.h>

#include "session.hpp"


class TLS_Server {
private: 
    asio::io_context& io_context;
    asio::ip::tcp::acceptor acceptor;
    std::vector<std::shared_ptr<session>> sessions;


    void start_accept();


public:
    TLS_Server(asio::io_context& io_context);
};