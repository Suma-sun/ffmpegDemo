

/**
 * 最基于FFmpeg的视频解码器,解码后输出首帧jpg
 *
 * Nanhua.sun
 * 284425176@qq.com
 *
 */
//#include <jni.h>
#include <string>
#include <cstdio>
#include <ctime>
#include <cassert>
#include "queue"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libswscale/swscale.h"
#include "libavutil/log.h"
#include "libavutil/imgutils.h"
#include <libavutil/pixfmt.h>
#include <libavutil/opt.h>
#include <libavutil/file.h>
#include <libavutil/samplefmt.h>
#include "libswresample/swresample.h"
#include "libavutil/time.h"
#include <libavutil/channel_layout.h>
}

#ifdef ANDROID

#include <jni.h>
#include <android/log.h>

#define LOGE(format, ...)  __android_log_print(ANDROID_LOG_ERROR, "decodelib", format, ##__VA_ARGS__)
#define LOGI(format, ...)  __android_log_print(ANDROID_LOG_INFO,  "decodelib", format, ##__VA_ARGS__)
#else
#define LOGE(format, ...)  printf("(>_<) " format "\n", ##__VA_ARGS__)
#define LOGI(format, ...)  printf("(^_^) " format "\n", ##__VA_ARGS__)
#endif

//存放视频、音频编码格式index的Bean,对应AVFormatContext的streams数组
struct AVCodeIDInfo {
    //视频格式索引
    int video_steam_index = -1;
    //音频格式索引
    int audio_steam_index = -1;

    AVCodeIDInfo() {
    }
};

bool isRunning = false;


/**
 * 根據傳入的AVFormatContext，解析出视频、音频流的index
 * @param *ctx
 * @param *info 存放结果的对象引用指针
 */
void get_avCodeId(AVFormatContext *ctx, AVCodeIDInfo *&info) {
    int ret = -1;
    int video_stream_index = -1;
    int audio_stream_index = -1;

    LOGI("Before get_avCodeId\n");
    // 获取流信息
    ret = avformat_find_stream_info(ctx, nullptr);
    if (ret < 0) {
        // 处理错误...
        LOGE("Error avformat_find_stream_info: %s\n", av_err2str(ret));
        avformat_close_input(&ctx);
        return;
    }

    LOGI("Before foreach find codec streams size %d\n", ctx->nb_streams);
    // 查找视频流
    for (int i = 0; i < ctx->nb_streams; i++) {
        if (ctx->streams[i] == nullptr || ctx->streams[i]->codecpar == nullptr ||
            ctx->streams[i]->codecpar->codec_type) {
            continue;
        }
        if (ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            LOGI("Foreach find AVMEDIA_TYPE_VIDEO  i =%d\n", i);
            video_stream_index = i;
            break;
        } else if (ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            LOGI("Foreach find AVMEDIA_TYPE_AUDIO  i =%d\n", i);
            audio_stream_index = i;
            break;
        } else {
            LOGI("Foreach find filter  i =%d\n", i);
        }
    }

    if (video_stream_index == -1 && audio_stream_index == -1) {
        // 没有找到视频、音频流...
        LOGE("Error not find AVMEDIA_TYPE_VIDEO \n");
        return;
    }
    info = new AVCodeIDInfo();
    // 获取编解码器ID
    if (video_stream_index != -1) {
        LOGI("After foreach find video steam codec index=%d\n", video_stream_index);
        info->video_steam_index = video_stream_index;
    }

    if (audio_stream_index != -1) {
        LOGI("After foreach find audio steam codec index=%d\n", audio_stream_index);
        info->audio_steam_index = audio_stream_index;
    }
}


