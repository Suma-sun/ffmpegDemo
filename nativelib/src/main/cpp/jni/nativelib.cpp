#include <jni.h>
#include <string>
#include "stdio.h"
#include "android/log.h"

extern "C" {
#include "libavcodec/version.h"
#include "libavcodec/avcodec.h"
#include "libavformat/version.h"
#include "libavutil/version.h"
#include "libavfilter/version.h"
#include "libswresample/version.h"
#include "libswscale/version.h"
}

extern "C" JNIEXPORT jstring  JNICALL
Java_com_example_nativelib_NativeLib_stringFromJNI(JNIEnv *env, jobject thiz) {
    const char* hello = "Hello JNI";
    return env->NewStringUTF(hello);
}


extern "C" JNIEXPORT jstring  JNICALL
Java_com_example_nativelib_NativeLib_ffmpegVersion(JNIEnv *env, jobject thiz) {
    char strBuffer[1024 * 4] = {0};
    strcat(strBuffer, "libavcodec: ");
    strcat(strBuffer, AV_STRINGIFY(LIBAVCODEC_VERSION));
    strcat(strBuffer, "\nlibavdevice: ");
    strcat(strBuffer, AV_STRINGIFY(LIBAVDEVICE_VERSION));
    strcat(strBuffer, "\nlibavfilter: ");
    strcat(strBuffer, AV_STRINGIFY(LIBAVFILTER_VERSION));
    strcat(strBuffer, "\nlibavformat: ");
    strcat(strBuffer, AV_STRINGIFY(LIBAVFORMAT_VERSION));
    strcat(strBuffer, "\nlibavutil: ");
    strcat(strBuffer, AV_STRINGIFY(LIBAVUTIL_VERSION));
    strcat(strBuffer, "\nlibpostproc: ");
    strcat(strBuffer, AV_STRINGIFY(LIBPOSTPROC_VERSION));
    strcat(strBuffer, "\nlibswresample: ");
    strcat(strBuffer, AV_STRINGIFY(LIBSWRESAMPLE_VERSION));
    strcat(strBuffer, "\nlibswscale: ");
    strcat(strBuffer, AV_STRINGIFY(LIBSWSCALE_VERSION));
    return env->NewStringUTF(strBuffer);
}

