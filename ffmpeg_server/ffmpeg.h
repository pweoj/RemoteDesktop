#ifndef FFMPEG_H
#define FFMPEG_H

#include <QObject>
#include<QDebug>
#include<QImage>
#include<QByteArray>
#include<QThread>
extern "C"{
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include<libswscale/swscale.h>
}

class FFmpeg : public QObject
{
    Q_OBJECT
public:
    explicit FFmpeg(QObject *parent = nullptr);

    //编码器上下文
    AVCodecContext* m_codecCtx=nullptr;
    AVFrame *m_inputYuv=nullptr;
    AVPacket*m_pkt=nullptr;//得到的一个h264包
    SwsContext *RGBtoYuv=nullptr;//图像格式转换上下文
    bool m_codec_flage=0;


    bool FFmpegInit();
    void FFmpegImageDeal(const QImage&image,int m_frameIndex);
    void SendFrameToClient();

signals:
    void FramToPacketIsOk(QByteArray H264Data,int index);
};

#endif // FFMPEG_H
