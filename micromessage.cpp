#include "micromessage.h"
#include "bytetools.h"



MicroMessage::MicroMessage(MSG_TYPE t, uint8_t uid[6], uint8_t mid, uint8_t *msg, uint8_t l) : MicroMessage(t, uid, mid, l)
{
    size_t pos = 0;
    pos += SIZE_TYPE;
    pos += SIZE_UID;
    pos += SIZE_MID;
    pos += SIZE_LEN;
    if(msg != nullptr){
        ByteTools::arrayCopy(msg, 0, microMessage, pos, l);
    }
    pos += l;
    createCRC(pos);

}

MicroMessage::MicroMessage(MSG_TYPE t, uint8_t uid[6], uint8_t mid, uint8_t l)
{
    type = t;
    len = SIZE_MIN + l;
    microMessage = new uint8_t[len];
    size_t pos = 0;
    ByteTools::arrayCopy((uint8_t *)&type, pos, microMessage, 0, SIZE_TYPE);
    pos += SIZE_TYPE;
    ByteTools::arrayCopy(uid, 0, microMessage, pos, SIZE_UID);
    pos += SIZE_UID;
    ByteTools::arrayCopy(&mid, 0, microMessage, pos, SIZE_MID);
    pos += SIZE_MID;
    ByteTools::arrayCopy(&l, 0, microMessage, pos, SIZE_LEN);
    pos += SIZE_LEN;
    createCRC(pos);

}

MicroMessage::MicroMessage(uint8_t *mm, uint8_t size)
{
    if(size >= SIZE_MIN){
        len = size;
        type = MSG_TYPE::NUL;
        microMessage = new uint8_t[len];
        memcpy(microMessage, mm, len);
        ByteTools::arrayCopy(microMessage, 0, (uint8_t *)&type, 0, SIZE_TYPE);
    }
    else{
        len = 0;
        microMessage = nullptr;
        type = MSG_TYPE::NUL;
    }
}

MicroMessage::~MicroMessage()
{
    if(microMessage != nullptr){
        delete[] microMessage;
    }
}

uint8_t *MicroMessage::getBytes(uint8_t *size) const
{
    *size = len;
    uint8_t *ret = new uint8_t[len];
    memcpy(ret, microMessage, len);
    return ret;
}

uint8_t *MicroMessage::getMessage(uint8_t *size)
{
    uint8_t *ret = nullptr;
    *size = -1;
    if(microMessage != nullptr && len >= SIZE_MIN){
        size_t pos = 0;
        pos += SIZE_TYPE;
        pos += SIZE_UID;
        pos += SIZE_MID;
        ByteTools::arrayCopy(microMessage, pos, size, 0, SIZE_LEN);
        pos += SIZE_LEN;
        ret = new uint8_t[*size];
        ByteTools::arrayCopy(microMessage, pos, ret, 0, *size);
    }
    return ret;
}

string MicroMessage::toString()
{
    string ret;
    if(microMessage != nullptr && len >= SIZE_MIN){

        ret += "Type: ";
        size_t pos = 0;
        if(type >= 0x21 && type <= 0x7e){ //ascii "printable"
            char c[] = "z";
            c[0] = type;
            ret += string(c) + "\n";
        }
        else{
            ret += "(0x" + ByteTools::toHex((uint8_t *)&type, SIZE_TYPE) + ")\n";
        }
        pos += SIZE_TYPE;
        ret += "User ID: " + ByteTools::toHex(&microMessage[pos], SIZE_UID) + "\n";
        pos += SIZE_UID;
        ret += "Msg ID: " + ByteTools::toHex(&microMessage[pos], SIZE_MID) + "\n";
        pos += SIZE_MID;
        uint8_t size;
        ByteTools::arrayCopy(microMessage, pos, &size, 0, SIZE_LEN);
        pos += SIZE_LEN + size;
        ret += "Msg Len: " + to_string(size) + "\n";
        ret += "Msg: " + msgToString() + "\n";
        uint16_t crc;
        ByteTools::arrayCopy(microMessage, pos, (uint8_t *)&crc, 0, SIZE_CRC);
        ret += "CRC: " + ByteTools::toHex((uint8_t *)&crc, SIZE_CRC) + "\n";
        ret += "isValid: ";
        ret += isValid() ? "True" : "False";
        ret += "\n";

    }
    else{
        ret = "undefined message";
    }
    return ret;
}

