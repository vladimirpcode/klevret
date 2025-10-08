#pragma once

#include <vector>
#include <cstdint>
#include <queue>
#include <mutex>
#include <atomic>
#include <thread>
#include "HardwareAddress.hpp"
#include "NetworkInterface.hpp"
#include "IpAddress.hpp"

struct UdpPacketWithInfo{
    std::vector<uint8_t> data;
    int interface_index;
    L3_address_t ip_address;
};

class UdpServer{
public:
    UdpServer(int port);
    ~UdpServer();
    UdpPacketWithInfo get_next_datagram();
    bool is_input_queue_blank();
    void stop();
    void send_to(std::vector<uint8_t> data);
private:
    int _socket;
    std::queue<UdpPacketWithInfo> _queue;
    std::mutex _mutex;
    std::atomic<bool> _stop_flag;
    std::thread* _pthread;
    std::vector<NetworkInterface> _host_interfaces;
    static void thread_func(UdpServer* udp_listener);
};
