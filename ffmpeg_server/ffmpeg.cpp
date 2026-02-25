#include "ffmpeg.h"

FFmpeg::FFmpeg(QObject *parent)
    : QObject{parent}
{}

void FFmpeg::FFmpegInit()
{

}

void FFmpeg::FFmpegImageDeal(const QImage &image)
{
    //qDebug()<<"deal frame::"<<image.size();
}
