#ifndef SERVER_H
#define SERVER_H
#define PORT 3232
#include <QObject>
#include <QTcpServer>
#include<QDebug>
#include<QTcpSocket>
#include<QImage>
#include<QPixmap>
#include<QScreen>
#include<QApplication>
#include <mdxgi.h>
#include<QVector>
#include<QThread>
#include<QMutex>
#include<QByteArray>
#include"ffmpeg_server/ffmpeg.h"
//*********************************************
class FrameDealer:public QObject
{
    Q_OBJECT
public:
    explicit FrameDealer(QObject *parent=nullptr);
    FFmpeg *FFmpegWorker=nullptr;
    void FramDealerRun();
signals:
    void ToServer(const QByteArray& H264Data);
};

//*********************************************
class Server : public QTcpServer
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);
    QTcpSocket *ClientSocket=0;
    DxGI *Capture=nullptr;
    FrameDealer *FrameDeal=nullptr;
    QThread*CaptureTh=nullptr;
    QThread *FramDealTh=nullptr;


    void ServerRun();
    void ScreenCapture();//截图,与上一帧对比,不同再发，只发不同的区域，非异步
    void SendFrameByte(const QByteArray& H264Data);
    void DealCaptureFrame(const QImage&image);
signals:
    void SocketIsOk();
    void sendFrame(const QImage&image);
    void FrameDealerStart();
    void StartCpatureTh();
};

#endif // SERVER_H
