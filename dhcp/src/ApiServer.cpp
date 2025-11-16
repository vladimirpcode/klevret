#include "ApiServer.hpp"


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <mutex>
#include <queue>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "../../common/src/TcpListener.hpp"
#include "ApiHandlers.hpp"


ApiServer *ApiServer::_instance;
std::mutex ApiServer::_instance_mutex;

ApiServer& ApiServer::instance(){
    if (!_instance){
        std::lock_guard<std::mutex> guard(_instance_mutex);
        if (!_instance){
            _instance = new ApiServer();
        }
    }
    return *_instance;
}


ApiServer::~ApiServer(){
    delete _thread;
}

std::string ptree_to_json_string(const boost::property_tree::ptree& pt) {
    std::ostringstream oss;
    boost::property_tree::write_json(oss, pt);
    return oss.str();
}

void ApiServer::start(DhcpServer& dhcp_server){
    _dhcp_server = &dhcp_server;
    _thread = new std::thread([this](){
        TcpListener tcp_server("127.0.0.1", ApiServer::PORT, 1);
        while (true)
        {
            if (!tcp_server.is_empty()){
                auto packet = tcp_server.get_next_packet();
                try{
                    boost::property_tree::ptree pt;
                    std::istringstream iss(std::string(packet.begin(), packet.end()));
                    boost::property_tree::read_json(iss, pt);
                    std::cout << ptree_to_json_string(pt) << "\n";
                    std::string error_msg;
                    bool result = api_handlers.at(pt.get<std::string>("cmd"))(*this->_dhcp_server, pt, error_msg);
                } catch (...){

                }
            }
        }
    });
    if (_thread->joinable()){
        _thread->detach();
    }
    delete _thread;
}


// ToDo заменить на общий TcpListener. Кажется, что я думал о возможности подключать несколько клиеннтов к серверу
// таких как CLI и фронтенд, но думаю, что нужно положить всю ответственность на Core
// и чтобы DHCP сервер держал одно подключение. Либо написать один на все компоненты ApiServer
// чтобы DHCP мог управляться вне Core
// Вопрос: может ли сервер управляться в обход Core или нет
// Todo: перевести проект на английский язык
