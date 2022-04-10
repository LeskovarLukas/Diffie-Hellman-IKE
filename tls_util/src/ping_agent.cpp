/*
author: Leskovar Lukas
matnr: i17057
file: ping_agent.cpp
desc: This module provides a pipe for communication over TCP.
date: 2022-04-06
class: 5b
catnr: 10
*/

#include "ping_agent.h"

#include "messagebuilder.h"

#include <spdlog/spdlog.h>


Ping_Agent::Ping_Agent(std::shared_ptr<Session> session, unsigned int timeout)
    : session(session), timeout(timeout)
{
}

Ping_Agent::~Ping_Agent()
{
    ping_thread.join();
}


void Ping_Agent::run()
{
    ping_thread = std::thread([this] {
        try {
            while (session && timeout > 0) {
                session->send(Messagebuilder::build_ping_message());
                std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
                if (!received_response) {
                    spdlog::debug("Ping agent - No response received");
                    session->close();
                    return;
                }
                received_response = false;
            }
        } catch (std::exception& e) {
            spdlog::debug("Ping agent - Connection closed");
        }
    });
    ping_thread.detach();
}


void Ping_Agent::notify(tls::Message_Wrapper message, unsigned int session_id)
{
    if (message.type() == tls::Message_Type::PING) {
        session->send(Messagebuilder::build_ping_response_message());
    }
    if (message.type() == tls::Message_Type::PING_RESPONSE) {
        received_response = true;
        spdlog::debug("Ping agent {} - Received response", session_id);
    }
}
