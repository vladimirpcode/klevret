#pragma once

#include "ApiMessage.hpp"
#include <cstdint>


namespace common{


class ApiConnector{
public:
    ApiConnector(uint16_t tcp_port);
    ~ApiConnector();
    // blocking request and receiving response
    ApiMessage send_and_whait_reply(ApiMessage& message_to_send);
private:
    int _socket;
    uint16_t _tcp_port;
};


}
