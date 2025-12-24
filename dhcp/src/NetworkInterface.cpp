#include "NetworkInterface.hpp"

#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <stdexcept>
#include "endians.hpp"
#include <sys/ioctl.h>
#include <linux/if.h>
#include <unistd.h>
#include <cstring>
#include "../../common/src/Defer.hpp"
#include <iostream>

int linux_if_nametoindex(const char *ifname) {
    int sockfd;
    ifreq ifr;

    // Создаем сокет для ioctl
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        throw std::runtime_error("Не удалось создать сокет для получения индекса интерфейса");
    }
    common::Defer close_socket([&](){
        close(sockfd);
    });

    // Заполняем структуру запроса
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);

    // Получаем индекс через ioctl
    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1) {
        throw std::runtime_error("Не удалось получить индекс сетевого интерфейса");

    }

    return ifr.ifr_ifindex;
}

std::vector<NetworkInterface> get_all_interfaces(){
    // https://man7.org/linux/man-pages/man3/getifaddrs.3.html
    // https://man7.org/linux/man-pages/man3/sockaddr.3type.html
    std::vector<NetworkInterface> result;
    ifaddrs *p_iffaddrs = nullptr;
    if (getifaddrs(&p_iffaddrs) != 0){
        throw std::runtime_error("Не удалось получить сетевые интерфейсы Linux");
    }
    auto current_iface = p_iffaddrs;
    while (current_iface){
        NetworkInterface new_iface;
        new_iface.name = std::string(current_iface->ifa_name);
        new_iface.linux_interface_index = linux_if_nametoindex(current_iface->ifa_name);
        if (current_iface->ifa_addr && current_iface->ifa_addr->sa_family == AF_INET){
            new_iface.type = L3AddressType::IPv4;
            new_iface.network_address = IPv4Address(network_to_host_endian<uint32_t>(((sockaddr_in *)(current_iface->ifa_addr))->sin_addr.s_addr));
            new_iface.subnet_mask = IPv4SubnetMask(network_to_host_endian<uint32_t>(((sockaddr_in *)current_iface->ifa_netmask)->sin_addr.s_addr));
            int fd = socket(AF_INET, SOCK_DGRAM, 0);
            if (fd == -1){
                throw std::runtime_error("не удалось создать сокет get_all_interfaces()");
            }
            common::Defer close_socket([&](){
                close(fd);
            });

            ifreq ifr;
            // get mac
            strcpy(ifr.ifr_ifrn.ifrn_name, new_iface.name.c_str());
            if (ioctl(fd, SIOCGIFHWADDR, &ifr) == -1){
                throw std::runtime_error("ошибка получения информации ioctl(fd, SIOCGIFHWADDR, &ifr) get_all_interfaces()");
            }
            std::vector<uint8_t> mac_data(std::begin(ifr.ifr_ifru.ifru_hwaddr.sa_data), std::begin(ifr.ifr_ifru.ifru_hwaddr.sa_data) + MAC_ADDRESS_LENGTH);
            new_iface.mac_address = MacAddress::from_big_endian_bytes(mac_data.begin(), mac_data.end());

            // get mtu
            strcpy(ifr.ifr_ifrn.ifrn_name, new_iface.name.c_str());
            if (ioctl(fd, SIOCGIFMTU, &ifr) == -1){
                throw std::runtime_error("ошибка получения информации ioctl(fd, SIOCGIFHWADDR, &ifr) get_all_interfaces()");
            }
            new_iface.mtu = ifr.ifr_ifru.ifru_mtu;
            // get broadcast address
            strcpy(ifr.ifr_ifrn.ifrn_name, new_iface.name.c_str());
            if (ioctl(fd, SIOCGIFBRDADDR, &ifr) == -1){
                throw std::runtime_error("Не удалось получить широковещательный IP адрес");
            }
            std::string ip_str = std::to_string(*reinterpret_cast<uint8_t*>(&(ifr.ifr_ifru.ifru_broadaddr.sa_data[2])))
                + "." + std::to_string(*reinterpret_cast<uint8_t*>(&(ifr.ifr_ifru.ifru_broadaddr.sa_data[3])))
                + "." + std::to_string(*reinterpret_cast<uint8_t*>(&(ifr.ifr_ifru.ifru_broadaddr.sa_data[4])))
                + "." + std::to_string(*reinterpret_cast<uint8_t*>(&(ifr.ifr_ifru.ifru_broadaddr.sa_data[5])));
            new_iface.broadcast_address = IPv4Address(ip_str);

            result.push_back(new_iface);
        } else if (current_iface->ifa_addr->sa_family == AF_INET6){

        }
        current_iface = current_iface->ifa_next;
    }
    freeifaddrs(p_iffaddrs);
    return result;
}
