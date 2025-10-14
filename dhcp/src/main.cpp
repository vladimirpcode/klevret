#include <iostream>
#include "DhcpMessage.hpp"
#include "NetworkInterface.hpp"
#include "UdpListener.hpp"
#include <algorithm>

#include <boost/asio.hpp>
#include <boost/algorithm/algorithm.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "AddressPool.hpp"
#include "api_server.hpp"

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

// здесь можно производить отладку
int main(){
    auto ifaces = get_all_interfaces();
    for (auto &iface : ifaces){
        if (iface.type == L3AddressType::IPv4){
            std::cout << iface.name << ": " << std::get<IPv4Address>(iface.network_address).to_string()
                << "/" << std::get<IPv4SubnetMask>(iface.subnet_mask).to_prefix()
                << "  " << iface.mac_address.to_string() <<"\n";
        }
    }
    std::vector<uint8_t> test_dhcp_message_data = {
        0x02, 0x01, 0x06, 0x00, // op, htype, hlen, hops
        0x07, 0xd0, 0xd9, 0x0d, // xid
        0x00, 0x01, 0x00, 0x00, // secs (2), flags (2)
        0x00, 0x00, 0x00, 0x00, // ciaddr
        0xc0, 0xa8, 0x01, 0x0a, // yiaddr
        0xc0, 0xa8, 0x01, 0x01, // siaddr
        0x00, 0x00, 0x00, 0x00, // giaddr
        0x38, 0x01, 0x22, 0xe4, 0xb2, 0xa8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // chaddr (16)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // sname
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ...
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ...
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // end sname
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // file
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ...
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ...
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ...
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ...
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ...
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ...
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // end file
        0x63, 0x82, 0x53, 0x63, // magic cookie
        0x35, 0x01, 0x05,  // (53) DHCP Message Type
        0x36, 0x04, 0xc0, 0xa8, 0x01, 0x01, // (54) server id
        0x33, 0x04, 0x00, 0x00, 0xa8, 0xc0, // (51) IP Address Lease Time
        0x3a, 0x04, 0x00, 0x00, 0x54, 0x60, // (58) Renewal (T1) Time Value
        0x3b, 0x04, 0x00, 0x00, 0x93, 0xa8, // (59) Rebinding (T2) Time Value
        0x01, 0x04, 0xff, 0xff, 0xff, 0x00, // (1) Subnet Mask
        0x1c, 0x04, 0xc0, 0xa8, 0x01, 0xff, // (28) Broadcast Address Option
        0x03, 0x04, 0xc0, 0xa8, 0x01, 0x01, // (3) Router Option
        0x06, 0x04, 0xc0, 0xa8, 0x01, 0x01, // (6) DNS
        0x0c, 0x0c, 0x69, 0x74, 0x2d, 0x6e, 0x6f, 0x74, 0x2d, 0x61, 0x2d, 0x70, 0x72, 0x6f, // (12) Hostname
        0xff
    };
    DhcpMessage test_dhcp_message(test_dhcp_message_data);
    UdpServer udp_listener(67);
    std::cout << "DHCP server started\n";
    //ApiServer::instance().start();
    //wireshark filter udp.payload == 64:63:62:61:60:5f:5e:5d:5c:5b:5a
    //udp_listener.send_to({100, 99, 98, 97, 96,95,94,93,92,91,90});
    AddressPool pool({"172.18.1.30"},{"172.18.1.40"});
    std::map<uint32_t, IPv4Address> xids;
    while(true){
        if (udp_listener.is_input_queue_blank()){
            continue;
        }
        UdpPacketWithInfo packet_with_info = udp_listener.get_next_datagram();
        std::cout << "received datagram length " << packet_with_info.data.size() << "\n";
        NetworkInterface interface = *std::find_if(ifaces.begin(), ifaces.end(), [&](NetworkInterface& iface){
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
            std::cout << "opt 53: " << (int)std::get<uint8_t>(option53->real_values.at(0)) << "\n";
            Option53MessageType msg_type = static_cast<Option53MessageType>(std::get<uint8_t>(option53->real_values.at(0)));
            if (msg_type == Option53MessageType::DHCPDISCOVER){
                MacAddress* mac = dynamic_cast<MacAddress*>(dhcp_packet.chaddr.get());
                xids[dhcp_packet.xid] = pool.get_address(*mac);
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
