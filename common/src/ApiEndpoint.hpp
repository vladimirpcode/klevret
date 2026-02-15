#pragma once

#include "TcpConnector.hpp"
#include "TcpListener.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <string>
#include "ApiMessage.hpp"


namespace common{


/*
abstraсtion over TcpConnector and TcpListener
for klevret components like dhcp, dns, etc...
*/
class ApiEndpoint{
public:
    ApiEndpoint(uint16_t local_port, const::std::string& remote_ip, uint16_t remote_port);
    void send_api_message(ApiMessage& message);
    ApiMessage get_next_message();
    bool is_there_messages();
private:
    TcpConnector _tcp_connector;
    TcpListener _tcp_listener;
};


}
