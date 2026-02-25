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
class Server : public QTcpServer
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);
    QTcpSocket *ClientSocket=0;
    DxGI *Capture=nullptr;
    QVector<QImage> FrameBuffer;
    QThread *Framesend_th=nullptr;

    void ServerRun();
    void ScreenCapture();//截图,与上一帧对比,不同再发，只发不同的区域，非异步
    void DealAndSendFrame(const QImage&image);//编解码,发送帧
signals:
    void SocketIsOk();
    void sendFrame(const QImage&image);
};

#endif // SERVER_H
