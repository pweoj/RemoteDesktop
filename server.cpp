#include "server.h"
#include"mdxgi.h"
#include<windows.h>
Server::Server(QObject *parent)
    : QTcpServer{parent}
{
    Capture=new DxGI(this);
    Capture->DxGiInit();
}

void Server::ServerRun()
{
    qDebug()<<"Server Start...";
    connect(this,&Server::SocketIsOk,&Server::ScreenCapture);
    connect(this,&QTcpServer::newConnection,[=](){
        ClientSocket=this->nextPendingConnection();
        this->close();
        qDebug()<<"new connection...";
        emit SocketIsOk();
    });//有新连接

    listen(QHostAddress::Any,PORT);//非阻塞
    //while(1);
}

void Server::ScreenCapture()
{

    while (1) {
        QImage image=Capture->DxGetOneFrame();

        //QPixmap piximage=QPixmap::fromImage(image);
        if(!image.isNull())emit sendFrame(image);

        else
            qDebug()<<"no frame";
        Sleep(5);
    }

}
