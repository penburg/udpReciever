#include <QCoreApplication>
#include "udpreciever.h"

int main(int argc, char *argv[])
{

    QCoreApplication a(argc, argv);
    UDPReciever rcv;
    rcv.setPassword("123456");
    return a.exec();
}
