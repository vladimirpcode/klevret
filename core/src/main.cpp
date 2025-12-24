#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdexcept>
#include <queue>
#include <cstring>
#include <thread>
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <string>
#include <sstream>
#include "../../common/src/TcpListener.hpp"
#include "../../common/src/ApiEndpoint.hpp"
#include "../../config/src/Config.hpp"


int main(){
    auto global_config = config::get_global_config();

    common::TcpListener tcp_listener_for_external_control_components(
        "127.0.0.1",
        global_config.get_json().get<int>("core_external_api_input_tcp_port"),
        2
    );

    //dhcp, dns, etc...
    common::TcpListener tcp_listener_for_internal_components(
        "127.0.0.1",
        global_config.get_json().get<int>("core_internal_api_input_tcp_port"),
        config::count_internal_klevret_components_without_core()
    );

    common::TcpConnector tcp_connector_dhcp(
        "127.0.0.1",
        global_config.get_json().get<int>("dhcp_api_input_tcp_port")
    );
    std::cout << "core server started\n";
    while (true){
        if (!tcp_listener_for_external_control_components.is_empty()) {
            try {
                common::ApiMessage api_message(tcp_listener_for_external_control_components.get_next_packet());
                api_message.json.put("message-type", "request");
                std::string component = api_message.json.get<std::string>("component");
                std::stringstream ss;
                boost::property_tree::write_json(ss, api_message.json);
                std::cout << ss.str() << "\n";
                if (component == "dhcp"){
                    std::cout << "получил команду для DHCP\n";
                    tcp_connector_dhcp.send_message(api_message.serialize());
                }
            } catch (const std::exception& e) {
                std::cerr << "Не удалось распарсить json: " << e.what() << std::endl;
            }
        }
        if (!tcp_listener_for_internal_components.is_empty()){
            try {
                common::ApiMessage api_message(tcp_listener_for_internal_components.get_next_packet());
                api_message.json.put("message-type", "response");
                common::TcpConnector cli_tcp_connector(
                    "127.0.0.1",
                    global_config.get_json().get<int>("cli_api_replies_input_tcp_port")
                );
                cli_tcp_connector.send_message(api_message.serialize());
            } catch (const std::exception& e) {
                std::cerr << "Не удалось распарсить json: " << e.what() << std::endl;
            }
        }
    }
}
