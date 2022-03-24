#pragma once

#include <asio.hpp>

class Pipe {
private:
    std::shared_ptr<asio::ip::tcp::socket> socket;

public:
    Pipe(asio::ip::tcp::socket socket) {
        spdlog::debug("Pipe - Creating pipe");
        this->socket = std::make_shared<asio::ip::tcp::socket>(std::move(socket));
    }

    ~Pipe() {
        spdlog::debug("Pipe - Destroying pipe");
        socket->close();
    }


    operator bool() {
        return socket->is_open();
    }


    void send(const std::string& message) {
        asio::write(*socket, asio::buffer(message));
    }

    void receive(std::string& message) {
        std::array<char, 128> buffer;
        asio::error_code error;
        size_t length = socket->read_some(asio::buffer(buffer), error);

        if (error == asio::error::eof) {
            spdlog::debug("Pipe - Connection closed");
            throw std::runtime_error("Connection closed");
        } else if (error) {
            spdlog::error("Pipe - Error: {}", error.message());
            throw std::runtime_error(error.message());
        } else {
            spdlog::debug("Pipe - Received {} bytes", length);
            message = std::string(buffer.data(), length);
        }
    }
};