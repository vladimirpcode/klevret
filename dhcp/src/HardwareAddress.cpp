#include "HardwareAddress.hpp"
#include <stdexcept>
#include <sstream>
#include <iomanip>


MacAddress MacAddress::from_big_endian_bytes(std::vector<uint8_t>::iterator begin, std::vector<uint8_t>::iterator end){
    MacAddress mac;
    int i = 0;
    auto iter = begin;
    while (iter != end && i < address_length){
        mac.data.at(i) = *iter;
        ++iter;
        ++i;
    }
    if (i < address_length){
        throw std::runtime_error("MAC адрес заполнен не полностью");
    }
    return mac;
}


MacAddress* MacAddress::make_ptr_from_big_endian_bytes(std::vector<uint8_t>::iterator begin, std::vector<uint8_t>::iterator end){
    MacAddress* mac = new MacAddress();
    int i = 0;
    auto iter = begin;
    while (iter != end && i < address_length){
        mac->data.at(i) = *iter;
        ++iter;
        ++i;
    }
    if (i < address_length){
        throw std::runtime_error("MAC адрес заполнен не полностью");
    }
    return mac;
}


std::string MacAddress::to_string() const{
    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < address_length; ++i){
        ss << std::setfill('0') << std::setw(2);
        ss << static_cast<int>(data[i]);
        if (i != address_length - 1){
            ss << ":";
        }
    }
    return ss.str();
}

chaddr_t MacAddress::to_chaddr(){
    chaddr_t result;
    result.fill(0);
    for (int i = 0; i < MAC_ADDRESS_LENGTH; ++i){
        result.at(i) = data.at(i);
    }
    return result;
}


bool operator<(const MacAddress& lhs, const MacAddress& rhs){
    for (int i = 0; i < MAC_ADDRESS_LENGTH; ++i){
        if (lhs.data[i] > rhs.data[i]){
            return false;
        }
        if (lhs.data[i] < rhs.data[i]){
            return true;
        }
    }
    return false;
}


bool MacAddress::operator==(const MacAddress& other) const{
    return data == other.data;
}
