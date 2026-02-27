#include "client.h"

Client::Client(){
    FrameUncode *Uncoder=new FrameUncode(this);
    Uncoder->UncodeInit();
}
void Client::ClientRun()
{
    qDebug()<<"Client Start...";

    connect(this,&QTcpSocket::connected,[=](){
        emit ConnectIsOk();
        qDebug()<<this->peerAddress();
    });//连接成功,发ok

    this->connectToHost(this->SerIp,this->SerPort);

    connect(this,&QTcpSocket::readyRead,[=](){//收数据，解码，显示
        QByteArray recvdata=this->readAll();;
        if(recvdata.isEmpty())
            qDebug()<<"nodata";
        else qDebug()<<"recv:"<<recvdata.size();

    });
    //while(1);

}
FrameUncode::FrameUncode(QObject *parent):QObject(parent)
{


}

void FrameUncode::UncodeInit()
{

}

