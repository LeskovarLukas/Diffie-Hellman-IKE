#include "pipe.h"

#include <spdlog/spdlog.h>


Pipe::Pipe(asio::ip::tcp::socket socket) {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    spdlog::debug("Pipe - Creating pipe");
    this->socket = std::make_shared<asio::ip::tcp::socket>(std::move(socket));
}


Pipe::~Pipe() {
    google::protobuf::ShutdownProtobufLibrary();
    spdlog::debug("Pipe - Destroying pipe");
    socket->close();
}


void Pipe::send(google::protobuf::Message& message) {
    u_int64_t message_size{message.ByteSizeLong()};
    asio::write(*socket, asio::buffer(&message_size, sizeof(message_size)));

    asio::streambuf buffer;
    std::ostream os(&buffer);
    message.SerializeToOstream(&os);
    asio::write(*socket, buffer);

    spdlog::debug("Pipe - Sent message");
}


void Pipe::receive(google::protobuf::Message& message) {
    u_int64_t message_size;
    asio::error_code error;
    asio::read(*socket, asio::buffer(&message_size, sizeof(message_size)), error);

    if (error == asio::error::eof) {
        spdlog::debug("Pipe - Socket unexpectedly closed");
        throw std::runtime_error("Pipe closed");
    }

    asio::streambuf buffer;
    asio::streambuf::mutable_buffers_type bufs = buffer.prepare(message_size);
    buffer.commit(asio::read(*socket, bufs));

    std::istream is(&buffer);
    message.ParseFromIstream(&is);


    spdlog::debug("Pipe - Received message");
}


void Pipe::close() {
    spdlog::debug("Pipe - Closing pipe");
    socket->close();
}
