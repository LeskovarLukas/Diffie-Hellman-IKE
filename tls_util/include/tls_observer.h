/*
author: Leskovar Lukas
matnr: i17057
file: tls_observer.h
desc: This class is the base class for handshake agent, client and server.
        It notifies the observers about the messages.
date: 2022-04-06
class: 5b
catnr: 10
*/


#pragma once

#include "Message.pb.h"


class TLS_Observer {
public:
    virtual ~TLS_Observer() {}

    virtual void notify(tls::Message_Wrapper message, unsigned int session_id) = 0;
};

typedef std::shared_ptr<TLS_Observer> TLS_Observer_ptr;
