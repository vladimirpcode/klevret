#include "DhcpServer.hpp"

#include "DhcpMessage.hpp"
#include "UdpListener.hpp"
#include <algorithm>
#include <iostream>

#include <boost/asio.hpp>
#include <boost/algorithm/algorithm.hpp>
#include <boost/property_tree/json_parser.hpp>

DhcpServer* DhcpServer::_instance;
std::mutex DhcpServer::_instance_mutex;

DhcpServer& DhcpServer::Instance(){
    if (!_instance){
        std::lock_guard<std::mutex> guard(_instance_mutex);
        if (!_instance){
            _instance = new DhcpServer();
            return *_instance;
        }
    }
    return *_instance;
}

DhcpServer::DhcpServer(){
    _network_interfaces = get_all_interfaces();
    for (auto &iface : _network_interfaces){
        if (iface.type == L3AddressType::IPv4){
            std::cout << iface.name << ": " << std::get<IPv4Address>(iface.network_address).to_string()
                << "/" << std::get<IPv4SubnetMask>(iface.subnet_mask).to_prefix()
                << "  " << iface.mac_address.to_string() <<"\n";
        }
    }
}

enum class Option53MessageType{
    DHCPDISCOVER = 1,
    DHCPOFFER = 2,
    DHCPREQUEST = 3,
    DHCPDECLINE = 4,
    DHCPACK = 5,
    DHCPNAK = 6,
    DHCPRELEASE = 7,
    DHCPINFORM = 8
};

DhcpMessage make_offer(const DhcpMessage& packet, const IPv4Address& ip){
    DhcpMessage offer;
    offer.op = DhcpMessageType::BOOTREPLY;
    offer.htype = HardwareAddressType::Ethernet_10Mb;
    offer.hlen = 6;
    offer.hops = 0;
    offer.xid = packet.xid;
    offer.ciaddr = IPv4Address(0);
    offer.yiaddr = ip;
    offer.siaddr = IPv4Address("172.18.1.1");
    offer.giaddr = IPv4Address(0);
    auto l2_address = dynamic_cast<MacAddress*>(packet.chaddr.get());
    offer.chaddr = std::unique_ptr<HardwareAddress> (new MacAddress(*l2_address));
    std::vector<uint8_t> option53_data{(int)Option53MessageType::DHCPOFFER};
    offer.options.push_back(DhcpOption(53, 1, option53_data.begin(), option53_data.end()));
    std::vector<uint8_t> option1_data{255, 255, 255, 0};
    offer.options.push_back(DhcpOption(1, 4, option1_data.begin(), option1_data.end()));
    std::vector<uint8_t> option3_data{172, 18, 1, 1};
    offer.options.push_back(DhcpOption(3, 4, option3_data.begin(), option3_data.end()));
    std::array<uint8_t, 4> lease_time = host_to_network_endian_array<uint32_t, 4>(86400);
    std::vector<uint8_t> option51_data{lease_time.begin(), lease_time.end()};
    offer.options.push_back(DhcpOption(51, 4, option51_data.begin(), option51_data.end()));

    return offer;
}

DhcpMessage make_acknowledge(const DhcpMessage& packet, const IPv4Address& ip){
    DhcpMessage acknowledge;
    acknowledge.op = DhcpMessageType::BOOTREPLY;
    acknowledge.htype = HardwareAddressType::Ethernet_10Mb;
    acknowledge.hlen = 6;
    acknowledge.hops = 0;
    acknowledge.xid = packet.xid;
    acknowledge.ciaddr = IPv4Address(0);
    acknowledge.yiaddr = ip;
    acknowledge.siaddr = IPv4Address("172.18.1.1");
    acknowledge.giaddr = IPv4Address(0);
    auto l2_address = dynamic_cast<MacAddress*>(packet.chaddr.get());
    acknowledge.chaddr = std::unique_ptr<HardwareAddress> (new MacAddress(*l2_address));
    std::vector<uint8_t> option53_data{(int)Option53MessageType::DHCPACK};
    acknowledge.options.push_back(DhcpOption(53, 1, option53_data.begin(), option53_data.end()));
    std::vector<uint8_t> option1_data{255, 255, 255, 0};
    acknowledge.options.push_back(DhcpOption(1, 4, option1_data.begin(), option1_data.end()));
    std::vector<uint8_t> option3_data{172, 18, 1, 1};
    acknowledge.options.push_back(DhcpOption(3, 4, option3_data.begin(), option3_data.end()));
    uint32_t lease_time = 86400;
    std::vector<uint8_t> option51_data{
        (uint8_t)(lease_time >> 24),
        (uint8_t)((lease_time << 8) >> 24),
        (uint8_t)((lease_time << 16) >> 24),
        (uint8_t)((lease_time & 0xFF))
    };
    acknowledge.options.push_back(DhcpOption(51, 4, option51_data.begin(), option51_data.end()));

    return acknowledge;
}