string MicroMessage::msgToString()
{
    string ret;
    uint8_t size = 0;
    uint8_t *msg = getMessage(&size);
    if(msg != nullptr && size > 0){
        if(type == MSG_TYPE::SMS){
            ret = string((char *)msg, size);
        }
        else{
            ret = ByteTools::toHex(msg, size);
        }

    }
    delete[] msg;
    return ret;
}

bool MicroMessage::isValid()
{
    bool ret = false;
    if(microMessage != nullptr && len >= SIZE_MIN){
        size_t pos = 0;
        pos += SIZE_TYPE;
        pos += SIZE_UID;
        pos += SIZE_MID;
        uint8_t size;
        ByteTools::arrayCopy(microMessage, pos, &size, 0, SIZE_LEN);
        pos += SIZE_LEN;
        pos += size;

        if(pos + SIZE_CRC == len){
            uint16_t msgCrc;
            ByteTools::arrayCopy(microMessage, pos, (uint8_t *)&msgCrc, 0, SIZE_CRC);
            uint16_t msgCrcHost = ntohs(msgCrc);

            uint16_t calcCrc = ByteTools::crc16(0, microMessage, pos);
            if(msgCrcHost != calcCrc){
                cerr << "CRC mismatch" << endl;
                cerr << "Expected: " << ByteTools::toHex((uint8_t *)&calcCrc, SIZE_CRC) << endl;
                cerr << "Found: " << ByteTools::toHex((uint8_t *)&msgCrcHost, SIZE_CRC) << endl;
            }

            return msgCrcHost == calcCrc;
        }
    }

    return ret;
}

void MicroMessage::test()
{
    MicroMessage mm = MicroMessage::getTestMessage();
    cout << "Test Message: " << endl;
    cout << mm.toString() << endl;

    uint8_t sizeBytes;
    uint8_t *bytes = mm.getBytes(&sizeBytes);

    cout << "Hex Size: " << std::to_string(sizeBytes) << endl;
    cout << "Hex: " << ByteTools::toHex(bytes, sizeBytes) << endl;

    cout << endl << "Re-Loading Message" << endl;
    MicroMessage load(bytes, sizeBytes);
    cout << load.toString() << endl;

    delete[] bytes;
}

MicroMessage MicroMessage::getTestMessage()
{
    MSG_TYPE tp = MSG_TYPE::SMS;
    uint8_t uid[6] = {0xde, 0xad, 0xbe, 0xef, 0xca, 0xfe};
    uint8_t mid = 0x42;
    string msg = string("Hello World!!");
    uint8_t size = msg.size() + 1;

    MicroMessage mm(tp, uid, mid, (uint8_t *)msg.c_str(), size);

    return mm;
}

uint8_t MicroMessage::getLen() const
{
    return len;
}

uint8_t MicroMessage::getSIZE_MIN()
{
    return SIZE_MIN;
}

uint8_t *MicroMessage::getBuffer()
{
    return microMessage;
}

void MicroMessage::createCRC(size_t pos)
{
    uint16_t crc = ByteTools::crc16(0, microMessage, pos);
    uint16_t crcNet = htons(crc);
    ByteTools::arrayCopy((uint8_t *)&crcNet, 0, microMessage, pos, SIZE_CRC);
}

void MicroMessage::setLen(uint8_t newLen)
{
    len = newLen;
}

MicroMessage::MSG_TYPE MicroMessage::getType() const
{
    return type;
}

void MicroMessage::setType(MSG_TYPE newType)
{
    type = newType;
}
