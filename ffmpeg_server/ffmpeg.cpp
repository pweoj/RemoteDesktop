#include "ffmpeg.h"
#define m_fps   25
#define m_width 1024
#define m_height 640
FFmpeg::FFmpeg(QObject *parent)
    : QObject{parent}
{}

bool FFmpeg::FFmpegInit()
{
    qDebug()<<"fingding codec";
    const AVCodec*codec=avcodec_find_encoder(AV_CODEC_ID_H264);
    if(codec==nullptr)
    {
        qDebug()<<"no h264 coder";
        return false;
    }
    //关联编码器与其上下文
    qDebug()<<"codec is finded";
    m_codecCtx=avcodec_alloc_context3(codec);
    //配置编码器参数
    qDebug()<<"context";
    m_codecCtx->width = m_width;                  // 输出视频宽度
    m_codecCtx->height = m_height;                // 输出视频高度
    m_codecCtx->pix_fmt = AV_PIX_FMT_YUV420P;     // 像素格式：H.264标准格式（必须YUV420P，否则兼容性差）
    m_codecCtx->framerate = av_make_q(m_fps, 1);  // 帧率：用分数表示（fps/1），如20帧/秒=av_make_q(20,1)
    m_codecCtx->time_base = av_make_q(1, m_fps);  // 时间基：与帧率对应（1/fps），PTS/DTS基于此计算
    m_codecCtx->bit_rate = 2000000;               // 目标码率：2Mbps（2000000 bps），平衡带宽和画质
    m_codecCtx->rc_buffer_size = m_codecCtx->bit_rate; // 码率控制缓冲区大小：等于目标码率
    m_codecCtx->rc_max_rate = m_codecCtx->bit_rate * 1.2; // 最大码率：目标码率的1.2倍，防止码率突增
    m_codecCtx->gop_size = m_fps * 2;             // GOP大小：每2秒生成1个I帧（关键帧），I帧可独立解码，便于网络重连
    m_codecCtx->max_b_frames = 0;                // 关闭B帧：B帧需要参考前后帧，会增加延迟，远程桌面优先低延迟
    m_codecCtx->thread_count = 4;

    int ret=avcodec_open2(m_codecCtx,codec,NULL);

    //m_inputYuv：yuv420参数描述结构体，设置YUV帧的格式和分辨率（必须与编码器一致）
    m_inputYuv=av_frame_alloc();
    m_inputYuv->format=AV_PIX_FMT_YUV420P;
    m_inputYuv->width=m_width;
    m_inputYuv->height=m_height;

    ret=av_frame_get_buffer(m_inputYuv,32);//分配yuv输入缓冲区,32字节对齐
    //分配h264包的参数描述结构体
    m_pkt=av_packet_alloc();

    //创建格式转换上下文
    RGBtoYuv=sws_getContext(m_width,m_height,AV_PIX_FMT_RGB24,m_width,m_height,
                              AV_PIX_FMT_YUV420P,SWS_FAST_BILINEAR,nullptr,nullptr,nullptr);
    if (!RGBtoYuv) {
        qCritical() << "Create sws context failed";
        //freeFFmpegResources(); // 分配失败，释放已分配的资源
        return false;
    }

    m_codec_flage=1;//编码器初始化完成
    return 1;

}

void FFmpeg::FFmpegImageDeal(const QImage &image,int m_frameIndex)
{
    qDebug()<<"deal frame::"<<image.size();
    if(!m_codec_flage) return;

    QImage m_image=image.convertToFormat(QImage::Format_RGB888);
    //RGB转YUV********
    uint8_t* bit[1];
    bit[0]=m_image.bits();
    int bytperline[1];
    bytperline[0]=m_image.bytesPerLine();

    sws_scale(RGBtoYuv,bit,bytperline,0,m_height,m_inputYuv->data,m_inputYuv->linesize);

    m_inputYuv->pts=m_frameIndex;//设置帧时间戳

    int ret=avcodec_send_frame(m_codecCtx,m_inputYuv);//发送帧到编码器缓冲区

    while(ret>=0){//循环读取编码器输出的h.264 packet包
        ret=avcodec_receive_packet(m_codecCtx,m_pkt);
        if(ret == AVERROR(EAGAIN)){
            continue;
            QThread::msleep(1);
        }
        QByteArray h264data((const char*)m_pkt->data,m_pkt->size);
        emit FramToPacketIsOk(h264data,m_pkt->pts);
    }

}
