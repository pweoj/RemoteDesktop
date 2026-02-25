#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include<QHostAddress>
class Client : public QTcpSocket
{
    Q_OBJECT
public:
    Client();
    QHostAddress SerIp;
    int SerPort;
    void ClientRun();
    void ConfirmMode();//ConfirmMode with Server
signals:
   void ConnectIsOk();

};

#endif // CLIENT_H
