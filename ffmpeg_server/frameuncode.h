#ifndef FRAMEUNCODE_H
#define FRAMEUNCODE_H

#include <QObject>
#include <QImage>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

class FrameUncode : public QObject
{
    Q_OBJECT
public:
    explicit FrameUncode(QObject *parent = nullptr);
    ~FrameUncode();

    bool UncodeInit();           // 初始化解码器
    QImage decode(const QByteArray &h264Data);  // 解码H.264数据

signals:
    void frameReady(QImage frame);  // 解码完成信号

private:
    AVCodecContext *m_codecCtx = nullptr;
    AVFrame *m_frame = nullptr;         // 解码后的YUV帧
    AVFrame *m_frameRGB = nullptr;      // 转换后的RGB帧
    AVPacket *m_pkt = nullptr;
    SwsContext *m_swsCtx = nullptr;     // YUV转RGB上下文

    uint8_t *m_rgbBuffer = nullptr;     // RGB缓冲区

    int m_width = 1024;
    int m_height = 640;
    bool m_inited = false;
};

#endif
