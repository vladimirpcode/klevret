#include "TcpConnector.hpp"

#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


namespace common{


TcpConnector::TcpConnector(const std::string& server_ip, uint16_t port)
    : _port(port), _socket(-1)
{
    bool failed = true;
    while (failed){
        sockaddr_in server_addr{0};
        _socket = socket(AF_INET, SOCK_STREAM, 0);
        if (_socket == -1){
            failed = true;
            continue;
            //throw std::runtime_error("Socket creation error (TcpConnector)");
        }
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(_port);
        server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
        if (connect(_socket, (sockaddr *)&server_addr, sizeof(server_addr)) == -1){
            failed = true;
            continue;
            //throw std::runtime_error("Connection error (TcpConnector)");
        }
        failed = false;
    }
}

void TcpConnector::send_message(const tcp_packet message){
    size_t bytes_sent = send(_socket, reinterpret_cast<const void *>(message.data()), message.size(), 0);
    if (bytes_sent < 0){
        throw std::runtime_error("error sending message (TcpConnector)");
    }
}


TcpConnector::~TcpConnector(){
    if (_socket >= 0){
        close(_socket);
    }
}

}
