#include "client.h"

Client::Client(){}

void Client::ClientRun()
{
    qDebug()<<"Client Start...";

    connect(this,&QTcpSocket::connected,[=](){emit ConnectIsOk();});//连接成功,发ok
    this->connectToHost(this->SerIp,this->SerPort);
    //while(1);
}


