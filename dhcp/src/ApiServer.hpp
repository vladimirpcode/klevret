#pragma once

#include <thread>
#include <mutex>
#include <memory>
#include "DhcpServer.hpp"


class ApiServer{
public:
    static constexpr inline int PORT = 40237;
    //singleton
    static ApiServer& instance();
    ~ApiServer();

    void start(DhcpServer& dhcp_server);
private:
    //singleton
    static ApiServer *_instance;
    static std::mutex _instance_mutex;
    ApiServer()=default;
    ApiServer(const ApiServer&)=delete;
    ApiServer(ApiServer&&)=delete;
    ApiServer& operator=(const ApiServer&)=delete;

    DhcpServer* _dhcp_server;
    std::thread *_thread;
};
