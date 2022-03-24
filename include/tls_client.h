#pragma once

#include "session.hpp"


class TLS_Client {
private:
    asio::io_context& io_context;
    asio::ip::tcp::resolver resolver;
    asio::ip::tcp::socket socket;
    asio::ip::tcp::resolver::results_type endpoints;
    std::shared_ptr<Session> session;

public: 
    TLS_Client(asio::io_context& io_context, std::string host, std::string port);

    void run();
};