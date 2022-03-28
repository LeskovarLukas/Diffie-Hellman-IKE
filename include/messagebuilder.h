#pragma once

#include "Message.pb.h"


class Messagebuilder {
public:
    static tls::MessageWrapper build_application_message(int size, std::string data) {
        tls::MessageWrapper message;
        message.set_type(tls::MessageType::DATA);
        message.mutable_application_data()->set_size(size);
        message.mutable_application_data()->set_data(data);

        return message;
    }

    static tls::MessageWrapper build_close_message() {
        tls::MessageWrapper message;
        message.set_type(tls::MessageType::CLOSE);

        return message;
    }

    static tls::MessageWrapper build_client_hello_message() {
        tls::MessageWrapper message;
        message.set_type(tls::MessageType::CLIENT_HELLO);

        return message;
    }

    static tls::MessageWrapper build_server_hello_message(int prime_goup) {
        tls::MessageWrapper message;
        message.set_type(tls::MessageType::SERVER_HELLO);
        message.mutable_server_hello()->set_prime_group(prime_goup);

        return message;
    }

    static tls::MessageWrapper build_certificate_message(std::string public_key) {
        tls::MessageWrapper message;
        message.set_type(tls::MessageType::CERTIFICATE);
        message.mutable_certificate()->set_public_key(public_key);

        return message;
    }

    static tls::MessageWrapper build_server_hello_done_message() {
        tls::MessageWrapper message;
        message.set_type(tls::MessageType::SERVER_HELLO_DONE);

        return message;
    }

    static tls::MessageWrapper build_client_key_exchange_message(std::string public_key) {
        tls::MessageWrapper message;
        message.set_type(tls::MessageType::CLIENT_KEY_EXCHANGE);
        message.mutable_client_key_exchange()->set_public_key(public_key);

        return message;
    }

    static tls::MessageWrapper build_change_cipher_spec_message() {
        tls::MessageWrapper message;
        message.set_type(tls::MessageType::CHANGE_CIPHER_SPEC);

        return message;
    }

    static tls::MessageWrapper build_finished_message(tls::FinishedType party, unsigned int size, std::string protocol) {
        tls::MessageWrapper message;
        message.set_type(tls::MessageType::FINISHED);
        message.mutable_finished()->set_party(party);
        message.mutable_finished()->set_size(size);
        message.mutable_finished()->set_protocol(protocol);

        return message;
    }

    static tls::MessageWrapper build_abort_message() {
        tls::MessageWrapper message;
        message.set_type(tls::MessageType::ABORT);

        return message;
    }
};