#include "ApiHandlers.hpp"
#include <iostream>
#include "IpAddress.hpp"
#include "DhcpServer.hpp"


bool dhcp_pool_create(DhcpServer& dhcp_server, const ptree& request, ptree& response){
    IPv4Address start{request.get<std::string>("ip_start")};
    IPv4Address end{request.get<std::string>("ip_end")};
    std::string error;
    bool result = dhcp_server.add_address_pool(AddressPool(start, end), error);
    response.put("status", result);
    response.put("msg", error);
    return result;
}


bool dhcp_pool_list(DhcpServer& dhcp_server, const ptree& request, ptree& response){
    //ToDo надо же еще какую-то ответку прислать
    response.put("msg", "blank msg");
    return false;
}
