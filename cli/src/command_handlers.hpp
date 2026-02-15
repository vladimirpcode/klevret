#pragma once
#include "Console.hpp"
#include "Command.hpp"

void cmd_version(std::stack<CommandElementRealValue>& cmd);

void blank(std::stack<CommandElementRealValue>& cmd);

void cmd_ip_address_ipv4(std::stack<CommandElementRealValue>& cmd);

void cmd_dhcp_pool_create(std::stack<CommandElementRealValue>& cmd);

void cmd_dhcp_pool_list(std::stack<CommandElementRealValue>& cmd);
