#include <iostream>
#include "DhcpServer.hpp"
#include "ApiServer.hpp"


int main(){
    DhcpServer& dhcp_server = DhcpServer::Instance();
    ApiServer::instance().start(dhcp_server);
    dhcp_server.main_loop();
}
