#pragma once

#include <asio.hpp>
#include "Message.pb.h"

class Pipe {
private:
    std::shared_ptr<asio::ip::tcp::socket> socket;

public:
    Pipe(asio::ip::tcp::socket socket);

    ~Pipe();


    operator bool() {
        return socket->is_open();
    }


    void send(google::protobuf::Message& message);

    void receive(google::protobuf::Message& message);
};