int frameToImage(AVFrame *frame, enum AVCodecID codecID, uint8_t *outbuf, size_t outbufSize) {
    int ret = 0;
    AVPacket *pkt = av_packet_alloc();
    AVCodecContext *ctx = nullptr;
    AVFrame *rgbFrame = nullptr;
    uint8_t *buffer = nullptr;
    struct SwsContext *swsContext = nullptr;
    const AVCodec *codec = avcodec_find_encoder(codecID);
    if (!codec) {
        printf("avcodec_send_frame error %d", codecID);
        goto end;
    }
    if (!codec->pix_fmts) {
        printf("unsupport pix format with codec %s", codec->name);
        goto end;
    }
    ctx = avcodec_alloc_context3(codec);
    ctx->bit_rate = 3000000;
    ctx->width = frame->width;
    ctx->height = frame->height;
    ctx->time_base.num = 1;
    ctx->time_base.den = 25;
    ctx->gop_size = 10;
    ctx->max_b_frames = 0;
    ctx->thread_count = 1;
    ctx->pix_fmt = *codec->pix_fmts;
    ret = avcodec_open2(ctx, codec, nullptr);
    if (ret < 0) {
        printf("avcodec_open2 error %d", ret);
        goto end;
    }
    if (frame->format != ctx->pix_fmt) {
        rgbFrame = av_frame_alloc();
        if (rgbFrame == nullptr) {
            printf("av_frame_alloc  fail");
            goto end;
        }
        swsContext = sws_getContext(frame->width, frame->height, (enum AVPixelFormat) frame->format,
                                    frame->width, frame->height, ctx->pix_fmt, 1, nullptr, nullptr, nullptr);
        if (!swsContext) {
            printf("sws_getContext  fail");
            goto end;
        }
        int bufferSize = av_image_get_buffer_size(ctx->pix_fmt, frame->width, frame->height, 1) * 2;
        buffer = (unsigned char *) av_malloc(bufferSize);
        if (buffer == nullptr) {
            printf("buffer alloc fail:%d", bufferSize);
            goto end;
        }
        av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, buffer, ctx->pix_fmt, frame->width,
                             frame->height, 1);
        if ((ret = sws_scale(swsContext, frame->data, frame->linesize, 0, frame->height,
                             rgbFrame->data, rgbFrame->linesize)) < 0) {
            printf("sws_scale error %d", ret);
        }
        rgbFrame->format = ctx->pix_fmt;
        rgbFrame->width = ctx->width;
        rgbFrame->height = ctx->height;
        ret = avcodec_send_frame(ctx, rgbFrame);
    } else {
        ret = avcodec_send_frame(ctx, frame);
    }
    if (ret < 0) {
        printf("avcodec_send_frame error %d", ret);
        goto end;
    }
    ret = avcodec_receive_packet(ctx, pkt);
    if (ret < 0) {
        printf("avcodec_receive_packet error %d", ret);
        goto end;
    }
    if (pkt->size > 0 && pkt->size <= outbufSize)
        memcpy(outbuf, pkt->data, pkt->size);
    ret = pkt->size;
    end:
    if (swsContext) {
        sws_freeContext(swsContext);
    }
    if (rgbFrame) {
        av_frame_unref(rgbFrame);
        av_frame_free(&rgbFrame);
    }
    if (buffer) {
        av_free(buffer);
    }
    av_packet_unref(pkt);
    if (pkt) {
        av_packet_free(&pkt);
    }
    if (ctx) {
        avcodec_free_context(&ctx);
    }
    return ret;
}

/// <summary>
/// 将视频帧保存为jpg图片
/// </summary>
/// <param name="frame">视频帧</param>
/// <param name="path">保存的路径</param>
void saveFrameToJpg(AVFrame *frame, const char *path) {
    //确保缓冲区长度大于图片,使用brga像素格式计算。如果是bmp或tiff依然可能超出长度，需要加一个头部长度，或直接乘以2。
    int bufSize = av_image_get_buffer_size(AV_PIX_FMT_BGRA, frame->width, frame->height, 64);
    //申请缓冲区
    uint8_t *buf = (uint8_t *) av_malloc(bufSize);
    //将视频帧转换成jpg图片，如果需要png则使用AV_CODEC_ID_PNG
    int picSize = frameToImage(frame, AV_CODEC_ID_MJPEG, buf, bufSize);
    //写入文件
    auto f = fopen(path, "wb+");
    if (f) {
        fwrite(buf, sizeof(uint8_t), bufSize, f);
        fclose(f);
    }
    //释放缓冲区
    av_free(buf);
}


struct buffer_data {
    uint8_t *ptr;
    size_t size; ///< size left in the buffer
};

