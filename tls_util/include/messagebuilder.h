#pragma once

#include "Message.pb.h"


class Messagebuilder {
public:
    static tls::Message_Wrapper build_application_message(int size, std::string data) {
        tls::Message_Wrapper message;
        message.set_type(tls::Message_Type::DATA);
        message.mutable_application_data()->set_size(size);
        message.mutable_application_data()->set_data(data);

        return message;
    }

    static tls::Message_Wrapper build_close_message() {
        tls::Message_Wrapper message;
        message.set_type(tls::Message_Type::CLOSE);

        return message;
    }

    static tls::Message_Wrapper build_client_hello_message() {
        tls::Message_Wrapper message;
        message.set_type(tls::Message_Type::CLIENT_HELLO);

        return message;
    }

    static tls::Message_Wrapper build_server_hello_message(int prime_goup) {
        tls::Message_Wrapper message;
        message.set_type(tls::Message_Type::SERVER_HELLO);
        message.mutable_server_hello()->set_prime_group(prime_goup);

        return message;
    }

    static tls::Message_Wrapper build_certificate_message(std::string public_key) {
        tls::Message_Wrapper message;
        message.set_type(tls::Message_Type::CERTIFICATE);
        message.mutable_certificate()->set_public_key(public_key);

        return message;
    }

    static tls::Message_Wrapper build_server_hello_done_message() {
        tls::Message_Wrapper message;
        message.set_type(tls::Message_Type::SERVER_HELLO_DONE);

        return message;
    }

    static tls::Message_Wrapper build_client_key_exchange_message(std::string public_key) {
        tls::Message_Wrapper message;
        message.set_type(tls::Message_Type::CLIENT_KEY_EXCHANGE);
        message.mutable_client_key_exchange()->set_public_key(public_key);

        return message;
    }

    static tls::Message_Wrapper build_change_cipher_spec_message() {
        tls::Message_Wrapper message;
        message.set_type(tls::Message_Type::CHANGE_CIPHER_SPEC);

        return message;
    }

    static tls::Message_Wrapper build_finished_message(tls::FinishedType party, unsigned int size, std::string protocol) {
        tls::Message_Wrapper message;
        message.set_type(tls::Message_Type::FINISHED);
        message.mutable_finished()->set_party(party);
        message.mutable_finished()->set_size(size);
        message.mutable_finished()->set_protocol(protocol);

        return message;
    }

    static tls::Message_Wrapper build_abort_message() {
        tls::Message_Wrapper message;
        message.set_type(tls::Message_Type::ABORT);

        return message;
    }
};