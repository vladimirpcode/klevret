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
#include "../../common/src/ApiConnector.hpp"
#include "../../common/src/ApiListener.hpp"
#include "../../config/src/Config.hpp"


int main(){
    auto global_config = config::get_global_config();

    common::ApiListener api_listener_for_cli(
        global_config.json.get<int>("core_external_api_input_tcp_port")
    );

    common::ApiConnector api_connector_dhcp(
        global_config.json.get<int>("dhcp_api_input_tcp_port")
    );
    std::cout << "core server started\n";
    while (true){
        if (api_listener_for_cli.is_there_messages()) {
            try {
                common::ApiMessage api_message = api_listener_for_cli.get_next_message();
                api_message.json.put("message-type", "request");
                std::string component = api_message.json.get<std::string>("component");
                std::stringstream ss;
                boost::property_tree::write_json(ss, api_message.json);
                std::cout << ss.str() << "\n";
                if (component == "dhcp"){
                    std::cout << "получил команду для DHCP\n";
                    common::ApiMessage reply = api_connector_dhcp.send_and_whait_reply(api_message);
                    api_listener_for_cli.send_reply(reply);
                }
            } catch (const std::exception& e) {
                std::cerr << "Не удалось распарсить json: " << e.what() << std::endl;
            }
        }
    }
}
