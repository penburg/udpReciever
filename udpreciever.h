#ifndef UDPRECIEVER_H
#define UDPRECIEVER_H

#include "chacha20.h"
#include <QObject>
#include <QUdpSocket>
#include <QNetworkInterface>
#include <QCoreApplication>

#include <messagerebuilder.h>

using namespace std;

class UDPReciever : public QObject
{
    Q_OBJECT
public:
    explicit UDPReciever(QObject *parent = nullptr);

    void HelloUDP();
    void setPassword(const string &newPassword);

signals:

public slots:
    void readyRead();

private:
    MessageReBuilder rebuilder;
    QUdpSocket *socket;
    Chacha20 chacha;
    string password;


};

#endif // UDPRECIEVER_H
