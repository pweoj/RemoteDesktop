#include "frameuncode.h"
#include <QDebug>

FrameUncode::FrameUncode(QObject *parent) : QObject(parent)
{
}

FrameUncode::~FrameUncode()
{
    // 释放资源
    if (m_swsCtx) sws_freeContext(m_swsCtx);
    if (m_frameRGB) av_frame_free(&m_frameRGB);
    if (m_frame) av_frame_free(&m_frame);
    if (m_pkt) av_packet_free(&m_pkt);
    if (m_codecCtx) avcodec_free_context(&m_codecCtx);
    if (m_rgbBuffer) av_free(m_rgbBuffer);
}

bool FrameUncode::UncodeInit()
{
    qDebug() << "Init FFmpeg decoder...";

    // 1. 查找H.264解码器
    const AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) {
        qDebug() << "H.264 decoder not found";
        return false;
    }

    // 2. 分配解码器上下文
    m_codecCtx = avcodec_alloc_context3(codec);
    if (!m_codecCtx) {
        qDebug() << "Failed to alloc codec context";
        return false;
    }

    // 3. 设置解码参数（低延迟模式）
    m_codecCtx->flags |= AV_CODEC_FLAG_LOW_DELAY;      // 低延迟
    m_codecCtx->thread_count = 4;                       // 多线程解码

    // 4. 打开解码器
    int ret = avcodec_open2(m_codecCtx, codec, nullptr);
    if (ret < 0) {
        qDebug() << "Failed to open codec:" << ret;
        return false;
    }

    // 5. 分配帧结构
    m_frame = av_frame_alloc();      // YUV帧
    m_frameRGB = av_frame_alloc();   // RGB帧
    m_pkt = av_packet_alloc();

    // 6. 预分配RGB缓冲区（用于YUV转RGB）
    int rgbSize = av_image_get_buffer_size(AV_PIX_FMT_RGB24, m_width, m_height, 1);
    m_rgbBuffer = (uint8_t*)av_malloc(rgbSize);

    av_image_fill_arrays(m_frameRGB->data, m_frameRGB->linesize,
                         m_rgbBuffer, AV_PIX_FMT_RGB24, m_width, m_height, 1);

    // 7. 创建YUV转RGB上下文
    m_swsCtx = sws_getContext(m_width, m_height, AV_PIX_FMT_YUV420P,
                              m_width, m_height, AV_PIX_FMT_RGB24,
                              SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);

    m_inited = true;
    qDebug() << "Decoder init OK";
    return true;
}

QImage FrameUncode::decode(const QByteArray &h264Data)
{
    if (!m_inited || h264Data.isEmpty()) {
        return QImage();
    }

    // 1. 将QByteArray数据填充到AVPacket
    // 注意：FFmpeg需要可写缓冲区，所以拷贝一份
    m_pkt->data = (uint8_t*)av_malloc(h264Data.size());
    memcpy(m_pkt->data, h264Data.constData(), h264Data.size());
    m_pkt->size = h264Data.size();

    // 2. 发送数据到解码器
    int ret = avcodec_send_packet(m_codecCtx, m_pkt);
    av_freep(&m_pkt->data);  // 释放临时缓冲区
    av_packet_unref(m_pkt);

    if (ret < 0) {
        //qDebug() << "Error sending packet:" << ret;
        return QImage();
    }

    // 3. 接收解码后的帧
    ret = avcodec_receive_frame(m_codecCtx, m_frame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        return QImage();  // 需要更多数据或结束
    }
    if (ret < 0) {
        qDebug() << "Error receiving frame:" << ret;
        return QImage();
    }

    // 4. YUV转RGB
    sws_scale(m_swsCtx, m_frame->data, m_frame->linesize,
              0, m_height, m_frameRGB->data, m_frameRGB->linesize);

    // 5. 构建QImage（不拷贝数据，直接引用）
    QImage image(m_rgbBuffer, m_width, m_height, m_frameRGB->linesize[0],
                 QImage::Format_RGB888);

    // 必须深拷贝，因为m_rgbBuffer会被下一帧覆盖
    return image.copy();
}
