#pragma once

#include <string>
#include <asio.hpp>

class Pipe {
private:
    asio::ip::tcp::iostream stream;
    std::chrono::milliseconds delay;

public:
    Pipe(const std::string& host="localhost", const std::string& port="4433"): stream{host, port} {
    }

    Pipe(asio::ip::tcp::socket socket): stream{std::move(socket)} {}

    ~Pipe() {
        this->stream.close();
    }

    explicit operator bool() {
        return this->stream.good();
    }

    Pipe& operator<<(const std::string& message) {
        if (stream) {
            std::this_thread::sleep_for(delay);
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

    void setDelay(std::chrono::milliseconds delay) {
        this->delay = delay;
    }
};