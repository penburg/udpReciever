#include "bytetools.h"

ByteTools::ByteTools()
{

}

uint16_t ByteTools::crc16(uint16_t crc, const uint8_t *buffer, size_t len)
{
    while (len--){
        crc = crc16_byte(crc, *buffer++);
    }
    return crc;
}


uint16_t ByteTools::crc16_byte(uint16_t crc, const uint8_t data)
{
    return (crc >> 8) ^ crc16_table[(crc ^ data) & 0xff];
}


void ByteTools::arrayCopy(uint8_t *src, size_t srcStart, uint8_t *dest, size_t destStart, size_t len)
{
    for(size_t i = 0; i < len; i++){
        dest[destStart + i] = src[srcStart + i];
    }
}

uint8_t *ByteTools::arrayConCat(uint8_t *left, size_t leftLen, uint8_t *right, size_t rightLen)
{
    uint8_t *ret = new uint8_t[leftLen + rightLen];
    arrayCopy(left, 0, ret, 0, leftLen);
    arrayCopy(right, 0, ret, leftLen, rightLen);
    return ret;
}

string ByteTools::toHex(uint8_t *bin, int len)
{
    stringstream ss;
    ss << hex;
    for(int i = 0; i < len; i++){
        ss << std::setw(2) << std::setfill('0')  << (int)bin[i];
    }
    return ss.str();
}
