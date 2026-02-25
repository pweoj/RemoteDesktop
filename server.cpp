#include "server.h"
#include"mdxgi.h"
#include<windows.h>

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}
Server::Server(QObject *parent)
    : QTcpServer{parent}
{
    Capture=new DxGI(this);
    Capture->DxGiInit();

    Framesend_th=new QThread(this);

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
    //connect(this,&Server::sendFrame,&Server::DealAndSendFrame);
    while (1) {
        QImage image=Capture->DxGetOneFrame();

        //QPixmap piximage=QPixmap::fromImage(image);
        if(!image.isNull()){
            FrameBuffer.append(image);
            emit sendFrame(image);
        }

        else
            qDebug()<<"no frame";
        Sleep(5);
    }

}

void Server::DealAndSendFrame(const QImage&image)
{
    qDebug()<<"get a frame from server!!";
    //使用ffmpeg处理并发送帧,多线程
}