int read_packet(void *opaque, uint8_t *buf, int buf_size) {
    struct buffer_data *bd = (struct buffer_data *) opaque;
    buf_size = FFMIN(buf_size, bd->size);

    if (!buf_size)
        return AVERROR_EOF;
    printf("ptr:%p size:%zu\n", bd->ptr, bd->size);

    /* copy internal buffer data to buf */
    memcpy(buf, bd->ptr, buf_size);
    bd->ptr += buf_size;
    bd->size -= buf_size;
    return buf_size;
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_nativelib_DecodeTool_decodeMP4ToImage2(JNIEnv *env, jobject thiz,
                                                        jstring inputFilePath, jstring path) {
    AVFormatContext *fmt_ctx = nullptr;
    AVIOContext *avio_ctx = nullptr;
    AVCodecContext *codec_ctx = nullptr;
    const AVCodec *codec = nullptr;
    AVPacket *packet = nullptr;
    uint8_t *buffer = nullptr, *avio_ctx_buffer = nullptr;
    size_t buffer_size, avio_ctx_buffer_size = 4096;
    const char *input_filename = env->GetStringUTFChars(inputFilePath, nullptr);
    int ret;
    struct buffer_data bd = {0};
    AVCodeIDInfo *info = nullptr;
    int video_stream_index = -1;

    /* slurp file content into buffer */
    ret = av_file_map(input_filename, &buffer, &buffer_size, 0, nullptr);
    if (ret < 0) {
        LOGE("av_file_map fail %s", av_err2str(ret));
        goto end;
    }

    /* fill opaque structure used by the AVIOContext read callback */
    bd.ptr = buffer;
    bd.size = buffer_size;

    if (!(fmt_ctx = avformat_alloc_context())) {
        ret = AVERROR(ENOMEM);
        LOGE("avformat_alloc_context fail %s", av_err2str(ret));
        goto end;
    }

    avio_ctx_buffer = static_cast<uint8_t *>(av_malloc(avio_ctx_buffer_size));
    if (!avio_ctx_buffer) {
        ret = AVERROR(ENOMEM);
        LOGE("av_malloc fail %s", av_err2str(ret));
        goto end;
    }

    avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size,
                                  0, &bd, &read_packet, nullptr, nullptr);
    if (!avio_ctx) {
        ret = AVERROR(ENOMEM);
        LOGE("avio_alloc_context fail %s", av_err2str(ret));
        goto end;
    }
    fmt_ctx->pb = avio_ctx;

    ret = avformat_open_input(&fmt_ctx, nullptr, nullptr, nullptr);
    if (ret < 0) {
        LOGE("avio_alloc_context fail %s", av_err2str(ret));
        goto end;
    }

    // 解码开始

    get_avCodeId(fmt_ctx, info);
    if (info == nullptr || info->video_steam_index == -1) {
        LOGE("Video Steam is NULL");
        goto end;
    }
    video_stream_index = info->video_steam_index;
    codec = avcodec_find_decoder(fmt_ctx->streams[video_stream_index]->codecpar->codec_id);
    codec_ctx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codec_ctx, fmt_ctx->streams[video_stream_index]->codecpar);

    LOGI("Before open avcodec");
    // 打开编解码器并分配解码器上下文
    if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
        LOGE("Could not open codec.\n");
        goto end;
    }

    // 分配解码器上下文
    AVFrame *frame;
    frame = av_frame_alloc();

    // 读取并解码视频帧
    packet = av_packet_alloc();

    packet->data = nullptr;
    packet->size = 0;

//    int frame_count = 0;
    while (av_read_frame(fmt_ctx, packet) >= 0) {
        if (packet->stream_index == video_stream_index) {
            int result = avcodec_send_packet(codec_ctx, packet);
            if (result < 0) {
                LOGE("Error sending the packet\n");
                break;
            }

            result = avcodec_receive_frame(codec_ctx, frame);
            if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
                break;
            } else if (result < 0) {
                LOGE("Error during decoding\n");
                break;
            }

            // 首帧处理
//            if (frame_count == 0) {
            // 提取首帧并保存为图像文件
            const char *outPath = env->GetStringUTFChars(path, JNI_FALSE);
            saveFrameToJpg(frame, outPath);
            env->ReleaseStringUTFChars(path, outPath);
            break;
//            }
        }
        av_packet_unref(packet);
    }

    // 释放资源
    av_frame_free(&frame);
    avcodec_free_context(&codec_ctx);
    // 解码结束

    end:
    if (info) {
        delete info;
    }
    avformat_close_input(&fmt_ctx);
    avcodec_free_context(&codec_ctx);
    av_file_unmap(buffer, buffer_size);
    /* note: the internal buffer could have changed, and be != avio_ctx_buffer */
//    if (avio_ctx)
//        av_freep(&avio_ctx->buffer);

    avio_context_free(&avio_ctx);

    av_file_unmap(buffer, buffer_size);
    env->ReleaseStringUTFChars(inputFilePath, input_filename);

    if (ret < 0) {
        LOGE("Error occurred: %s", av_err2str(ret));
    } else {
        LOGI("Save success");
    }
}

/**
 * 获取音视频流的每帧间隔时间（微秒）
 * @param ctx
 * @param streamIndex
 * @return
 */
double getDelayTime(AVFormatContext *ctx, int streamIndex) {
    AVRational rate = ctx->streams[streamIndex]->avg_frame_rate;
    double fps = static_cast<double>(rate.num) / rate.den;
    //计算出延时时间（单位：微秒）
    return 1.0f / fps * 1000000;
}

#include <android/native_window_jni.h>
#include <zconf.h>


extern "C"
JNIEXPORT jint JNICALL
/**
 * 根据指定视频文件，解码视频帧，通过surface进行渲染播放(进度只是简单的文件大小播放进度，不是时间判断)
 * @param env
 * @param thiz
 * @param path 文件路径
 * @param surface java层的surface实例
 * @param file_size 文件大小
 * @param callback 进度回调
 * @return
 */
