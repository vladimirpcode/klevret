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
#include <sstream>
#include "../../common/src/ApiListener.hpp"
#include "../../config/src/Config.hpp"
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
    _thread = new std::thread([&](){
        auto global_config = config::get_global_config();
        std::cout << "start _thread in ApiServer\n";
        int dhcp_api_input_tcp_port = global_config.json.get<int>("dhcp_api_input_tcp_port");
        common::ApiListener api_endpoint(dhcp_api_input_tcp_port);
        std::cout << "start API server loop\n";
        while (true)
        {
            if (api_endpoint.is_there_messages()){
                common::ApiMessage api_message = api_endpoint.get_next_message();
                std::cout << ptree_to_json_string(api_message.json) << "\n";
                try{
                    ptree response;
                    auto api_handler = api_handlers.at(api_message.json.get<std::string>("cmd"));
                    bool result = api_handler(*_dhcp_server, api_message.json, response);
                    common::ApiMessage reply(api_message);
                    reply.json = response;
                    reply.json.put("cmd", api_message.json.get<std::string>("cmd"));
                    api_endpoint.send_reply(reply);
                    // ToDo сделать отдельный канал назад в кор на один порт со всех компонент, еще назад в консоль.
                    // помечать запросы рендомным ID, с разным префиксом для фронтов и консолей. И получится асинхронный
                    // обмен месенджами на основе очередей. И еще сделать в CLI функцию с таймайтом whait_response_for_id()
                } catch (...){
                    std::cout << "ошибка обработки запроса\n";
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
