#pragma once

#include <mutex>
#include <vector>
#include "NetworkInterface.hpp"
#include "AddressPool.hpp"

class ApiServer;

class DhcpServer{
    friend class ApiServer;
public:
    // singleton
    static DhcpServer& Instance();
    void main_loop();
private:
    // singleton
    DhcpServer();
    DhcpServer(const DhcpServer&)=delete;
    DhcpServer& operator=(const DhcpServer&)=delete;
    static DhcpServer* _instance;
    static std::mutex _instance_mutex;

    // dhcp state
    std::mutex _state_change_mutex;
    std::vector<NetworkInterface> _network_interfaces;
    std::vector<AddressPool> _address_pools;

};
