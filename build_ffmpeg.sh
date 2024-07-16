#!/bin/bash
set -e
SRC_DIR=/home/user_name/ffmpeg/ffmpeg-7.0.1
DST_DIR=/home/user_name/ffmpeg/ffmpeg-7.0.1_out
NDK=C:/Users/user_name/AppData/Local/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/windows-x86_64
API=21 #支持最低的api等级
 
cd $SRC_DIR
 
function build_onearch
{
	echo ">> configure $CPU"
	./configure --enable-cross-compile --target-os=android  \
	--prefix=$DST_DIR/$ANDROID_ABI --cross-prefix=$CROSS_PREFIX \
	--cc=clang --cxx=clang++ --strip=llvm-strip \
	--extra-cflags="--target=aarch64-linux-android21"  \
	--extra-ldflags="--target=aarch64-linux-android21" \
	--arch=$ARCH  --cpu=$CPU  --sysroot=$NDK/sysroot \
	--enable-shared --enable-static \
	--enable-small \
	--enable-asm --enable-neon \
	--enable-jni --enable-mediacodec \
	--disable-ffmpeg \
	--disable-ffprobe \
	--disable-ffplay \
	--disable-programs \
	--disable-symver \
	--disable-doc \
	--disable-htmlpages \
	--disable-manpages \
	--disable-podpages \
	--disable-txtpages \
	--enable-avformat \
	--enable-avcodec \
	--enable-avutil \
	   
	echo "<< configure $CPU"
	make clean
	make -j16 #这个是线程数，根据电脑配置修改，我这时32线程用了一半
	make install
}
 
ANDROID_ABI=arm64-v8a
ARCH=arm64
CPU=armv8-a
CROSS_PREFIX=$NDK/bin/aarch64-linux-android-
CLANG_PREFIX=$NDK/bin/aarch64-linux-android$API-clang
build_onearch
 
ANDROID_ABI=armeabi-v7a
ARCH=arm
CPU=armv7-a
CROSS_PREFIX=$NDK/bin/arm-linux-androideabi-
CLANG_PREFIX=$NDK/bin/armv7a-linux-androideabi$API-clang
build_onearch