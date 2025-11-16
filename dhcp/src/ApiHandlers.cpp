#include "ApiHandlers.hpp"
#include <iostream>


bool dhcp_pool_create(const boost::property_tree::ptree& pt, std::string& error){
    std::cout << "dhcp.pool.create handler is called\n";
    return true;
}
