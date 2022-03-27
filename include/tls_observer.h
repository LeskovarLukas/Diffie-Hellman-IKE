#pragma once

#include "Message.pb.h"

class TLS_Observer {
public:
    virtual ~TLS_Observer() {}

    virtual void notify(tls::MessageWrapper message, unsigned int session_id) = 0;
};

typedef std::shared_ptr<TLS_Observer> TLS_Observer_ptr;