void DhcpServer::main_loop(){
    UdpServer udp_listener(67);
    std::cout << "DHCP server started\n";
    std::map<uint32_t, IPv4Address> xids;
    while(true){
        if (!have_free_address()){
            continue;
        }
        //todo Ограничить очеред размером
        if (udp_listener.is_input_queue_blank()){
            continue;
        }
        UdpPacketWithInfo packet_with_info = udp_listener.get_next_datagram();
        std::cout << "received datagram length " << packet_with_info.data.size() << "\n";
        NetworkInterface interface = *std::find_if(_network_interfaces.begin(), _network_interfaces.end(), [&](NetworkInterface& iface){
            return iface.linux_interface_index == packet_with_info.interface_index;
        });
        std::cout << "interface:  " << interface.name << "\n";
        DhcpMessage dhcp_packet(packet_with_info.data);
        /*
        std::cout << dhcp_packet.yiaddr.to_string() << "\n";
        for (auto option : dhcp_packet.options){
            std::cout << option.description.code << "\n";
        } */
        std::cout << "op = " << (int)dhcp_packet.op << "\n";
        if (dhcp_packet.op == DhcpMessageType::BOOTREQUEST){
            auto option53 = std::find_if(dhcp_packet.options.begin(), dhcp_packet.options.end(), [](DhcpOption& op){
                return op.description.code == 53;
            });
            //// ToDO
            if (option53 == dhcp_packet.options.end()){
                continue;
            }
            ////
            std::cout << "opt 53: " << (int)std::get<uint8_t>(option53->real_values.at(0)) << "\n";
            Option53MessageType msg_type = static_cast<Option53MessageType>(std::get<uint8_t>(option53->real_values.at(0)));
            if (msg_type == Option53MessageType::DHCPDISCOVER){
                MacAddress* mac = dynamic_cast<MacAddress*>(dhcp_packet.chaddr.get());
                xids[dhcp_packet.xid] = take_address(*mac);
                DhcpMessage offer = make_offer(dhcp_packet, xids[dhcp_packet.xid]);
                udp_listener.broadcast_send_to(offer.to_network_data(), std::get<IPv4Address>(interface.broadcast_address));
                //udp_listener.unicast_send_to(offer.to_network_data(), interface, *dynamic_cast<MacAddress*>(offer.chaddr.get()), IPv4Address("255.255.255.255"));
                std::cout << "отправил DHCPOFFER\n";
            } else if (msg_type == Option53MessageType::DHCPREQUEST){
                DhcpMessage acknowledge = make_acknowledge(dhcp_packet, xids[dhcp_packet.xid]);
                udp_listener.broadcast_send_to(acknowledge.to_network_data(), std::get<IPv4Address>(interface.broadcast_address));
                //udp_listener.unicast_send_to(acknowledge.to_network_data(), interface, *dynamic_cast<MacAddress*>(acknowledge.chaddr.get()), IPv4Address("255.255.255.255"));
                std::cout << "отправил DHCPACK\n";
            }
        }
    }
}


bool DhcpServer::add_address_pool(const AddressPool& pool, std::string& error){
    std::lock_guard<std::mutex> lock_guard{_state_change_mutex};
    for (const auto& current_pool : _address_pools){
        uint32_t adding_start = pool.get_start_ip().to_uint32_t();
        uint32_t adding_end = pool.get_end_ip().to_uint32_t();
        uint32_t current_start = current_pool.get_start_ip().to_uint32_t();
        uint32_t current_end = current_pool.get_end_ip().to_uint32_t();
        if (adding_start >= current_start && adding_start <= current_end){
            error = "Intersection of address pools";
            return false;
        }
        if (adding_end >= current_start && adding_end <= current_end){
            error = "Intersection of address pools";
            return false;
        }
    }
    _address_pools.push_back(pool);
    return true;
}


bool DhcpServer::have_free_address(){
    std::lock_guard<std::mutex> lock_guard(_state_change_mutex);
    if (_address_pools.size() == 0){
        return false;
    }
    return true;
    //ToDo
}


IPv4Address DhcpServer::take_address(const MacAddress& mac){
    std::lock_guard<std::mutex> lock_guard{_state_change_mutex};
    return _address_pools.at(0).get_address(mac);
}
