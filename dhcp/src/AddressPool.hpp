#pragma once

#include "IpAddress.hpp"
#include <set>
#include <unordered_map>
#include "HardwareAddress.hpp"
#include "DhcpOption.hpp"
#include <boost/property_tree/ptree.hpp>

//включительно
struct IPv4AddressRange{
    IPv4AddressRange(const IPv4Address& start, const IPv4Address& end);
    IPv4Address start;
    IPv4Address end;
    bool contains(const IPv4Address& ip) const;
};

class AddressPool{
public:
    AddressPool(const IPv4Address& start_ip, const IPv4Address& end_ip);
    void exclude_ip(const IPv4Address& ip);
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

    IPv4AddressRange get_addresses_range() const;
    friend boost::property_tree::ptree to_ptree(const AddressPool& address_pool);
    friend AddressPool from_ptree(const boost::property_tree::ptree pt);
private:
    const IPv4AddressRange _addresses_range;
    std::set<IPv4Address> _included_addresses;
    std::set<IPv4Address> _excluded_addresses;
    std::map<IPv4Address, MacAddress> _reserved_ip_addresses; // вместе
    std::map<MacAddress, IPv4Address> _reserved_mac_addresses;// вместе
    std::map<IPv4Address, MacAddress> _taken_ip_addresses; // тоже вместе
    std::map<MacAddress, IPv4Address> _taken_mac_addresses; // тоже вместе
    std::map<MacAddress, IPv4Address> _cache;
    std::map<int, DhcpOption> _options;
    bool is_address_cached(const MacAddress& mac) const;
};

boost::property_tree::ptree to_ptree(const AddressPool& address_pool);
AddressPool addr_pool_from_ptree(const boost::property_tree::ptree pt);
boost::property_tree::ptree to_ptree(const IPv4AddressRange& address_range);
IPv4AddressRange addr_range_from_ptree(const boost::property_tree::ptree pt);
