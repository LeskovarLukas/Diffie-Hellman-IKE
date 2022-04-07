#include "session.h"
#include "tls_observer.h"


class Ping_Agent : public TLS_Observer {
private:
    std::shared_ptr<Session> session;
    unsigned int timeout;
    std::thread ping_thread;
    bool received_response;


public:
    Ping_Agent(std::shared_ptr<Session> session, unsigned int timeout = 0);
    ~Ping_Agent();

    void run();

    void notify(tls::Message_Wrapper message, unsigned int session_id);
};