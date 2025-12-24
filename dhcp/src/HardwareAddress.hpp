#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <string>

// RFC 1700, page 163
enum class HardwareAddressType : uint8_t{
    Ethernet_10Mb = 1,
    Experimental_Ethernet_3Mb,
    Amateur_Radio_AX_25,
    Proteon_ProNET_Token_Ring,
    Chaos,
    IEEE_802_Networks,
    ARCNET,
    Hyperchannel,
    Lanstar,
    Autonet_Short_Address,
    LocalTalk,
    LocalNet_IBM_PCNet_or_SYTEK_LocalNET,
    Ultra_link,
    SMDS,
    Frame_Relay,
    Asynchronous_Transmission_Mode_ATM,
    HDLC,
    Fibre_Channel,
    Asynchronous_Transmission_Mode_ATM_19,
    Serial_Line,
    Asynchronous_Transmission_Mode_ATM_21
};

// ToDo
constexpr int get_hardware_address_length(HardwareAddressType htype){
    switch (htype)
    {
    case HardwareAddressType::Ethernet_10Mb: return 6;
    case HardwareAddressType::Experimental_Ethernet_3Mb: return -1;
    case HardwareAddressType::Amateur_Radio_AX_25: return -1;
    case HardwareAddressType::Proteon_ProNET_Token_Ring: return -1;
    case HardwareAddressType::Chaos: return -1;
    case HardwareAddressType::IEEE_802_Networks: return -1;
    case HardwareAddressType::ARCNET: return -1;
    case HardwareAddressType::Hyperchannel: return -1;
    case HardwareAddressType::Lanstar: return -1;
    case HardwareAddressType::Autonet_Short_Address: return -1;
    case HardwareAddressType::LocalTalk: return -1;
    case HardwareAddressType::LocalNet_IBM_PCNet_or_SYTEK_LocalNET: return -1;
    case HardwareAddressType::Ultra_link: return -1;
    case HardwareAddressType::SMDS: return -1;
    case HardwareAddressType::Frame_Relay: return -1;
    case HardwareAddressType::Asynchronous_Transmission_Mode_ATM: return -1;
    case HardwareAddressType::HDLC: return -1;
    case HardwareAddressType::Fibre_Channel: return -1;
    case HardwareAddressType::Asynchronous_Transmission_Mode_ATM_19: return -1;
    case HardwareAddressType::Serial_Line: return -1;
    case HardwareAddressType::Asynchronous_Transmission_Mode_ATM_21: return -1;
    }
    return 0;
}

constexpr int DHCP_HARDWARE_ADDRESS_MAX_LENGTH = 16;
using chaddr_t = std::array<uint8_t, DHCP_HARDWARE_ADDRESS_MAX_LENGTH>;

class HardwareAddress{
public:
    virtual chaddr_t to_chaddr()=0;
};

constexpr int MAC_ADDRESS_LENGTH = get_hardware_address_length(HardwareAddressType::Ethernet_10Mb);

class MacAddress : public HardwareAddress{
public:
    MacAddress()=default;
    static MacAddress from_big_endian_bytes(std::vector<uint8_t>::iterator begin, std::vector<uint8_t>::iterator end);
    static MacAddress* make_ptr_from_big_endian_bytes(std::vector<uint8_t>::iterator begin, std::vector<uint8_t>::iterator end);
    std::string to_string() const;
    virtual chaddr_t to_chaddr() override;
    friend bool operator<(const MacAddress& lhs, const MacAddress& rhs);

    const static uint8_t address_length = MAC_ADDRESS_LENGTH;
    std::array<uint8_t, MAC_ADDRESS_LENGTH> data;
};


bool operator<(const MacAddress& lhs, const MacAddress& rhs);
