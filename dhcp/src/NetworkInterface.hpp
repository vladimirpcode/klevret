#pragma once

#include <vector>
#include <string>
#include "IpAddress.hpp"
#include "HardwareAddress.hpp"



struct NetworkInterface{
    NetworkInterface()
        : name{""}, linux_interface_index(0), network_address(IPv4Address(0)), broadcast_address(IPv4Address(0)),
        subnet_mask(IPv4SubnetMask(0)), type(L3AddressType::IPv4), mtu(1500){}
    std::string name;
    int linux_interface_index;
    L3_address_t network_address;
    L3_address_t broadcast_address; //ToDo
    subnet_mask_t subnet_mask;
    L3AddressType type;
    MacAddress mac_address;
    int mtu;
};

std::vector<NetworkInterface> get_all_interfaces();
