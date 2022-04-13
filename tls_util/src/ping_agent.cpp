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
    std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
    ping_thread = std::thread([this] {
        try {
            while (session && timeout > 0) {
                session->send(Messagebuilder::build_ping_message());
                spdlog::debug("Ping_Agent::run() - sent ping");
                std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
                if (!received_response) {
                    spdlog::debug("Ping_Agent::run() - No response received");
                    session->close();
                    return;
                }
                received_response = false;
            }
        } catch (std::exception& e) {
            spdlog::debug("Ping_Agent::run() - Connection closed");
        }
    });
    ping_thread.detach();
}


void Ping_Agent::notify(tls::Message_Wrapper message, unsigned int session_id)
{
    if (message.type() == tls::Message_Type::PING) {
        spdlog::debug("Ping_Agent::notify() - received ping");
        session->send(Messagebuilder::build_ping_response_message());
    }
    if (message.type() == tls::Message_Type::PING_RESPONSE) {
        received_response = true;
        spdlog::debug("Ping_Agent {}::notify() - Received response", session_id);
    }
}
