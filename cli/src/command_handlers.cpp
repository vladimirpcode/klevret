#include "command_handlers.hpp"

#include <map>
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <sstream>
////
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
////
#include "../../common/src/TcpListener.hpp"
#include "../../common/src/ApiEndpoint.hpp"
#include "../../config/src/Config.hpp"

using namespace std::string_literals;

common::TcpConnector core_tcp_connector(
    "127.0.0.1",
    config::get_global_config().get_json().get<int>("core_external_api_input_tcp_port")
);
common::TcpListener api_reponse_listener(
    "127.0.0.1",
    config::get_global_config().get_json().get<int>("cli_api_replies_input_tcp_port"),
    1
);

void check_args_size(const std::stack<CommandElementRealValue>& cmd, int number_of_args, const std::string cmd_str){
    if (cmd.size() != number_of_args){
        throw std::runtime_error("Внутренняя ошибка обработки команды " + cmd_str);
    }
}

void send_cmd(KlevretComponent klevret_component, boost::property_tree::ptree& obj){
    obj.put("component", "dhcp");
    std::stringstream ss;
    boost::property_tree::write_json(ss, obj);
    std::cout << ss.str() << "\n";
    std::string str = ss.str();
    common::tcp_packet packet(str.begin(), str.end());
    core_tcp_connector.send_message(packet);
}

void cmd_version(std::stack<CommandElementRealValue>& cmd){
    Console::Instance().write_str("Клеврет. Версия 0.1 (в разработке)\n");
}

void blank(std::stack<CommandElementRealValue>& cmd){
    Console::Instance().write_str("пустая команда\n");
}

void cmd_ip_address_ipv4(std::stack<CommandElementRealValue>& cmd){
    check_args_size(cmd, 3, "ip address <IPv4Address>");
    IPv4Address ip = std::get<IPv4Address>(cmd.top());
    boost::property_tree::ptree json;
    json.put("cmd", "ip.address");
    json.put("ip", ip.to_string());
    send_cmd(KlevretComponent::CORE, json);
}

void cmd_dhcp_pool_create(std::stack<CommandElementRealValue>& cmd){
    check_args_size(cmd, 5, "dhcp pool create <IPv4Address> <IPv4Address>");
    IPv4Address ip_end = std::get<IPv4Address>(cmd.top());
    cmd.pop();
    IPv4Address ip_start = std::get<IPv4Address>(cmd.top());
    boost::property_tree::ptree json;
    json.put("component", "dhcp");
    json.put("cmd", "dhcp.pool.create");
    json.put("ip_start", ip_start.to_string());
    json.put("ip_end", ip_end.to_string());
    send_cmd(KlevretComponent::CORE, json);
}
