#ifndef FFMPEG_H
#define FFMPEG_H

#include <QObject>
#include<QDebug>
#include<QImage>
extern "C"{
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

class FFmpeg : public QObject
{
    Q_OBJECT
public:
    explicit FFmpeg(QObject *parent = nullptr);
    void FFmpegInit();
    void FFmpegImageDeal(const QImage&image);
    void SendFrameToClient();

signals:
};

#endif // FFMPEG_H
