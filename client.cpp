#include "client.h"
#include <QDebug>

Client::Client(QObject *parent) : QTcpSocket(parent)
{
    // 初始化解码器
    m_decoder = new FrameUncode(this);
    if (!m_decoder->UncodeInit()) {
        qDebug() << "Decoder init failed!";
    }
}

void Client::ClientRun()
{
    qDebug() << "Client Start...";

    // 连接成功
    connect(this, &QTcpSocket::connected, [=]() {
        emit ConnectIsOk();
        qDebug() << "Connected to:" << this->peerAddress();
    });

    // 连接服务器
    this->connectToHost(this->SerIp, this->SerPort);

    // 接收数据并解码
    connect(this, &QTcpSocket::readyRead, [=]() {
        QByteArray recvData = this->readAll();
        if (recvData.isEmpty()) {
            qDebug() << "No data received";
            return;
        }

        qDebug() << "Recv:" << recvData.size() << "bytes *** R16="<<recvData.left(16).toHex(' ');

        static QByteArray buffer;
        static int expectedLen = 0;

        buffer.append(recvData);

        while (true) {
            // 等待长度头
            if (expectedLen == 0 && buffer.size() >= 4) {
                expectedLen = ((buffer[0] & 0xFF) << 24) |
                              ((buffer[1] & 0xFF) << 16) |
                              ((buffer[2] & 0xFF) << 8) |
                              (buffer[3] & 0xFF);
                buffer = buffer.mid(4);
            }

            // 等待完整数据
            if (expectedLen > 0 && buffer.size() >= expectedLen) {
                QByteArray frame = buffer.left(expectedLen);
                buffer = buffer.mid(expectedLen);
                expectedLen = 0;

                // 解码完整帧
                QImage img = m_decoder->decode(frame);
                if (!img.isNull()) emit frameReady(img);
            } else {
                break;  // 数据不足，等待下次接收
            }
        }


    });

    // 错误处理
    // connect(this, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
    //         [=](QAbstractSocket::SocketError socketError) {
    //             qDebug() << "Socket error:" << this->errorString();
    //         });
}
