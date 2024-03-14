#include "udpreciever.h"

UDPReciever::UDPReciever(QObject *parent)
    : QObject{parent}
{
    socket = new QUdpSocket(this);

    if(!socket->bind(QHostAddress::AnyIPv4, 9000, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)){
        qDebug() << "Unable to bind";
    }

    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    qDebug() << "Startup";
}

void UDPReciever::HelloUDP()
{
    QByteArray Data;
    Data.append("Hello from UDP");

    socket->writeDatagram(Data, QHostAddress::LocalHost, 9000);
}

void UDPReciever::readyRead()
{
    qDebug() << "Packet Recieved";
    // when data comes in
    QByteArray buffer;
    buffer.resize(socket->pendingDatagramSize());

    QHostAddress sender;
    quint16 senderPort;

    socket->readDatagram(buffer.data(), buffer.size(),
                         &sender, &senderPort);

    QString from = sender.toString() + ":" + QString::number((int)senderPort);
    qDebug() << "Message from:" << from ;
    int pLen = 0;
    uint8_t *pText = chacha.decrypt(password, (uint8_t *)buffer.data(), buffer.length(), &pLen);
    if(pLen > MicroMessage::getSIZE_MIN()){
        MicroMessage *mm = new MicroMessage(pText, pLen);
        //qInfo() << "Origional" << mm->toString().c_str();
        mm = rebuilder.Rebuild(mm);
        if(mm->isValid()){
            qInfo() << "Valid" << mm->toString().c_str();
        }
        else{
            qInfo() << "Message is invalid";
            //qInfo() << mm.toString().c_str();
        }
        delete mm;
    }
    else{
        qDebug() << "Unable to decode message";
    }

    delete[] pText;
    //QCoreApplication::quit();
}

void UDPReciever::setPassword(const string &newPassword)
{
    password = newPassword;
}
