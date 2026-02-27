#include "server.h"
#include"mdxgi.h"
#include<windows.h>


QVector<QImage> FrameBuffer;//帧缓冲区
QMutex FrameBufferMutex=QMutex();
QVector<QByteArray> SendBuffer;//发送缓冲区
QMutex SendMutex=QMutex();

Server::Server(QObject *parent)
    : QTcpServer{parent}
{
    Capture=new DxGI;
    FrameDeal=new FrameDealer;
    CaptureTh=new QThread(this);//截图线程
    FramDealTh=new QThread(this);

    FrameDeal->moveToThread(FramDealTh);
    FramDealTh->start();
    connect(this,&Server::SocketIsOk,FrameDeal,&FrameDealer::FramDealerRun);
    bool ok=connect(FrameDeal,&FrameDealer::ToServer,this,&Server::SendFrameByte);
    qDebug()<<"[OK]:"<<ok;

    Capture->moveToThread(CaptureTh);
    CaptureTh->start();
    connect(Capture,&DxGI::CaptureIsOk,this,&Server::DealCaptureFrame);
    connect(this,&Server::StartCpatureTh,Capture,&DxGI::DxCaptureScreen);

}

void Server::ServerRun()
{
    qDebug()<<"Server Start...";
    connect(this,&Server::SocketIsOk,&Server::StartCpatureTh);//连接一建立就开截屏

    connect(this,&QTcpServer::newConnection,[=](){
        ClientSocket=this->nextPendingConnection();
        this->close();
        qDebug()<<"new connection...";
        emit SocketIsOk();
    });//有新连接

    listen(QHostAddress::Any,PORT);//非阻塞
    //while(1);
}

void Server::SendFrameByte(const QByteArray& H264Data)
{
    qDebug()<<"[Server]byte send...";
    this->ClientSocket->write(H264Data);
}

void Server::DealCaptureFrame(const QImage &image)//截的屏放缓冲区
{
    //qDebug()<<"[Server]*****";
    FrameBufferMutex.lock();
    FrameBuffer.append(image);
    emit sendFrame(image);
    FrameBufferMutex.unlock();
}


FrameDealer::FrameDealer(QObject *parent)
{
    FFmpegWorker=new FFmpeg(this);
    FFmpegWorker->FFmpegInit();
}

void FrameDealer::FramDealerRun()
{
    QImage firstImage;
        QByteArray pck;
        int index=0;
        while(!QThread::currentThread()->isInterruptionRequested()){
            //qDebug()<<"[FrameDealer]run...";
            FrameBufferMutex.lock();
            if(!FrameBuffer.isEmpty()){
                firstImage=FrameBuffer.takeFirst();
                index++;
            }
            else{
                FrameBufferMutex.unlock();
                QThread::msleep(10);
                continue;
            }
            FrameBufferMutex.unlock();
            pck=FFmpegWorker->FFmpegImageDeal(firstImage.scaled(1024,640,Qt::KeepAspectRatio,Qt::FastTransformation),
                                          index);
            if(pck.size()!=0){

                emit ToServer(pck);
                qDebug()<<pck.size();
            }
            QThread::msleep(5);
        }
}
