#include "ApiHandlers.hpp"
#include <iostream>
#include "IpAddress.hpp"
#include "DhcpServer.hpp"


bool dhcp_pool_create(DhcpServer& dhcp_server, const boost::property_tree::ptree& pt, std::string& error){
    std::cout << "dhcp.pool.create handler is called\n";
    IPv4Address start{pt.get<std::string>("ip_start")};
    IPv4Address end{pt.get<std::string>("ip_end")};
    return dhcp_server.add_address_pool(AddressPool(start, end), error);
}
