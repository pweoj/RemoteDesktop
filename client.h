#ifndef CLIENT_H
#define CLIENT_H

#include <QTcpSocket>
#include <QImage>
#include "ffmpeg_server/frameuncode.h"

class Client : public QTcpSocket
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = nullptr);
    QHostAddress SerIp;
    quint16 SerPort = 3232;
    void ClientRun();

signals:
    void ConnectIsOk();
    void frameReady(QImage frame);  // 转发解码后的图像

private:
    FrameUncode *m_decoder = nullptr;




};

#endif
