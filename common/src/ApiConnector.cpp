#include "ApiConnector.hpp"

#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


namespace common{


ApiConnector::ApiConnector(uint16_t tcp_port)
    : _tcp_port(tcp_port), _socket(-1)
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
        server_addr.sin_port = htons(_tcp_port);
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        if (connect(_socket, (sockaddr *)&server_addr, sizeof(server_addr)) == -1){
            failed = true;
            continue;
            //throw std::runtime_error("Connection error (TcpConnector)");
        }
        failed = false;
        usleep(1000000);
    }
}


ApiConnector::~ApiConnector(){
    if (_socket >= 0){
        close(_socket);
    }
}

ApiMessage ApiConnector::send_and_whait_reply(ApiMessage& message_to_send){
    tcp_packet packet_to_send = message_to_send.serialize();
    size_t bytes_sent = send(_socket, reinterpret_cast<const void *>(packet_to_send.data()), packet_to_send.size(), 0);
    if (bytes_sent < 0){
        throw std::runtime_error("error sending message (ApiConnector)");
    }
    const int BUFFER_SIZE = 1024*10;
    uint8_t buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_read = recv(_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read == -1){
        throw std::runtime_error("error receiving message (ApiConnector)");
    }
    tcp_packet received_packet{std::begin(buffer), std::begin(buffer) + bytes_read};
    ApiMessage api_message(received_packet);
    return api_message;
}


}
