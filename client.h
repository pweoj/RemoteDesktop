#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include<QHostAddress>
#include<QImage>
#include<QThread>
#include<QDebug>
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
    void ReciveImage(const QImage&image);

};
class FrameUncode:public QObject
{
    Q_OBJECT
public:

    explicit FrameUncode(QObject *parent=nullptr);
    void UncodeInit();//初始化解码
    void UncodeDev(const QImage&image);//解码


};
#endif // CLIENT_H
