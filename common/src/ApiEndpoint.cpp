#include "ApiEndpoint.hpp"
#include <sstream>

namespace common{

tcp_packet ApiMessage::serialize(){
    json.put("id", id);
    json.put("component", component_name);
    std::ostringstream oss;
    boost::property_tree::write_json(oss, json);
    std::string json_str = oss.str();
    tcp_packet packet(json_str.begin(), json_str.end());
    return packet;
}

void ApiMessage::deserialize(const tcp_packet& packet){
    std::istringstream iss(std::string(packet.begin(), packet.end()));
    json = boost::property_tree::ptree();
    boost::property_tree::read_json(iss, json);
    id = json.get<int>("id", 0);
    component_name = json.get("component", "");
}


ApiEndpoint::ApiEndpoint(uint16_t local_port, const::std::string& remote_ip, uint16_t remote_port)
    : _tcp_listener("127.0.0.1", local_port, 1), _tcp_connector(remote_ip, remote_port)
{

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
