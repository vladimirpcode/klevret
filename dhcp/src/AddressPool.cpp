#include "AddressPool.hpp"


AddressPool::AddressPool(const IPv4Address& start_ip, const IPv4Address& end_ip)
    : _start_ip(start_ip), _end_ip(end_ip)
{
    IPv4Address current = start_ip;
    while (current != end_ip){
        _included_ip_addresses.insert(current);
        current++;
    }
    _included_ip_addresses.insert(end_ip);
}


void AddressPool::include_ip(const IPv4Address& ip){
    _included_ip_addresses.insert(ip);
    _excluded_ip_addresses.erase(ip);
}


void AddressPool::exclude_ip(const IPv4Address& ip){
    _included_ip_addresses.erase(ip);
    _excluded_ip_addresses.insert(ip);
}


void AddressPool::include_ip_range(const IPv4Address& ip_start, const IPv4Address& ip_end){
    IPv4Address current = ip_start;
    while (current != ip_end){
        include_ip(current);
        current++;
    }
    include_ip(ip_end);
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
    for (auto& ip : _included_ip_addresses){
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
    return _included_ip_addresses.find(ip) != _included_ip_addresses.end();
}

bool AddressPool::is_address_excluded(const IPv4Address& ip) const{
    return _excluded_ip_addresses.find(ip) != _excluded_ip_addresses.end();
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

IPv4Address AddressPool::get_start_ip() const{
    return _start_ip;
}

IPv4Address AddressPool::get_end_ip() const{
    return _end_ip;
}