Java_com_example_nativelib_DecodeTool_startVideoPlay(JNIEnv *env, jobject thiz, jstring path,
                                                     jobject surface, jlong file_size,
                                                     jobject callback) {
    int result = -1;
    int ret = -1;
    //用于解析文件中音视频流的顶层上下文
    AVFormatContext *ctx = avformat_alloc_context();
    AVCodeIDInfo *info = nullptr;
    //编解码器参数信息
    AVCodecParameters *params = nullptr;
    //编解码器
    const AVCodec *avCodec;
    //编解码所需的上下文
    AVCodecContext *codecContext;
    //存储压缩编码数据的结构体
    AVPacket *pkt = nullptr;
    //存储一帧原始数据的结构，未编码的如YUV、RGB、PCM
    AVFrame *frame = nullptr;
    SwsContext *swsContext = nullptr;
    //每帧间隔
    double delayTime = 0.0;

    FILE *cFile = nullptr;

    //IO输入/输出上下文
    AVIOContext *avio_ctx = nullptr;
    uint8_t *buffer = nullptr, *avio_ctx_buffer = nullptr;
    size_t buffer_size, avio_ctx_buffer_size = 4096;
    struct buffer_data bd = {0};

    //获取绘制用的Native view
    ANativeWindow *aNativeWindow = ANativeWindow_fromSurface(env, surface);
    //绘制时，用于接收数据的缓冲区
    ANativeWindow_Buffer windowBuffer;
    //java string 转 C++能用的字符串
    const char *cPath = env->GetStringUTFChars(path, JNI_FALSE);
    //文件大小
    const long size = file_size;
    //当前读取到的数据量
    double curr_size = 0;

    jclass callbackClass = env->FindClass("com/example/nativelib/DecodeTool$ProgressCallback");
    jmethodID call_invoke = env->GetMethodID(callbackClass, "invoke", "(I)V");

    //初始化网络模块，可以播放url
//    avformat_network_init();

    //配置超时时间？
    AVDictionary *option = nullptr;
    av_dict_set(&option, "timeout", "3000000", 0);

    //AVIO 安卓系统文件访问需要权限，使用IO可以避免用文件管理系统

    //读取指定的文件，并将内容放入新分配的缓冲区，或者mmap()映射的物理地址
    ret = av_file_map(cPath, &buffer, &buffer_size, 0, nullptr);
    if (ret < 0) {
        LOGE("startVideoPlay> av_file_map fail %s", av_err2str(ret));
        goto end;
    }

    /* fill opaque structure used by the AVIOContext read callback */
    bd.ptr = buffer;
    bd.size = buffer_size;

    if (!(ctx = avformat_alloc_context())) {
        ret = AVERROR(ENOMEM);
        LOGE("startVideoPlay> avformat_alloc_context fail %s", av_err2str(ret));
        goto end;
    }

    avio_ctx_buffer = static_cast<uint8_t *>(av_malloc(avio_ctx_buffer_size));
    if (!avio_ctx_buffer) {
        ret = AVERROR(ENOMEM);
        LOGE("av_malloc fail %s", av_err2str(ret));
        goto end;
    }

    avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size,
                                  0, &bd, &read_packet, nullptr, nullptr);
    if (!avio_ctx) {
        ret = AVERROR(ENOMEM);
        LOGE("startVideoPlay> avio_alloc_context fail %s", av_err2str(ret));
        goto end;
    }
//    ctx->pb = avio_ctx;

    //打开文件
//    ret = avformat_open_input(&ctx, nullptr, nullptr, &option);
    cFile = fopen(cPath, "rb");
    if (!cFile) {
        LOGE("startVideoPlay> Cannot open input file ");
    }
    ret = avformat_open_input(&ctx, cPath, nullptr, &option);
    if (ret != 0) {
        LOGE("startVideoPlay> Open input fail %s", av_err2str(ret));
        goto end;
    }

    get_avCodeId(ctx, info);
    if (info == nullptr || info->video_steam_index == -1) {
        goto end;
    }
    params = ctx->streams[info->video_steam_index]->codecpar;
    avCodec = avcodec_find_decoder(params->codec_id);
    LOGI("startVideoPlay> avCodec %s", avCodec->name);

    delayTime = getDelayTime(ctx, info->video_steam_index);
    LOGI("startVideoPlay>getDelayTime %f", delayTime);

    //获取解码器上下文
    codecContext = avcodec_alloc_context3(avCodec);
    //将参数填充到编解码器上下文中
    avcodec_parameters_to_context(codecContext, params);
    //进行解码
    ret = avcodec_open2(codecContext, avCodec, &option);
    if (ret < 0) {
        LOGE("startVideoPlay> avcodec_open2 fail %s", av_err2str(ret));
        goto end;
    }

    //因为YUV数据被封装在了AVPacket中，因此我们需要用AVPacket去获取数据
    pkt = av_packet_alloc();
