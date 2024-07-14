# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.
cmake_minimum_required(VERSION 3.22.1)

project("nativelib")

#设置源文件目录
set(FFMPEG_LIB_DIR "${CMAKE_SOURCE_DIR}/../jniLibs/${CMAKE_ANDROID_ARCH_ABI}")
set(FFMPEG_HEAD_IDR "${CMAKE_SOURCE_DIR}/include")

#设置头文件目录
include_directories(${FFMPEG_HEAD_IDR})

#查找jni目录下的所有cpp文件
file(GLOB NATIVE_LIB_SOURCE
        "${CMAKE_SOURCE_DIR}/jni/*.cpp"
        "${CMAKE_SOURCE_DIR}/jni/decode/*.cpp"
        )

#添加共享目标
add_library(native-lib
        SHARED
        ${NATIVE_LIB_SOURCE})

#导入三方库
add_library(avcodec SHARED IMPORTED)
#设置三方库的名称、位置
set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION "${FFMPEG_LIB_DIR}/libavcodec.so")

add_library(avdevice SHARED IMPORTED)
set_target_properties(avdevice PROPERTIES IMPORTED_LOCATION "${FFMPEG_LIB_DIR}/libavdevice.so")

add_library(avfilter SHARED IMPORTED)
set_target_properties(avfilter PROPERTIES IMPORTED_LOCATION "${FFMPEG_LIB_DIR}/libavfilter.so")

add_library(avformat SHARED IMPORTED)
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION "${FFMPEG_LIB_DIR}/libavformat.so")

add_library(avutil SHARED IMPORTED)
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION "${FFMPEG_LIB_DIR}/libavutil.so")

add_library(swresample SHARED IMPORTED)
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION "${FFMPEG_LIB_DIR}/libswresample.so")

add_library(swscale SHARED IMPORTED)
set_target_properties(swscale PROPERTIES IMPORTED_LOCATION "${FFMPEG_LIB_DIR}/libswscale.so")

# 指定FFmpeg头文件目录
include_directories("${CMAKE_SOURCE_DIR}/include")

#链接库
target_link_libraries(native-lib
        # List libraries link to the target library
        avcodec
        avdevice
        avfilter
        avformat
        avutil
        swresample
        swscale
        android
        log
        # 链接OpenSLES库
        OpenSLES
#        ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libavcodec.so
#        ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libavdevice.so
#        ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libavfilter.so
#        ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libavformat.so
#        ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libavutil.so
#        ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libswresample.so
#        ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libswscale.so
        )
