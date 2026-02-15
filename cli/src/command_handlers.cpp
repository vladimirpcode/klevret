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
#include "../../common/src/ApiConnector.hpp"
#include "../../config/src/Config.hpp"

using namespace std::string_literals;

common::ApiConnector core_api_connector(
    config::get_global_config().json.get<int>("core_external_api_input_tcp_port")
);


void check_args_size(const std::stack<CommandElementRealValue>& cmd, int number_of_args, const std::string cmd_str){
    if (cmd.size() != number_of_args){
        throw std::runtime_error("Внутренняя ошибка обработки команды " + cmd_str);
    }
}

void send_cmd_and_get_reply( boost::property_tree::ptree& obj){
    obj.put("component", "dhcp");
    std::stringstream ss;
    boost::property_tree::write_json(ss, obj);
    std::cout << ss.str() << "\n";
    std::string str = ss.str();
    common::tcp_packet packet(str.begin(), str.end());
    common::ApiMessage api_message(packet);
    common::ApiMessage reply = core_api_connector.send_and_whait_reply(api_message);
    if (reply.json.get<bool>("status", false)){
        Console::Instance().write_str(reply.json.get<std::string>("msg", "") + "\n");
        Console::Instance().change_text_color(Color::GREEN);
        Console::Instance().write_str("OK\n");
        Console::Instance().change_text_color(Color::WHITE);
    } else {
        Console::Instance().write_str(reply.json.get<std::string>("msg", "") + "\n");
        Console::Instance().change_text_color(Color::RED);
        Console::Instance().write_str("ERROR\n");
        Console::Instance().change_text_color(Color::WHITE);
    }
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
    send_cmd_and_get_reply(json);
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
    send_cmd_and_get_reply(json);
}

void cmd_dhcp_pool_list(std::stack<CommandElementRealValue>& cmd){
    boost::property_tree::ptree json;
    json.put("component", "dhcp");
    json.put("cmd", "dhcp.pool.list");
    send_cmd_and_get_reply(json);
}