//    pkt->data = nullptr;
//    pkt->size = 0;
    //创建一个frame去接收解码后的数据
    frame = av_frame_alloc();

    //获取转换器上下文,surfaceView无法直接展示YUV数据，需要转换成RGB
    swsContext = sws_getContext(codecContext->width, codecContext->height,
                                codecContext->pix_fmt,
                                codecContext->width, codecContext->height,
                                AV_PIX_FMT_RGBA, SWS_BILINEAR,
                                0, 0, 0);
    //设置ANativeWindow的绘制缓冲区
    ANativeWindow_setBuffersGeometry(aNativeWindow,
                                     codecContext->width, codecContext->height,
                                     WINDOW_FORMAT_RGBA_8888);

    //计算出转换为RGB所需要的容器的大小
    //接收的容器
    uint8_t *dst_data[4];
    //每一行的首地址（R、G、B、A四行）
    int dst_line_size[4];
    //计算出图片所需的大小
    av_image_alloc(dst_data, dst_line_size,
                   codecContext->width, codecContext->height,
                   AV_PIX_FMT_RGBA, 1);
    isRunning = true;

    //循环读取每一帧压缩数据(h264之类的)，小于0是错误或者文件读完
    while ((ret = av_read_frame(ctx, pkt)) >= 0 && isRunning) {
        curr_size += pkt->size;
        LOGI("startVideoPlay> curr_size=%f, pkt.size=%d", curr_size, pkt->size);
        if (pkt->stream_index == info->video_steam_index) {
            uint8_t *first_window;
            uint8_t *src_data;
            int dst_stride;
            int src_line_size;

            //将取出的数据发送出来
            ret = avcodec_send_packet(codecContext, pkt);
            if (ret < 0) {
                LOGE("startVideoPlay> avcodec_send_packet fail %s,", av_err2str(ret));
                continue;
            }
            LOGI("startVideoPlay> before avcodec_receive_frame");

            //接收解码后的帧数据
            ret = avcodec_receive_frame(codecContext, frame);
            if (ret == AVERROR(EAGAIN)) {
                LOGE("startVideoPlay> avcodec_receive_frame fail EAGAIN");
                //这一帧解码失败，读取下一帧
                continue;
            } else if (ret < 0) {
                LOGE("startVideoPlay> avcodec_receive_frame fail %s,", av_err2str(ret));
                break;
            }
            result = 0;
            LOGI("startVideoPlay> before sws_scale");
            //开始原始YUV数据转RGB

            //将这一帧数据放到RGB容器中
            sws_scale(swsContext, frame->data, frame->linesize,
                      0, frame->height, dst_data, dst_line_size);

            LOGI("startVideoPlay> before ANativeWindow_lock");
            //加锁等待绘制
            ret = ANativeWindow_lock(aNativeWindow, &windowBuffer, 0);
            if (ret < 0) {
                LOGE("startVideoPlay> ANativeWindow_lock fail %s,", av_err2str(ret));
            }

            first_window = static_cast<uint8_t *>(windowBuffer.bits);
            src_data = dst_data[0];

            //拿到每行有多少个RGBA字节
            dst_stride = windowBuffer.stride * 4;
            src_line_size = dst_line_size[0];
            //循环遍历所得到的缓冲区数据
            for (int i = 0; i < windowBuffer.height; i++) {
                //内存拷贝进行渲染
                memcpy(first_window + i * dst_stride, src_data + i * src_line_size, dst_stride);
            }

            //解锁并绘制
            ANativeWindow_unlockAndPost(aNativeWindow);
            long progress = curr_size / size;
            LOGI("startVideoPlay> file_size=%ld, curr=%f, progress = %ld", size, curr_size,
                 progress);
            env->CallVoidMethod(callback, call_invoke, static_cast<jint>(curr_size / size * 100));
            //休眠指定时间
            usleep(delayTime);
        }
        av_packet_unref(pkt);
    }
    LOGI("startVideoPlay> av_read_frame %s", av_err2str(ret));

    end:
    env->ReleaseStringUTFChars(path, cPath);
    avformat_free_context(ctx);
    av_file_unmap(buffer, buffer_size);
    av_dict_free(&option);
    if (info) {
        delete info;
    }
    if (frame) {
        av_frame_free(&frame);
    }
    if (pkt) {
        av_packet_free(&pkt);
    }
    if (aNativeWindow) {
        ANativeWindow_release(aNativeWindow);
    }
    return result;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_nativelib_DecodeTool_stopPlay(JNIEnv *env, jobject thiz) {
    isRunning = false;
}

// https://github.com/android/ndk-samples/blob/main/native-audio/app/src/main/cpp/native-audio-jni.c
//opensl
#include "SLES/OpenSLES.h"
#include "SLES/OpenSLES_Android.h"

//open sl 套路是通过CreateXXX来获取SLObjectItf来管理生命周期，
//通过Realize方法初始化，通过该对象GetInterface获取对应的接口对象来进行操作，
//最后再通过Destroy该对象释放资源

// opensl 发射器对象及接口
static SLObjectItf engineObj = nullptr;
static SLEngineItf engineEngine;

// 输出混合器
static SLObjectItf outputMixObj = nullptr;

