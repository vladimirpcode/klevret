#pragma once

#include "IpAddress.hpp"
#include <set>
#include <map>
#include "HardwareAddress.hpp"
#include "DhcpOption.hpp"

class AddressPool{
public:
    AddressPool(const IPv4Address& start_ip, const IPv4Address& end_ip);
    void include_ip(const IPv4Address& ip);
    void exclude_ip(const IPv4Address& ip);
    void include_ip_range(const IPv4Address& ip_start, const IPv4Address& ip_end);
    void exclude_ip_range(const IPv4Address& ip_start, const IPv4Address& ip_end);
    void reserve_ip(const IPv4Address& ip, const MacAddress& mac);
    void unreserve_ip(const IPv4Address& ip);
    IPv4Address get_address(const MacAddress& mac);
    void release_address(const IPv4Address& ip);
    bool is_address_taken(const IPv4Address& ip) const;
    bool is_address_taken(const MacAddress& mac) const;
    bool is_address_reserved(const IPv4Address& ip) const;
    bool is_address_reserved(const MacAddress& mac) const;
    bool is_address_included(const IPv4Address& ip) const;
    bool is_address_excluded(const IPv4Address& ip) const;
    void set_option(DhcpOption option);
    void remove_option(int option_number);

    IPv4Address get_start_ip() const;
    IPv4Address get_end_ip() const;
private:
    const IPv4Address _start_ip; // for info
    const IPv4Address _end_ip; // for info
    std::set<IPv4Address> _included_ip_addresses;
    std::set<IPv4Address> _excluded_ip_addresses;
    std::map<IPv4Address, MacAddress> _reserved_ip_addresses; // вместе
    std::map<MacAddress, IPv4Address> _reserved_mac_addresses;// вместе
    std::map<IPv4Address, MacAddress> _taken_ip_addresses; // тоже вместе
    std::map<MacAddress, IPv4Address> _taken_mac_addresses; // тоже вместе
    std::map<MacAddress, IPv4Address> _cache;
    std::map<int, DhcpOption> _options;
    bool is_address_cached(const MacAddress& mac) const;
};
