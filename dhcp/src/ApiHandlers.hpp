#pragma once

#include <map>
#include <string>
#include <boost/property_tree/ptree.hpp>

using handler_func = bool (&)(const boost::property_tree::ptree&, std::string&);

bool dhcp_pool_create(const boost::property_tree::ptree& pt, std::string& error);

const std::map<std::string, handler_func> api_handlers{
    {"dhcp.pool.create", dhcp_pool_create},
};
