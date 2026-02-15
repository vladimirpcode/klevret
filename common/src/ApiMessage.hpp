#pragma once

#include <string>
#include <boost/property_tree/ptree.hpp>
#include "tcp_packet.hpp"


namespace common{


struct ApiMessage{
    ApiMessage(const std::string& component_name, boost::property_tree::ptree json)
        : component_name(component_name), json(json){}
    ApiMessage(const tcp_packet& packet)
        : component_name(""), json(boost::property_tree::ptree()){deserialize(packet);}
    std::string component_name;
    boost::property_tree::ptree json;

    tcp_packet serialize();
    void deserialize(const tcp_packet& packet);
};


}
