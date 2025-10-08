#pragma once

#include <vector>
#include <string>
#include "IpAddress.hpp"
#include "HardwareAddress.hpp"



struct NetworkInterface{
    NetworkInterface()
        : name{""}, linux_interface_index(0), network_address(0),
        subnet_mask(0), type(L3AddressType::IPv4){}
    std::string name;
    int linux_interface_index;
    L3_address_t network_address;
    subnet_mask_t subnet_mask;
    L3AddressType type;
    MacAddress mac_address;
};

std::vector<NetworkInterface> get_all_interfaces();