// buffer queue player interfaces
static SLObjectItf playerObj = nullptr;
static SLPlayItf playerPlayItf;
// 音频输出的buffer queue 接口
static SLAndroidSimpleBufferQueueItf playBufferQueueInterface;

const int out_channels_count = 2; // 输出声道数，可以根据需要更改
const int out_sample_rate = 48000; // 输出采样率，可以根据需要更改
const int out_sample_size = out_channels_count * 1024; // 输出采样大小，可以根据需要更改

/**
 * 创建OpenSLES
 * @return
 */
int createEngine() {
    //创建audioEngine
    const SLEngineOption engineOptions[1] = {{(SLuint32) SL_ENGINEOPTION_THREADSAFE, (SLuint32) SL_BOOLEAN_TRUE}};
    //第四个参数0表示忽略第五第六个参数
    SLresult sLResult = slCreateEngine(&engineObj, 1,
                                       engineOptions, 0, nullptr, nullptr);
    if (sLResult != SL_RESULT_SUCCESS) {
        LOGE("createEngine> slCreateEngine fail result%d", sLResult);
        return -1;
    }
    //初始化刚才拿到的audioEngine，第二个参数false代表非异步
    sLResult = (*engineObj)->Realize(engineObj, SL_BOOLEAN_FALSE);
    if (sLResult != SL_RESULT_SUCCESS) {
        LOGE("createEngine> Realize fail result%d", sLResult);
        return -1;
    }
    //获取接口对象，第二个参数是接口的ID
    sLResult = (*engineObj)->GetInterface(engineObj, SL_IID_ENGINE, &engineEngine);
    if (sLResult != SL_RESULT_SUCCESS) {
        LOGE("createEngine> GetInterface fail result%d", sLResult);
        return -1;
    }
    return 0;
}

/**
 *  创建输出混合器
 * @return
 */
int createOutputInterface() {
    // 相关参数
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};//环境混合音响的id
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    // 创建音频输出
    SLresult sLResult = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObj, 1, ids, req);
    if (sLResult != SL_RESULT_SUCCESS) {
        LOGE("createOutputInterface> CreateOutputMix fail result%d", sLResult);
        return -1;
    }
    // 初始化
    sLResult = (*outputMixObj)->Realize(outputMixObj, SL_BOOLEAN_FALSE);
    if (sLResult != SL_RESULT_SUCCESS) {
        LOGE("createOutputInterface> Realize fail result%d", sLResult);
        return -1;
    }
    //不需要操作该对象，所以不需要通过GetInterface获取接口
    return 0;
}

int createAudioPlay() {
    //输入源为缓冲队列 buffer queue的参数
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                            1};
    // 设置音频格式
    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM, // 格式类型
            (SLuint32) out_channels_count,//通道数
            SL_SAMPLINGRATE_48,// 采样率
            SL_PCMSAMPLEFORMAT_FIXED_16,// 位宽
            SL_PCMSAMPLEFORMAT_FIXED_16,
            out_channels_count > 1 ? SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT
                                    : SL_SPEAKER_FRONT_CENTER,// 通道屏蔽
            SL_BYTEORDER_LITTLEENDIAN // 字节顺序
    };
    // 输入源
    SLDataSource slDataSource = {&android_queue, &pcm};

    // 输出源
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObj};
    SLDataSink slDataSink = {&outputMix, nullptr};

    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};

    SLresult result = (*engineEngine)->CreateAudioPlayer(engineEngine, &playerObj, &slDataSource,
                                                         &slDataSink, 1, ids, req);
    // todo CreateAudioPlayer 的numInterfaces 2 or 3?
    if (result != SL_RESULT_SUCCESS) {
        LOGE("createAudioPlay> CreateAudioPlayer fail. result=%d", result);
        return -1;
    }

    result = (*playerObj)->Realize(playerObj, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("createAudioPlay>  Realize fail. result=%d", result);
        return -1;
    }
    // 获取播放接口
    result = (*playerObj)->GetInterface(playerObj, SL_IID_PLAY, &playerPlayItf);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("createAudioPlay> GetInterface SL_IID_PLAY fail. result=%d", result);
        return -1;
    }
    // 获取音频输出的BufferQueue接口(缓冲队列)
    result = (*playerObj)->GetInterface(playerObj, SL_IID_BUFFERQUEUE, &playBufferQueueInterface);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("createAudioPlay> GetInterface SL_IID_BUFFERQUEUE fail. result=%d", result);
        return -1;
    }

    // 注册输出回调 todo 不注册缓冲回调
//    result = (*playBufferQueueInterface)->RegisterCallback(playBufferQueueInterface,
//                                                           AudioPlayerCallback, nullptr);
//    if (result != SL_RESULT_SUCCESS) {
//        LOGE("createAudioPlay> RegisterCallback fail. result=%d", result);
//        return -1;
//    }

    //设置播放状态
    result = (*playerPlayItf)->SetPlayState(playerPlayItf, SL_PLAYSTATE_PLAYING);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("createAudioPlay> SetPlayState fail. result=%d", result);
        return -1;
    }
    return 0;
}


