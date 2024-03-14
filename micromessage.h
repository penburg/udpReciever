#ifndef MICROMESSAGE_H
#define MICROMESSAGE_H

#include <cstdint>
#include <memory>
#include <regex>
#include <cstring>
#include <netinet/in.h>

using namespace std;
/*
 * Format v1.0
 *
 * <Bytes>  <Name>      <Desc>
 *  1       MSG_TYPE    The Message Type from enum
 *  6       UID         The User / Device id
 *  1       MID         Message ID
 *  1       LEN         The Message Length
 *  0+      MSG         The message, format determined by MSG_TYPE
 *  2       CRC-16      CRC-16 starting with MSG_TYPE and ending with MSG
 *
 */

class MicroMessage
{
public:

    enum MSG_TYPE : std::uint8_t {
        NUL = 0x00,     //NULL ie invalid
        ACK = 0x06,     //ASCII ACK Code
        NAK = 0x15,     //ASCII NAK Code
        SMS = 0x53,     //ASCII S
        DHCP = 0x44,    //ASCII D
        GPS = 0x47,     //ASCII G
    };

    MicroMessage(MSG_TYPE t, uint8_t uid[6], uint8_t mid, uint8_t *msg, uint8_t l);
    MicroMessage(MSG_TYPE t, uint8_t uid[6], uint8_t mid, uint8_t l = 0);
    MicroMessage(uint8_t *mm, uint8_t size);
    virtual ~MicroMessage();

    uint8_t *getBytes(uint8_t *size) const;
    uint8_t *getMessage(uint8_t *size);
    string toString();
    virtual string msgToString();
    bool isValid();
    static void test();
    static MicroMessage getTestMessage();

    uint8_t getLen() const;


    static uint8_t getSIZE_MIN();
    uint8_t *getBuffer();



    MSG_TYPE getType() const;

protected:
    uint8_t *microMessage = nullptr;
    void createCRC(size_t pos);

    void setType(MSG_TYPE newType);
    void setLen(uint8_t newLen);

    static const uint8_t SIZE_TYPE = 1;
    static const uint8_t SIZE_UID = 6;
    static const uint8_t SIZE_MID = 1;
    static const uint8_t SIZE_LEN = 1;
    static const uint8_t SIZE_CRC = 2;
    static const uint8_t SIZE_MIN = SIZE_TYPE + SIZE_UID + SIZE_MID + SIZE_LEN + SIZE_CRC;


private:
    MSG_TYPE type;
    uint8_t len;






};

#endif // MICROMESSAGE_H
