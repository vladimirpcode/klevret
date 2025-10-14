#include "UdpListener.hpp"

#include <thread>
#include <random>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdexcept>
#include <cstring>
#include <sys/types.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <cstring>
#include <linux/if_packet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <iostream>
#include "../../common/src/Defer.hpp"
#include "../../common/src/utils.hpp"


UdpServer::UdpServer(int port)
    : _stop_flag(false),
    _pthread(nullptr),
    _host_interfaces(get_all_interfaces())
{
    _socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (_socket == -1){
        throw std::runtime_error("Ошибка создания серверного UDP сокета");
    }

    // Включить получение информации о пакете
    int yes = 1;
    setsockopt(_socket, IPPROTO_IP, IP_PKTINFO, &yes, sizeof(yes));

    sockaddr_in sa_server;
    std::memset(&sa_server, 0, sizeof sa_server);
    sa_server.sin_family = AF_INET;
    sa_server.sin_port = htons(port);
    sa_server.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(_socket, (sockaddr*)&sa_server, sizeof sa_server) == -1){
        throw std::runtime_error("Ошибка создания серверного UDP сокета (bind)");
    }
    _pthread = new std::thread(thread_func, this);
}

void UdpServer::thread_func(UdpServer* udp_listener){
    constexpr int BUFFER_SIZE = 10000;
    uint8_t buffer[BUFFER_SIZE];
    while (!udp_listener->_stop_flag){
        UdpPacketWithInfo datagram_with_info;
        struct iovec iov[1];
        iov[0].iov_base = buffer;
        iov[0].iov_len = BUFFER_SIZE;

        char control_msg[1024];
        struct sockaddr_in client_addr;

        msghdr msg = {0};
        msg.msg_name = &client_addr;
        msg.msg_namelen = sizeof(client_addr);
        msg.msg_iov = iov;
        msg.msg_iovlen = 1;
        msg.msg_control = control_msg;
        msg.msg_controllen = sizeof(control_msg);

        ssize_t recivied_bytes = recvmsg(udp_listener->_socket, &msg, 0);

        if (recivied_bytes <= 0) {
            continue;
        }
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        datagram_with_info.ip_address = IPv4Address(client_ip);
        // Поиск информации о интерфейсе
        cmsghdr *cmsg;
        // ToDo Добавить обработку ошибок
        for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
            if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_PKTINFO) {
                in_pktinfo *pktinfo = (in_pktinfo *)CMSG_DATA(cmsg);
                datagram_with_info.interface_index = pktinfo->ipi_ifindex;
            }
        }
        datagram_with_info.data = std::vector<uint8_t>(std::begin(buffer), std::begin(buffer) + recivied_bytes);
        udp_listener->_mutex.lock();
        udp_listener->_queue.push(datagram_with_info);
        udp_listener->_mutex.unlock();
    }
}

UdpServer::~UdpServer(){
    stop();
    _pthread->join();
    delete _pthread;
    close(_socket);
}

UdpPacketWithInfo UdpServer::get_next_datagram(){
    std::lock_guard<std::mutex> lock_guard(_mutex);
    UdpPacketWithInfo datagram_with_info = _queue.front();
    _queue.pop();
    return datagram_with_info;
}


bool UdpServer::is_input_queue_blank(){
    std::lock_guard<std::mutex> lock_guard(_mutex);
    return _queue.size() == 0;
}

void UdpServer::stop(){
    _stop_flag = true;
}

struct __attribute__((packed)) UdpPacket{
    ethhdr eth;
    iphdr ip;
    udphdr udp;
    uint8_t data[1024];
};

