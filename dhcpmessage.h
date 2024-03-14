#ifndef DHCPMESSAGE_H
#define DHCPMESSAGE_H

#include "micromessage.h"
#include <string>
#include <arpa/inet.h>
#include <iostream>

using namespace std;

class DHCPMessage : public MicroMessage
{
public:
    enum TYPE : std::uint8_t {
        ADD = 0,
        OLD = 1,
        DEL = 2,
        INIT = 3,
        ARP_ADD = 4,
        ARP_DEL = 5,
        RELAY_SNOOP = 6,
        TFTP = 7,
        UNK = 255
    };

    DHCPMessage(uint8_t uid[6], uint8_t mid) : MicroMessage(MSG_TYPE::DHCP, uid, mid, 0){};
    DHCPMessage(MicroMessage *other);

    void init(string cmd, string mac, string ip, string hostname);
    void init(uint8_t *msg, uint8_t size);
    void setInterface(const string &newInterface);
    void setRemaining(string time);

    string msgToString() override;
    uint8_t *stringToMac(string s);

private:
    uint8_t *toBytes(uint8_t *size);

    TYPE stringToType(string c);
    string typeToString(TYPE t);

    string hostName;
    string ipAddress;
    string macAddress;
    string command;
    string interface = "";
    uint32_t remain = 0;
    bool isValid = false;


};

#endif // DHCPMESSAGE_H
