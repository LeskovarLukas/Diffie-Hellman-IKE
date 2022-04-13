/*
author: Leskovar Lukas
matnr: i17057
file: pipe.cpp
desc: See pipe.h
date: 2022-04-06
class: 5b
catnr: 10
*/


#include "pipe.h"

#include <spdlog/spdlog.h>


void Pipe::wait_random() {
    std::uniform_int_distribution<> dis(0, delay);
    int wait = dis(gen);
    std::this_thread::sleep_for(std::chrono::milliseconds(wait));
}


void Pipe::send(google::protobuf::Message& message) {
    wait_random();
    std::lock_guard<std::mutex> lock(mtx);
    u_int64_t message_size{message.ByteSizeLong()};
    asio::write(*socket, asio::buffer(&message_size, sizeof(message_size)));

    asio::streambuf buffer;
    std::ostream os(&buffer);
    message.SerializeToOstream(&os);
    asio::write(*socket, buffer);

    spdlog::debug("Pipe - Sent message");
}


Pipe::Pipe(asio::ip::tcp::socket socket) {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    spdlog::debug("Pipe - Creating pipe");
    this->socket = std::make_shared<asio::ip::tcp::socket>(std::move(socket));
    open = true;
}


Pipe::~Pipe() {
    google::protobuf::ShutdownProtobufLibrary();
    spdlog::debug("Pipe - Destroying pipe");
    socket->close();
}


void Pipe::send_message(google::protobuf::Message& message) {
    auto send_future = std::async(&Pipe::send, this, std::ref(message));
    send_future.wait();
}


void Pipe::receive(google::protobuf::Message& message) {
    u_int64_t message_size;
    asio::error_code error;
    asio::read(*socket, asio::buffer(&message_size, sizeof(message_size)), error);

    if (error == asio::error::eof) {
        spdlog::debug("Pipe - Socket closed");
        throw std::runtime_error("Pipe closed");
    } else if (message_size == 0) {
        spdlog::debug("Pipe - Received empty message");
        throw std::runtime_error("Received empty message");
    } else {
        asio::streambuf buffer;
        asio::streambuf::mutable_buffers_type bufs = buffer.prepare(message_size);
        buffer.commit(asio::read(*socket, bufs));

        std::istream is(&buffer);
        message.ParseFromIstream(&is);


        spdlog::debug("Pipe - Received message");
    }
}


void Pipe::close() {
    std::lock_guard<std::mutex> lock(mtx);
    spdlog::debug("Pipe - Closing pipe");
    socket->close();
    open = false;
}


bool Pipe::is_open() {
    return open && socket->is_open();
}



void Pipe::set_delay(unsigned int delay) {
    this->delay = delay;
}