unsigned short calculate_checksum(uint16_t *ptr, int nbytes) {
    long sum;
    unsigned short oddbyte;
    short answer;

    sum = 0;
    while (nbytes > 1) {
        sum += *ptr++;
        nbytes -= 2;
    }
    if (nbytes == 1) {
        oddbyte = 0;
        *((u_char*)&oddbyte) = *(u_char*)ptr;
        sum += oddbyte;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = (short)~sum;
    return answer;
}

int send_packet(int sockfd, bool more_fragments, int fragment_offest, int id, uint8_t data[1024],
        const NetworkInterface& iface, const MacAddress& dst_mac, const IPv4Address& dst_ip){
    std::string source_mac = iface.mac_address.to_string();
    sockaddr_ll s_ll = {0};
    s_ll.sll_ifindex = iface.linux_interface_index;
    s_ll.sll_family = AF_PACKET;
    s_ll.sll_protocol = htons(ETH_P_IP);
    UdpPacket pkt = {0};
    // заполняем Ethernet заголовок
    sscanf(dst_mac.to_string().c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
        &pkt.eth.h_dest[0], &pkt.eth.h_dest[1], &pkt.eth.h_dest[2],
        &pkt.eth.h_dest[3], &pkt.eth.h_dest[4], &pkt.eth.h_dest[5]
    );
    sscanf(source_mac.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
        &pkt.eth.h_source[0], &pkt.eth.h_source[1], &pkt.eth.h_source[2],
        &pkt.eth.h_source[3], &pkt.eth.h_source[4], &pkt.eth.h_source[5]
    );
    pkt.eth.h_proto = htons(ETH_P_IP);

    // Заполняем IP заголовок
    pkt.ip.ihl = 5;
    pkt.ip.version = 4;
    pkt.ip.tos = 0;
    pkt.ip.tot_len = htons(sizeof(iphdr) + sizeof(udphdr) + sizeof(pkt.data));
    pkt.ip.id = id;
    pkt.ip.frag_off = htons((more_fragments ? IP_MF : 0) | fragment_offest);
    pkt.ip.ttl = 64;
    pkt.ip.protocol = IPPROTO_UDP;
    pkt.ip.saddr = inet_addr(std::get<IPv4Address>(iface.network_address).to_string().c_str());
    pkt.ip.daddr = inet_addr(dst_ip.to_string().c_str());
    pkt.ip.check = calculate_checksum((uint16_t *)&pkt.ip, sizeof(pkt.ip));

    // Заполняем UDP заголовок
    pkt.udp.source = htons(67); // порт источника
    pkt.udp.dest = htons(68); // порт назначения
    pkt.udp.len = htons(sizeof(udphdr) + sizeof(pkt.data));

    // копируем payload
    memcpy(pkt.data, data, 1024);

    // UDP checksum опционально можно установить в 0
    pkt.udp.check = 0;

    // отправляем
    int total_len = sizeof(ethhdr) + htons(pkt.ip.tot_len);
    int sent = 0;
    if ((sent = sendto(sockfd, &pkt, total_len, 0, (sockaddr *) &s_ll, sizeof(s_ll))) == -1){
        throw std::runtime_error("ошибка отправки\n");
    }
    return sent;
}

int gen_rand_packet_id(){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1000, 10000);
    return dist(gen);
}

void divide_and_send(int sockfd, const std::vector<uint8_t>& data, const NetworkInterface& iface,
        const MacAddress& dst_mac, const IPv4Address& dst_ip){
    int packets_count = data.size() / 1024;
    if (data.size() % 1024 != 0){
        packets_count++;
    }
    int packet_id = gen_rand_packet_id();
    for (int i = 0; i < packets_count; ++i){
        uint8_t current_data[1024];
        memset(current_data, 0, 1024);
        if (i != packets_count - 1){
            std::copy(data.begin() + (i * 1024), data.begin() + ((i + 1) * 1024), std::begin(current_data));
        } else {
            std::copy(data.begin() + (i * 1024), data.end(), std::begin(current_data));
        }
        bool more_fragments = (i != packets_count - 1) ? true : false;
        send_packet(sockfd, more_fragments, i * 1024, packet_id, current_data, iface, dst_mac, dst_ip);
    }
}

void UdpServer::unicast_send_to(const std::vector<uint8_t>& data, const NetworkInterface& iface,
        const MacAddress& dst_mac, const IPv4Address& dst_ip){
    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd == -1){
        throw std::runtime_error("Ошибка создания сокета UdpServer::send_to");
    }
    Defer close_socket([&](){
        close(sockfd);
    });
    divide_and_send(sockfd, data, iface, dst_mac, dst_ip);
}

void UdpServer::broadcast_send_to(const std::vector<uint8_t>& data, const IPv4Address& dst_broadcast_ip){
    int socket_for_sending = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_for_sending == -1){
        throw std::runtime_error("Не удалось создать сокет для отправки UdpServer::send_to");
    }
    Defer close_socket_for_sending([&](){
        close(socket_for_sending);
    });

    int broadcast_enable = 1;
    if(setsockopt(socket_for_sending, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof broadcast_enable) == -1){
        throw std::runtime_error("Не удалось задать опцию SO_BROADCAST сокету для отправки (UdpServer::send_to");
    }
    sockaddr_in sa2 = {0};
    sa2.sin_family = AF_INET;
    sa2.sin_port = htons(68);
    sa2.sin_addr.s_addr = inet_addr(dst_broadcast_ip.to_string().c_str());

    int sent = sendto(socket_for_sending, data.data(), data.size(), 0, (sockaddr*)&sa2, sizeof sa2);
    if (sent <= 0){
        throw std::runtime_error("Не удалось отправить UDP пакет UdpServer::send_to");
    }
}
