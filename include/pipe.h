#pragma once

#include <string>
#include <asio.hpp>
#include <spdlog/spdlog.h>

class Pipe {
private:
    std::string host;
    std::string port;
    asio::ip::tcp::iostream stream;
    std::chrono::milliseconds delay = std::chrono::milliseconds(0);

public:
    Pipe(const std::string& host="localhost", const std::string& port="4433"): stream{host, port} {
        if (!stream) {
            throw std::runtime_error("Pipe: Could not connect");
        }
        this->host = host;
        this->port = port;
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
            spdlog::debug("Pipe Sending: {}", message);
            stream << message << std::endl;
        } else {
            throw std::runtime_error("Pipe: stream is not connected");
        }
        return *this;
    }

    Pipe& operator>>(std::string& message) {
        if (stream) {
            getline(stream, message);
            spdlog::debug("Pipe Received: {}", message);
        } else {
            throw std::runtime_error("Pipe: stream is not connected");
        }
        return *this;
    }

    void try_reconnect() {
        stream.close();
        stream.clear();
        stream = asio::ip::tcp::iostream{host, port};
        if (!stream) {
            throw std::runtime_error("Pipe: Could not connect");
        }
    }

    void set_delay(std::chrono::milliseconds delay) {
        this->delay = delay;
    }
};