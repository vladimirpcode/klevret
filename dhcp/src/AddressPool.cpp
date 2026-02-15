#include "AddressPool.hpp"

#include <algorithm>


IPv4AddressRange::IPv4AddressRange(const IPv4Address& start, const IPv4Address& end)
    : start(start), end(end)
{

}

bool IPv4AddressRange::contains(const IPv4Address& ip) const {
    return ip.to_uint32_t() >= start.to_uint32_t() && ip.to_uint32_t() <= end.to_uint32_t();
}

AddressPool::AddressPool(const IPv4Address& start_ip, const IPv4Address& end_ip)
    :   _addresses_range(start_ip, end_ip)
{
    for (IPv4Address ip = start_ip; ip.to_uint32_t() <= end_ip.to_uint32_t(); ip++){
        _included_addresses.insert(ip);
    }
}



void AddressPool::exclude_ip(const IPv4Address& ip){
    if (std::find(_included_addresses.begin(), _included_addresses.end(), ip) == _included_addresses.end()){
        return;
    }
    _excluded_addresses.insert(ip);
    if (_reserved_ip_addresses.find(ip) != _reserved_ip_addresses.end()){
        unreserve_ip(ip);
    }
    if (_taken_ip_addresses.find(ip) != _taken_ip_addresses.end()){
        release_address(ip);
    }
    auto cache_iter = std::find_if(_cache.begin(), _cache.end(),
        [&](const auto& pair) {
            return pair.second == ip;
        }
    );
    if (cache_iter != _cache.end()){
        _cache.erase(cache_iter);
    }
    _included_addresses.erase(std::find(_included_addresses.begin(), _included_addresses.end(), ip));
}


void AddressPool::exclude_ip_range(const IPv4Address& ip_start, const IPv4Address& ip_end){
    IPv4Address current = ip_start;
    while (current != ip_end){
        exclude_ip(current);
        current++;
    }
    exclude_ip(ip_end);
}

void AddressPool::reserve_ip(const IPv4Address& ip, const MacAddress& mac){
    if (!is_address_included(ip)){
        throw std::runtime_error("Невозможно зарезервировать IP адрес, он отсутствует в пуле.");
    }
    if (is_address_reserved(ip)){
        throw std::runtime_error("Невозможно зарезервировать IP адрес, он уже зарезервирован");
    }
    if (is_address_taken(ip)){
        throw std::runtime_error("Невозможно зарезервировать IP адрес, он используется");
    }
    _reserved_ip_addresses[ip] = mac;
    _reserved_mac_addresses[mac] = ip;
}

void AddressPool::unreserve_ip(const IPv4Address& ip){
    if (!is_address_reserved(ip)){
        throw std::runtime_error("Невозможно разрезервировать IP адрес, он не зарезервирован");
    }
    _reserved_mac_addresses.erase(_reserved_ip_addresses[ip]);
    _reserved_ip_addresses.erase(ip);

}

IPv4Address AddressPool::get_address(const MacAddress& mac){
    // 1, смотрим в резервированные, если нет, то во взятые, если взят, выдаем его же IP
    // если не взят, смотрим в кеш, и выдаем IP из кеша если он не взят
    // иначе выдаем первый свободный IP, заносим в кеш
    if (is_address_reserved(mac)){
        _taken_mac_addresses[mac] = _reserved_mac_addresses[mac];
        _taken_ip_addresses[_reserved_mac_addresses[mac]] = mac;
        return _reserved_mac_addresses[mac];
    }
    if (is_address_taken(mac)){
        return _taken_mac_addresses[mac];
    }
    if (is_address_cached(mac)){
        if (!is_address_taken(mac)){
            _taken_mac_addresses[mac] = _cache[mac];
            _taken_ip_addresses[_cache[mac]] = mac;
            return _cache[mac];
        }
    }
    for (auto& ip : _included_addresses){
        if (is_address_reserved(ip) || is_address_taken(ip)){
            continue;
        }
        _taken_mac_addresses[mac] = ip;
        _taken_ip_addresses[ip] = mac;
        return ip;
    }
    throw std::runtime_error("нет свободных адресов для выдачи");
}

void AddressPool::release_address(const IPv4Address& ip){

}


bool AddressPool::is_address_taken(const IPv4Address& ip) const{
    return _taken_ip_addresses.find(ip) != _taken_ip_addresses.end();
}


bool AddressPool::is_address_taken(const MacAddress& mac) const{
    return _taken_mac_addresses.find(mac) != _taken_mac_addresses.end();
}

bool AddressPool::is_address_reserved(const IPv4Address& ip) const{
    return _reserved_ip_addresses.find(ip) != _reserved_ip_addresses.end();
}


bool AddressPool::is_address_reserved(const MacAddress& mac) const{
    return _reserved_mac_addresses.find(mac) != _reserved_mac_addresses.end();
}

bool AddressPool::is_address_included(const IPv4Address& ip) const{
    return _included_addresses.find(ip) != _included_addresses.end();
}

bool AddressPool::is_address_excluded(const IPv4Address& ip) const{
    return _excluded_addresses.find(ip) != _excluded_addresses.end();
}


bool AddressPool::is_address_cached(const MacAddress& mac) const{
    return _cache.find(mac) != _cache.end();
}


void AddressPool::set_option(DhcpOption option){
    _options[option.description.code] = option;
}

void AddressPool::remove_option(int option_number){
    _options.erase(option_number);
}

IPv4AddressRange AddressPool::get_addresses_range() const{
    return _addresses_range;
}


boost::property_tree::ptree to_ptree(const AddressPool& address_pool){
    boost::property_tree::ptree pt;
    pt.put_child("addresses", to_ptree(address_pool._addresses_range));

    boost::property_tree::ptree excluded_addresses_pt;
    for (auto& ip : address_pool._excluded_addresses){

    }
    return pt;
}

AddressPool addr_pool_from_ptree(const boost::property_tree::ptree pt){

}


boost::property_tree::ptree to_ptree(const IPv4AddressRange& address_range){

}

IPv4AddressRange addr_range_from_ptree(const boost::property_tree::ptree pt){

}
