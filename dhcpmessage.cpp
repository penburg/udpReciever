#include "dhcpmessage.h"

#include "bytetools.h"

DHCPMessage::DHCPMessage(MicroMessage *other) : MicroMessage(other->getBuffer(), other->getLen())
{
    //cout << "Rebuilding " << endl;
    uint8_t pos = 0;
    pos += SIZE_TYPE;
    pos += SIZE_UID;
    pos += SIZE_MID;
    uint8_t len = 0;
    uint8_t *buff = other->getBuffer();
    ByteTools::arrayCopy(&buff[pos], 0, &len, 0, SIZE_LEN);
    pos += SIZE_LEN;
    init(&buff[pos], len);
    //cout << toString() << endl;
}

void DHCPMessage::init(string cmd, string mac, string ip, string hostname)
{
    command = cmd;
    macAddress = mac;
    ipAddress = ip;
    hostName = hostname;
    interface = "";
    remain = 0;

    isValid = false;
    if(ipAddress.find('.') != string::npos || ipAddress.find(':') != string::npos){
        if(macAddress.find(':') != string::npos){
            isValid = true;
        }
    }
    uint8_t pos = 0;
    pos += SIZE_TYPE;
    pos += SIZE_UID;
    pos += SIZE_MID;

    uint8_t dhcpLen = 0;
    uint8_t *dhcpMsg = getBytes(&dhcpLen);
    uint8_t *oldMM = microMessage;
    uint8_t len = SIZE_MIN + dhcpLen;
    microMessage = new uint8_t[len];
    setLen(len);
    memcpy(microMessage, oldMM, pos);
    ByteTools::arrayCopy(&dhcpLen, 0, microMessage, pos, SIZE_LEN);
    pos += SIZE_LEN;
    ByteTools::arrayCopy(dhcpMsg, 0, microMessage, pos, dhcpLen);
    pos += dhcpLen;
    createCRC(pos);

    delete[] oldMM;
}

void DHCPMessage::init(uint8_t *msg, uint8_t size)
{
    uint8_t len = 1; //Type
    len += 6; //MAC
    len += 1; //IP Ver
    len += 1; //HOST_LEN
    len += 1; //IFACE_LEN
    len += 4; //T_REM
    len += 2; //CRC-16
    if(size > len){
        uint8_t pos = 0;
        TYPE type;
        ByteTools::arrayCopy(msg, pos, (uint8_t *)&type, 0, 1);
        command = typeToString(type);
        pos += 1;

        uint8_t mac[6];
        ByteTools::arrayCopy(msg, pos, (uint8_t *)&mac, 0, 6);
        macAddress = "";
        for(int i = 0; i < 5; i++){
            macAddress += ByteTools::toHex(&mac[i], 1) + ":";
        }
        macAddress += ByteTools::toHex(&mac[5], 1) ;
        pos += 6;

        uint8_t ipVer = 0;
        ByteTools::arrayCopy(msg, pos, (uint8_t *)&ipVer, 0, 1);
        pos += 1;

        uint8_t *ipBytes = new uint8_t[sizeof(struct in6_addr)];

        if(ipVer == 4){
            char str[INET_ADDRSTRLEN];
            ByteTools::arrayCopy(msg, pos, ipBytes, 0, sizeof(struct in_addr));
            inet_ntop(AF_INET, ipBytes, str, INET_ADDRSTRLEN);
            ipAddress = str;
            pos += sizeof(struct in_addr);
        }
        else if(ipVer == 6){
            char str[INET6_ADDRSTRLEN];
            ByteTools::arrayCopy(msg, pos, ipBytes, 0, sizeof(struct in6_addr));
            inet_ntop(AF_INET6, ipBytes, str, INET6_ADDRSTRLEN);
            ipAddress = str;
            pos += sizeof(struct in6_addr);
        }

        uint8_t hostLen = 0;
        ByteTools::arrayCopy(msg, pos, (uint8_t *)&hostLen, 0, 1);
        pos += 1;
        hostName = string((char *)&msg[pos], hostLen);
        pos += hostLen;

        uint8_t ifaceLen = 0;
        ByteTools::arrayCopy(msg, pos, (uint8_t *)&ifaceLen, 0, 1);
        pos += 1;
        interface = string((char *)&msg[pos], ifaceLen);
        pos += ifaceLen;

        remain = 0;
        uint32_t remainNet = 0;
        ByteTools::arrayCopy(msg, pos, (uint8_t *)&remainNet, 0, 4);
        remain = ntohl(remainNet);

        pos += 4;
        uint16_t crc = ByteTools::crc16(0, msg, pos);

        uint16_t check = 0;
        ByteTools::arrayCopy(msg, pos, (uint8_t *)&check, 0, 2);
        uint16_t checkHost = ntohs(check);
        isValid = crc == checkHost;

        delete[] ipBytes;
    }
}

void DHCPMessage::setInterface(const string &newInterface)
{
    interface = newInterface;
}

void DHCPMessage::setRemaining(string time)
{
    try {
        remain = stoul(time);
    }
    catch (std::invalid_argument &e) {
        cerr << "Unable to extract time remaing: " << e.what() << endl;
    }
}

