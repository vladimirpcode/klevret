#pragma once

#include <cstdint>
#include <vector>
#include <array>
#include <string>
#include <variant>
#include "endians.hpp"


constexpr int IP_V4_ADDRESS_LENGTH = 4;
constexpr int IP_V4_SUBNET_MASK_LENGTH = 4;

class IPv4Address{
public:
    IPv4Address();
    IPv4Address(std::string ip_addr);
    IPv4Address(uint32_t ip_addr);
    static IPv4Address from_big_endian_bytes(std::vector<uint8_t>::iterator begin, std::vector<uint8_t>::iterator end);
    std::string to_string() const;
    std::vector<uint8_t> to_network_data() const;
    IPv4Address operator++(int);
    bool operator==(const IPv4Address& other);
    friend bool operator<(const IPv4Address& lhs, const IPv4Address rhs);
    uint32_t to_uint32_t() const;
private:
    void _increment();
    std::array<uint8_t, IP_V4_ADDRESS_LENGTH> _data;   // big endian
};


bool operator<(const IPv4Address& lhs, const IPv4Address rhs);

class IPv4SubnetMask{
public:
    IPv4SubnetMask(std::string mask);
    IPv4SubnetMask(uint32_t mask);
    static IPv4SubnetMask from_big_endian_bytes(std::vector<uint8_t>::iterator begin, std::vector<uint8_t>::iterator end);
    static IPv4SubnetMask from_prefix(int prefix);
    std::string to_string() const;
    int to_prefix() const;
    std::vector<uint8_t> to_network_data() const;
private:
    std::array<uint8_t, IP_V4_SUBNET_MASK_LENGTH> _data;
};

class IPv6Address{
    IPv6Address(std::string ip_addr);
    IPv6Address(std::vector<uint8_t>::iterator begin, std::vector<uint8_t>::iterator end);
};

class IPv6SubnetMask{
    IPv6SubnetMask(std::string mask);
    IPv6SubnetMask(std::vector<uint8_t>::iterator begin, std::vector<uint8_t>::iterator end);
};

using L3_address_t = std::variant<IPv4Address, IPv6Address>;
using subnet_mask_t = std::variant<IPv4SubnetMask, IPv6SubnetMask>;

enum class L3AddressType{
    IPv4,
    IPv6
};