extern "C"
JNIEXPORT jint JNICALL
/**
 * 根据指定的视频文件，解码音频帧，通过opensl播放，进度只是简单的数据进度
 * @param env
 * @param thiz
 * @param path
 * @param file_size
 * @param callback
 * @return
 */
Java_com_example_nativelib_DecodeTool_startPlayAudio(JNIEnv *env, jobject thiz, jstring path,
                                                     jlong file_size, jobject callback) {
    int result = -1;
    int ret = -1;
    int audio_steam_index = -1;
    //用于解析文件中音视频流的顶层上下文
    AVFormatContext *ctx = avformat_alloc_context();
    //编解码器参数信息
    AVCodecParameters *params = nullptr;
    //编解码器
    const AVCodec *avCodec;
    //编解码所需的上下文
    AVCodecContext *codecContext;
    //存储压缩编码数据的结构体
    AVPacket *pkt = nullptr;
    //存储一帧原始数据的结构，未编码的如YUV、RGB、PCM
    AVFrame *frame = nullptr;
    SwrContext *swrContext = nullptr;
    AVChannelLayout outputChannel;
    int dataSize = 0;
    uint8_t *audioBuffer = nullptr;



    FILE *pFile = nullptr;
    //IO输入/输出上下文
    AVIOContext *avio_ctx = nullptr;
    uint8_t *buffer = nullptr, *avio_ctx_buffer = nullptr;
    size_t buffer_size, avio_ctx_buffer_size = 4096;
    struct buffer_data bd = {0};

    //java string 转 C++能用的字符串
    const char *cPath = env->GetStringUTFChars(path, JNI_FALSE);
    //文件大小
    const long size = file_size;
    //当前读取到的数据量
    double curr_size = 0;

    jclass callbackClass = env->FindClass("com/example/nativelib/DecodeTool$ProgressCallback");
    jmethodID call_invoke = env->GetMethodID(callbackClass, "invoke", "(I)V");

    //初始化网络模块，可以播放url
//    avformat_network_init();

    //配置超时时间？
    AVDictionary *option = nullptr;
    av_dict_set(&option, "timeout", "3000000", 0);

    //读取指定的文件，并将内容放入新分配的缓冲区，或者mmap()映射的物理地址
    ret = av_file_map(cPath, &buffer, &buffer_size, 0, nullptr);
    if (ret < 0) {
        LOGE("startPlayAudio> av_file_map fail %s", av_err2str(ret));
        goto end;
    }

    /* fill opaque structure used by the AVIOContext read callback */
    bd.ptr = buffer;
    bd.size = buffer_size;

    if (!(ctx = avformat_alloc_context())) {
        ret = AVERROR(ENOMEM);
        LOGE("startPlayAudio> avformat_alloc_context fail %s", av_err2str(ret));
        goto end;
    }

    avio_ctx_buffer = static_cast<uint8_t *>(av_malloc(avio_ctx_buffer_size));
    if (!avio_ctx_buffer) {
        ret = AVERROR(ENOMEM);
        LOGE("startPlayAudio> av_malloc fail %s", av_err2str(ret));
        goto end;
    }

    // 自定义pb
    pFile = fopen(cPath, "rb");
    if (!pFile) {
        LOGE("startPlayAudio> Cannot open input file ");
    }
    
    avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size,
                                  0, &bd, &read_packet, nullptr, nullptr);
    if (!avio_ctx) {
        ret = AVERROR(ENOMEM);
        LOGE("startPlayAudio> avio_alloc_context fail %s", av_err2str(ret));
        goto end;
    }

    //打开文件
    ret = avformat_open_input(&ctx, cPath, nullptr, &option);
