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
};