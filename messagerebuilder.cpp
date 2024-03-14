#include "messagerebuilder.h"



MicroMessage *MessageReBuilder::Rebuild(MicroMessage *oldMsg)
{
    MicroMessage::MSG_TYPE type = oldMsg->getType();
    MicroMessage *ret = oldMsg;
    switch(type){
    case MicroMessage::DHCP:
        ret = new DHCPMessage(oldMsg);
        delete oldMsg;
        return ret;
    default:
        ret = oldMsg;
        return ret;
    }
}

MicroMessage MessageReBuilder::ReBuild(MicroMessage oldMsg)
{
    MicroMessage::MSG_TYPE type = oldMsg.getType();

    switch(type){
    case MicroMessage::DHCP:
    {
        DHCPMessage dMsg = DHCPMessage(&oldMsg);
        return dMsg;
    }
    default:
        return oldMsg;
        break;
    }
}
