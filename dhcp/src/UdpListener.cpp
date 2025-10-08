#include "UdpListener.hpp"

#include <thread>
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
        std::cout << "словил\n";
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
        std::cout << "щас буду в очередь добавлять\n";
        udp_listener->_mutex.lock();
        udp_listener->_queue.push(datagram_with_info);
        udp_listener->_mutex.unlock();
        std::cout << "добавил в очередь, размер очереди: " << udp_listener->_queue.size() << "\n";
    }
}

UdpServer::~UdpServer(){
    stop();
    _pthread->join();
    delete _pthread;
    close(_socket);
}

UdpPacketWithInfo UdpServer::get_next_datagram(){
    std::cout << "щас буду ждать\n";
    std::lock_guard<std::mutex> lock_guard(_mutex);
    std::cout << "дождался\n";
    UdpPacketWithInfo datagram_with_info = _queue.front();
    _queue.pop();
    std::cout << "возвращаю следующую датаграмму\n";
    return datagram_with_info;
}


bool UdpServer::is_input_queue_blank(){
    std::lock_guard<std::mutex> lock_guard(_mutex);
    return _queue.size() == 0;
}

void UdpServer::stop(){
    _stop_flag = true;
}


void UdpServer::send_to(std::vector<uint8_t> data){
    int raw_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (raw_socket == -1){
        throw std::runtime_error("Не удалось создать сырой сокет UdpServer::send_to");
    }
    Defer close_raw_socket([&](){
        close(raw_socket);
    });
    //ToDo сделать класс для датаграммы, чтобы была инфа с какого интерфейса пришло/
    const char interface_name[100] = "virbr1";
    ifreq ifr;
    memset(&ifr, 0, sizeof ifr);
    strncpy(ifr.ifr_ifrn.ifrn_name, interface_name, IFNAMSIZ - 1);
    if (ioctl(raw_socket, SIOCGIFBRDADDR, &ifr) == -1){
        throw std::runtime_error("Не удалось получить широковещательный IP адрес UdpServer::send_to");
    }
    std::string ip_str = std::to_string(*reinterpret_cast<uint8_t*>(&(ifr.ifr_ifru.ifru_broadaddr.sa_data[2])))
        + "." + std::to_string(*reinterpret_cast<uint8_t*>(&(ifr.ifr_ifru.ifru_broadaddr.sa_data[3])))
        + "." + std::to_string(*reinterpret_cast<uint8_t*>(&(ifr.ifr_ifru.ifru_broadaddr.sa_data[4])))
        + "." + std::to_string(*reinterpret_cast<uint8_t*>(&(ifr.ifr_ifru.ifru_broadaddr.sa_data[5])));
    if (ioctl(raw_socket, SIOGIFINDEX, &ifr) == -1){
        throw std::runtime_error("Не удалось получить индекс интерфейса UdpServer::send_to");
    }
    //=====================================
    int socket_for_sending = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_for_sending == -1){
        throw std::runtime_error("Не удалось создать сокет для отправки UdpServer::send_to");
    }
    int broadcast_enable = 1;
    if(setsockopt(socket_for_sending, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof broadcast_enable) == -1){
        throw std::runtime_error("Не удалось задать опцию SO_BROADCAST сокету для отправки (UdpServer::send_to");
    }
    Defer close_socket_for_sending([&](){
        close(socket_for_sending);
    });
    /*
    sockaddr_ll sa;
    memset(&sa, 0, sizeof sa);
    sa.sll_ifindex = ifr.ifr_ifru.ifru_ivalue;
    sa.sll_family = AF_PACKET;
    if (bind(socket_for_sending, (sockaddr *)&sa, sizeof sa) == -1){
        throw std::runtime_error("Не удалось забиндить сокет для отправки UdpServer::send_to");
    } */
    // ====================================
    sockaddr_in sa2;
    memset(&sa2, 0, sizeof sa2);
    sa2.sin_family = AF_INET;
    sa2.sin_port = htons(68);
    sa2.sin_addr.s_addr = inet_addr(ip_str.c_str());
    char buf[200];
    inet_ntop(AF_INET, &(sa2.sin_addr), buf, 200);
    std::cout << "ip address for UDP sending: " << buf << "\n";
    int sent = sendto(socket_for_sending, data.data(), data.size(), 0, (sockaddr*)&sa2, sizeof sa2);
    std::cout << "отправлено " << sent << " байт\n";
    if (sent <= 0){
        std::cout << "ошибка: " << errno << "\n";EACCES;
        throw std::runtime_error("Не удалось отправить UDP пакет UdpServer::send_to");
    }
}
