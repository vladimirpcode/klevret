#include <iostream>
#include "DhcpMessage.h"

// здесь можно производить отладку
int main(){
    std::vector<uint8_t> dhcp_message_data = {
        0x02, 0x01, 0x06, 0x00, // op, htype, hlen, hops
        0x07, 0xd0, 0xd9, 0x0d, // xid
        0x00, 0x01, 0x00, 0x00, // secs (2), flags (2)
        0x00, 0x00, 0x00, 0x00, // ciaddr
        0xc0, 0xa8, 0x01, 0x0a, // yiaddr
        0xc0, 0xa8, 0x01, 0x01, // siaddr
        0x00, 0x00, 0x00, 0x00, // giaddr
        0x38, 0x01, 0x22, 0xe4, 0xb2, 0xa8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // chaddr (16)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // sname
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ...
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ...
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // end sname
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // file
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ...
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ...
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ...
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ...
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ...
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ...
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // end file
        0x63, 0x82, 0x53, 0x63, // magic cookie
        0x35, 0x01, 0x05,  // (53) DHCP Message Type
        0x36, 0x04, 0xc0, 0xa8, 0x01, 0x01, // (54) server id
        0x33, 0x04, 0x00, 0x00, 0xa8, 0xc0, // (51) IP Address Lease Time
        0x3a, 0x04, 0x00, 0x00, 0x54, 0x60, // (58) Renewal (T1) Time Value
        0x3b, 0x04, 0x00, 0x00, 0x93, 0xa8, // (59) Rebinding (T2) Time Value
        0x01, 0x04, 0xff, 0xff, 0xff, 0x00, // (1) Subnet Mask
        0x1c, 0x04, 0xc0, 0xa8, 0x01, 0xff, // (28) Broadcast Address Option
        0x03, 0x04, 0xc0, 0xa8, 0x01, 0x01, // (3) Router Option
        0x06, 0x04, 0xc0, 0xa8, 0x01, 0x01, // (9) DNS
        0x0c, 0x0c, 0x69, 0x74, 0x2d, 0x6e, 0x6f, 0x74, 0x2d, 0x61, 0x2d, 0x70, 0x72, 0x6f, // (12) Hostname
        0xff
    };
    DhcpMessage dhcp_message(dhcp_message_data);
    std::cout << "DHCP server started\n";
    std::cout << dhcp_message.yiaddr.to_string() << "\n";
}
