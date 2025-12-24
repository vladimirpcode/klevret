#pragma once

#include "TcpConnector.hpp"
#include "TcpListener.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <string>


namespace common{


struct ApiMessage{
    ApiMessage(int id, const std::string& component_name, boost::property_tree::ptree json)
        : id(id), component_name(component_name), json(json){}
    ApiMessage(const tcp_packet& packet)
        : id(0), component_name(""), json(boost::property_tree::ptree()){deserialize(packet);}
    int id;
    std::string component_name;
    boost::property_tree::ptree json;

    tcp_packet serialize();
    void deserialize(const tcp_packet& packet);
};

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
