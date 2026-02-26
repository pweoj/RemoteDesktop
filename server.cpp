#include "server.h"
#include"mdxgi.h"
#include<windows.h>


QVector<QImage> FrameBuffer;//帧缓冲区
QMutex FrameBufferMutex=QMutex();
QVector<QByteArray> SendBuffer;//发送缓冲区
QMutex SendMutex=QMutex();
FFmpeg *FFmpegWorker=nullptr;


Server::Server(QObject *parent)
    : QTcpServer{parent}
{
    Capture=new DxGI(this);
    Capture->DxGiInit();

    Framesend_th=new FrameDealer(this);
    Framesend_th->start();//帧处理,发送子线程


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
    connect(FFmpegWorker,&FFmpeg::FramToPacketIsOk,[=](QByteArray H264Data,int m_index){
        qDebug()<<"index of the packet(after h246):"<<m_index;});


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
            FrameBufferMutex.lock();
            FrameBuffer.append(image);
            emit sendFrame(image);
            FrameBufferMutex.unlock();
        }

        else
            qDebug()<<"no frame";
        Sleep(5);
    }

}

FrameDealer::FrameDealer(QObject *parent):QThread(parent)
{
    FFmpegWorker=new FFmpeg(this);//创建ffmpeg对象
    FFmpegWorker->FFmpegInit();


}

void FrameDealer::run()//子线程主事件
{
    QImage firstImage;
    int index=0;
    while(1){
        FrameBufferMutex.lock();
        if(!FrameBuffer.isEmpty()){
            firstImage=FrameBuffer.takeFirst();
            index++;
        }
        else{
            FrameBufferMutex.unlock();
            QThread::msleep(1);
            continue;
        }
        FrameBufferMutex.unlock();
        //帧处理
        //qDebug()<<"index of the packet(after h246):"<<index;
        FFmpegWorker->FFmpegImageDeal(firstImage.scaled(1024,640,Qt::KeepAspectRatio,Qt::FastTransformation),
                                      index);
        //帧发送
    }
}
