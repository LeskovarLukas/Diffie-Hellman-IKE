#pragma once

#include <string>
#include <asio.hpp>

class Pipe {
private:
    asio::ip::tcp::iostream stream;

public:
    Pipe(const std::string& host="localhost", const std::string& port="4433"): stream{host, port} {}

    Pipe(asio::ip::tcp::socket socket): stream{std::move(socket)} {}

    ~Pipe() {
        this->stream.close();
    }

    Pipe& operator<<(const std::string& message) {
        if (stream) {
            stream << message << std::endl;
        } else {
            throw std::runtime_error("Pipe: stream is not connected");
        }
        return *this;
    }

    Pipe& operator>>(std::string& message) {
        if (stream) {
            getline(stream, message);
        } else {
            throw std::runtime_error("Pipe: stream is not connected");
        }
        return *this;
    }
};