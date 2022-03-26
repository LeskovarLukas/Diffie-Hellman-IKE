#pragma once

#include <string>

class TLS_Observer {
public:
    virtual ~TLS_Observer() {}

    virtual void notify(tls::MessageWrapper message) = 0;
};

typedef std::shared_ptr<TLS_Observer> TLS_Observer_ptr;