//    ret = avformat_open_input(&ctx, nullptr, nullptr, &option);
    if (ret != 0) {
        LOGE("startPlayAudio> Open input fail %s", av_err2str(ret));
        goto end;
    }

    //找到对应的流数据
    ret = avformat_find_stream_info(ctx, nullptr);
    if (ret != 0) {
        LOGE("startPlayAudio> Find stream info fail %s", av_err2str(ret));
        goto end;
    }

    ret = av_find_best_stream(ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &avCodec, 0);
    if (ret < 0) {
        LOGE("startPlayAudio> av_find_best_stream AVMEDIA_TYPE_AUDIO fail %s", av_err2str(ret));
        goto end;
    }
    audio_steam_index = ret;

    codecContext = avcodec_alloc_context3(avCodec);
    params = ctx->streams[audio_steam_index]->codecpar;
    avcodec_parameters_to_context(codecContext, params);

    ret = avcodec_open2(codecContext, avCodec, nullptr);
    if (ret < 0) {
        LOGE("startPlayAudio> avcodec_open2 fail %s", av_err2str(ret));
        goto end;
    }

    LOGI("startPlayAudio> before init swrContext");
    //重采样

    // 设置参数
    outputChannel = AVChannelLayout();
    av_channel_layout_default(&outputChannel, out_channels_count);
    ret = swr_alloc_set_opts2(&swrContext,
                              &outputChannel,
                              AV_SAMPLE_FMT_S16,
                              out_sample_rate,
                              &(params->ch_layout),
                              codecContext->sample_fmt,
                              params->sample_rate,
                              0, nullptr);
    ret = swr_init(swrContext);

    if (ret < 0) {
        LOGE("startPlayAudio> swr_init fail %s", av_err2str(ret));
        goto end;
    }

    /* 分配空间 */
    audioBuffer = (uint8_t*)calloc(codecContext->ch_layout.nb_channels,
                                     sizeof(*audioBuffer));
    av_samples_alloc(&audioBuffer, nullptr,
                     codecContext->ch_layout.nb_channels,
                     codecContext->frame_size,
                     codecContext->sample_fmt, 0);

    //初始化解析的原始包和解码包
    pkt = av_packet_alloc();
    frame = av_frame_alloc();

    //初始化OpenSL
    if (createEngine() < 0) {
        goto end;
    }
    //初始化音频输出混合器
    if (createOutputInterface() < 0) {
        goto end;
    }
    //初始化播放
    if (createAudioPlay() < 0) {
        goto end;
    }
    isRunning = true;
    while ((ret = av_read_frame(ctx, pkt)) >= 0 && isRunning) {
        curr_size += pkt->size;
        long progress = curr_size / size;
        LOGI("startVideoPlay> file_size=%ld, curr=%f, progress = %ld", size, curr_size,
             progress);
        env->CallVoidMethod(callback, call_invoke, static_cast<jint>(curr_size / size * 100));
        if (pkt->stream_index == audio_steam_index) {
            ret = avcodec_send_packet(codecContext, pkt);
            if (ret < 0) {
                LOGE("startPlayAudio> avcodec_send_packet fail %s", av_err2str(ret));
                goto end;
            }
            ret = avcodec_receive_frame(codecContext, frame);
            if (ret == AVERROR(EAGAIN)) {
                LOGE("startPlayAudio> avcodec_receive_frame fail EAGAIN");
                //这一帧解码失败，读取下一帧
                continue;
            } else if (ret < 0) {
                LOGE("startPlayAudio> avcodec_receive_frame fail %s", av_err2str(ret));
                goto end;
            }

            // 将音频帧数据转换为适合OpenSL ES播放器的格式

            ret = swr_convert(swrContext, &audioBuffer, out_sample_size,
                        (const uint8_t **) frame->data,
                        frame->nb_samples);
            if (ret < 0) {
                LOGE("startPlayAudio> swr_convert fail %s", av_err2str(ret));
            }
            LOGI("startPlayAudio> swr_convert end");
            dataSize = av_samples_get_buffer_size(nullptr, out_channels_count, frame->nb_samples,AV_SAMPLE_FMT_S16, 1);
            LOGI("startPlayAudio> av_samples_get_buffer_size dataSize = %d", dataSize);
//            AudioPlayerCallback(playBufferQueueInterface, AudioContext(audioBuffer,dataSize));

            // 将音频数据添加到OpenSL ES播放器的缓冲区队列
            SLresult slResult = (*playBufferQueueInterface)->Enqueue(playBufferQueueInterface, audioBuffer, dataSize);
            if (slResult != SL_RESULT_SUCCESS) {
                LOGE("startPlayAudio> Enqueue fail %d", slResult);
            }
            LOGI("startPlayAudio> Enqueue end");

//            usleep(delayTime);

            //计算公式 acc  1024 * 1000 / 48000 = 21.34
            //计算公式 mp3  1152 * 1000 / 48000 = 24
            usleep(1000 * 21.34);

        } else {
            LOGI("startPlayAudio> av_read_frame filter by stream_index != audio_stream_index");
        }
        av_packet_unref(pkt);
    }
    LOGI("startPlayAudio> end");

    if (playerPlayItf) {
        (*playerPlayItf)->SetPlayState(playerPlayItf, SL_PLAYSTATE_STOPPED);
    }

    end:
    avformat_free_context(ctx);
    avcodec_free_context(&codecContext);
    swr_free(&swrContext);
    av_packet_free(&pkt);
    av_frame_free(&frame);
    if (audioBuffer) {
        free(audioBuffer);
    }
    //释放资源
    if (playerObj) {
        (*playerObj)->Destroy(playerObj);
    }
    if (outputMixObj) {
        (*outputMixObj)->Destroy(outputMixObj);
    }
    if (engineObj) {
        (*engineObj)->Destroy(engineObj);
    }

    return result;
}