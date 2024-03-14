#ifndef MESSAGEREBUILDER_H
#define MESSAGEREBUILDER_H

#include <micromessage.h>
#include <dhcpmessage.h>

class MessageReBuilder
{
public:

    MicroMessage *Rebuild(MicroMessage *oldMsg);
    MicroMessage ReBuild(MicroMessage oldMsg);
};

#endif // MESSAGEREBUILDER_H
