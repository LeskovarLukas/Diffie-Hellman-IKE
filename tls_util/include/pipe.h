/*
author: Leskovar Lukas
matnr: i17057
file: pipe.h
desc: This module provides a pipe for communication over TCP.
date: 2022-04-06
class: 5b
catnr: 10
*/


#pragma once

#include "Message.pb.h"

#include <asio.hpp>
#include <random>


class Pipe {
private:
    std::shared_ptr<asio::ip::tcp::socket> socket;
    unsigned int delay = 0;  // in milliseconds
    std::random_device rd;
    std::mt19937 gen{rd()};

    void wait_random();

public:
    Pipe(asio::ip::tcp::socket socket);

    ~Pipe();


    operator bool() {
        return socket->is_open();
    }

    void send(google::protobuf::Message& message);

    void receive(google::protobuf::Message& message);

    void close();


    void set_delay(unsigned int delay);
};