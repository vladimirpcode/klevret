#pragma once

#include <map>
#include <string>
#include <boost/property_tree/ptree.hpp>

class DhcpServer;

using ptree = boost::property_tree::ptree;

using handler_func = bool (&)(DhcpServer&, const ptree&, ptree&);

bool dhcp_pool_create(DhcpServer& dhcp_server, const ptree& request, ptree& response);
bool dhcp_pool_list(DhcpServer& dhcp_server, const ptree& request, ptree& response);

const std::map<std::string, handler_func> api_handlers{
    {"dhcp.pool.create", dhcp_pool_create},
    {"dhcp.pool.list", dhcp_pool_list},
};
