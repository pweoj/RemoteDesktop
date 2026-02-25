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
#include"ffmpeg_server/ffmpeg.h"


class Server : public QTcpServer
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);
    QTcpSocket *ClientSocket=0;
    DxGI *Capture=nullptr;
    QThread *Framesend_th=nullptr;


    void ServerRun();
    void ScreenCapture();//截图,与上一帧对比,不同再发，只发不同的区域，非异步
signals:
    void SocketIsOk();
    void sendFrame(const QImage&image);
    void FrameDealerStart();
};
class FrameDealer:public QThread
{
    Q_OBJECT
public:

    explicit FrameDealer(QObject *parent=nullptr);
    FFmpeg *FFmpegWorker=nullptr;
    void run()override;

};
#endif // SERVER_H
