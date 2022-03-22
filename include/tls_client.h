#pragma once

#include "tls_util.h"

class TLS_Client {
private:
    Pipe& pipe;
    BigInt key;
    TLS_Util tls_util;

public:
    TLS_Client(Pipe& pipe): pipe{pipe}, tls_util{pipe, key} {
        tls_util.initiate_handshake();
    }


    void run() {
        while (true) {
            try {
                if (tls_util.is_secure()) {
                    std::string input;
                    std::cout << "Enter message: ";
                    std::getline(std::cin, input);

                    if (input == "quit") {
                        pipe << "TYPE_CLOSE";
                        spdlog::info("Closing connection");
                        break;
                    }

                    pipe << "TYPE_DATA|" + Utility::send_message(key, input);
                } else if (tls_util.is_establishing()) {
                    std::string message;
                    pipe >> message;

                    if (message == "") {
                        spdlog::warn("Received empty message");
                        continue;
                    }
                    std::vector<std::string> message_parts;
                    Utility::split_message(message, message_parts);

                    tls_util.handle_message(message_parts);
                } else {
                    tls_util.initiate_handshake();
                }
            } catch (std::exception& e) {
                spdlog::warn(e.what());

                if (!pipe) {
                    spdlog::info("Trying to reconnect");

                    for (int i = 0; i < 10; i++) {
                        try {
                            pipe.try_reconnect();
                            tls_util.reconnect();
                            spdlog::info("Reconnected");
                            break;
                        } catch (std::exception& e) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                            if (i == 9) {
                                throw std::runtime_error("Could not reconnect");
                            }
                        }
                    }
                }
            }
        }
    }
};