#pragma once

#include <map>
#include <string>
#include <boost/property_tree/ptree.hpp>

class DhcpServer;

using handler_func = bool (&)(DhcpServer&, const boost::property_tree::ptree&, std::string&);

bool dhcp_pool_create(DhcpServer& dhcp_server, const boost::property_tree::ptree& pt, std::string& error);

const std::map<std::string, handler_func> api_handlers{
    {"dhcp.pool.create", dhcp_pool_create},
};
