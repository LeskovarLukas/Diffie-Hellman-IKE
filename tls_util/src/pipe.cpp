#include <spdlog/spdlog.h>

#include "pipe.h"


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
    asio::read(*socket, asio::buffer(&message_size, sizeof(message_size)));

    asio::streambuf buffer;
    asio::streambuf::mutable_buffers_type bufs = buffer.prepare(message_size);
    buffer.commit(asio::read(*socket, bufs));

    std::istream is(&buffer);
    message.ParseFromIstream(&is);


    spdlog::debug("Pipe - Received message");
}
