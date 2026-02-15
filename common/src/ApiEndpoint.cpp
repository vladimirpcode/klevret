#include "ApiEndpoint.hpp"
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <iostream>

namespace common{

tcp_packet ApiMessage::serialize(){
    json.put("component", component_name);
    std::ostringstream oss;
    boost::property_tree::write_json(oss, json);
    std::string json_str = oss.str();
    boost::trim(json_str);
    tcp_packet packet(json_str.begin(), json_str.end());
    return packet;
}

void ApiMessage::deserialize(const tcp_packet& packet){
    std::string data(packet.begin(), packet.end());
    boost::trim(data);
    std::istringstream iss(data);
    json = boost::property_tree::ptree();
    try{
        boost::property_tree::read_json(iss, json);
    } catch (...){
        std::cout << "[[[" << iss.str() << "]]]\n";
        for (const auto c : packet){
            std::cout << (int)c << "\n";
        }
        throw;
    }
    component_name = json.get("component", "");
}


ApiEndpoint::ApiEndpoint(uint16_t local_port, const::std::string& remote_ip, uint16_t remote_port)
    : _tcp_listener("127.0.0.1", local_port, 1), _tcp_connector(remote_ip, remote_port)
{
    std::cout << "API ENDPOINT localhost:" << local_port << " (remote " << remote_ip << ":" << remote_port << ") started\n";
}

void ApiEndpoint::send_api_message(ApiMessage& message){
    _tcp_connector.send_message(message.serialize());
}

ApiMessage ApiEndpoint::get_next_message(){
    tcp_packet packet = _tcp_listener.get_next_packet();
    ApiMessage api_message(packet);
    return api_message;
}

bool ApiEndpoint::is_there_messages(){
    return !_tcp_listener.is_empty();
}


}