string DHCPMessage::msgToString()
{
    string ret;
    ret += "Action: " + command + "\n";
    ret += "Mac: " + macAddress + "\n";
    ret += "IP: " + ipAddress + "\n";
    ret += "Host: " + hostName + "\n";
    if(!interface.empty()){
        ret += "On Interface: " + interface + "\n";
    }
    if(remain > 0){
        ret += "Time Remaing: " + to_string(remain) + "\n";
    }
    ret += "Valid: ";
    ret += isValid ? "true" : "false";
    return ret;
}

uint8_t *DHCPMessage::toBytes(uint8_t *size)
{
    *size =  1; //Type
    *size += 6; //MAC
    *size += 1; //IP Ver
    *size += 1; //HOST_LEN
    uint8_t hostSize = hostName.size() <= UINT8_MAX ? hostName.size() : 0;
    *size +=  hostSize; //hostName
    *size += 1; //IFACE_LEN
    uint8_t ifaceSize = interface.size() <= UINT8_MAX ? interface.size() : 0;
    *size += ifaceSize; //interface
    *size += 4; //T_REM
    *size += 2; //CRC-16

    uint8_t ipVer = 0;
    uint8_t sizeIP = 0;
    uint8_t *ipBytes = new uint8_t[sizeof(struct in6_addr)];
    int success = -1;
    if(ipAddress.find('.') != string::npos){
        ipVer = 4;
        sizeIP = sizeof(struct in_addr);
        success = inet_pton(AF_INET, ipAddress.c_str(), ipBytes);
    }
    else if(ipAddress.find(':') != string::npos){
        ipVer = 6;
        sizeIP = sizeof(struct in6_addr);
        success = inet_pton(AF_INET6, ipAddress.c_str(), ipBytes);
    }

    if(success == 1){
        *size += sizeIP;
    }
    else{
        sizeIP = 0;
    }

    uint8_t *mac = stringToMac(macAddress);

    uint8_t *ret = new uint8_t[*size];
    size_t pos = 0;
    TYPE type = stringToType(command);
    ByteTools::arrayCopy((uint8_t *)&type, 0, ret, pos, 1);
    pos += 1;
    ByteTools::arrayCopy(mac, 0, ret, pos, 6);
    pos += 6;
    ByteTools::arrayCopy((uint8_t *)&ipVer, 0, ret, pos, 1);
    pos += 1;
    ByteTools::arrayCopy(ipBytes, 0, ret, pos, sizeIP);
    pos += sizeIP;
    ByteTools::arrayCopy((uint8_t *)&hostSize, 0, ret, pos, 1);
    pos += 1;
    ByteTools::arrayCopy((uint8_t *)hostName.c_str(), 0, ret, pos, hostSize);
    pos += hostSize;
    ByteTools::arrayCopy((uint8_t *)&ifaceSize, 0, ret, pos, 1);
    pos += 1;
    ByteTools::arrayCopy((uint8_t *)interface.c_str(), 0, ret, pos, ifaceSize);
    pos += ifaceSize;
    uint32_t remainNet = htonl(remain);
    ByteTools::arrayCopy((uint8_t *)&remainNet, 0, ret, pos, 4);
    pos += 4;
    uint16_t crc = ByteTools::crc16(0, ret, pos);
    uint16_t crcNet = htons(crc);
    ByteTools::arrayCopy((uint8_t *)&crcNet, 0, ret, pos, sizeof(crc));

    delete[] ipBytes;
    delete[] mac;
    return ret;
}

uint8_t *DHCPMessage::stringToMac(string s)
{
    int macI[6];
    if (std::sscanf(s.c_str(),
                    "%02x:%02x:%02x:%02x:%02x:%02x",
                    &macI[0], &macI[1], &macI[2],
                    &macI[3], &macI[4], &macI[5]) != 6)
    {
        cerr << "Unable to parse MAC" << endl;
    }
    uint8_t *mac = new uint8_t[6];
    for(int i = 0; i < 6; i++){
        mac[i] = macI[i];
    }
    return mac;
}

DHCPMessage::TYPE DHCPMessage::stringToType(string c)
{
    TYPE ret = TYPE::UNK;
    if(c.compare("add") == 0){
        ret = TYPE::ADD;
    }
    else if(c.compare("old") == 0){
        ret = TYPE::OLD;
    }
    else if(c.compare("del") == 0){
        ret = TYPE::DEL;
    }
    else if(c.compare("init") == 0){
        ret = TYPE::INIT;
    }
    else if(c.compare("arp-add") == 0){
        ret = TYPE::ARP_ADD;
    }
    else if(c.compare("arp-del") == 0){
        ret = TYPE::ARP_DEL;
    }
    else if(c.compare("relay-snoop") == 0){
        ret = TYPE::RELAY_SNOOP;
    }
    else if(c.compare("tftp") == 0){
        ret = TYPE::TFTP;
    }


    return ret;
}

string DHCPMessage::typeToString(TYPE t)
{
    string ret = "unk";
    switch(t){
    case ADD:
        ret = "add";
        break;
    case OLD:
        ret = "old";
        break;
    case DEL:
        ret = "del";
        break;
    case INIT:
        ret = "init";
        break;
    case ARP_ADD:
        ret = "arp-add";
        break;
    case ARP_DEL:
        ret = "arp-del";
        break;
    case RELAY_SNOOP:
        ret = "relay-snoop";
        break;
    case UNK:
        ret = "unk";
        break;
    case TFTP:
        ret = "tftp";
        break;
    }

    return ret;
}
