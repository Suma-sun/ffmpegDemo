该Demo采用2024 ffmpeg官网最新的7.0.1源码编译的so库进行开发，线上的文章大多都是4+的版本，本demo用7开发，可参考的内容不多效率不高<p>

1.已完成ffmpeg解码mp4，保存首帧为jpg<p>
2.已完成ffmpeg解码mp4视频帧，转为RGB通过ANativeWindow渲染（SurfaceView）<p>
3.已完成ffmpeg解码mp4音频帧，重采样通过OpenSL 播放<p>
4.待开发ffmpeg解码，opengl渲染视频，opensl播放音频，音视频同步<p>
<p>
<p>
以下是在Window编译ffmpeg的教程<p>
1.首先下载msys2,打开MSYS2 MINGW64<p>
2.更新获取所需的工具<p>
<li> pacman -Sy </li>
<li> pacman -S mingw-w64-x86_64-toolchain </li>
<li> pacman -S git </li>
<li> pacman -S make </li>
<li> pacman -S automake  </li>
<li> pacman -S autoconf </li>
<li> pacman -S perl </li>
<li> pacman -S libtool </li>
<li> pacman -S mingw-w64-x86_64-cmake </li>
<li> pacman -S pkg-config </li>
<li> pacman -S yasm </li>
<li> pacman -S diffutils </li>
3.将根目录的build_ffmpeg拷贝到msys2目录中的home中的user_name目录下<p>
4.将ffmpeg源码考到home中的user_name目录下，user_name目录下创建一个输出的文件夹<p>
5.修改build_ffmpeg.sh文件中的对应路径名将user_name改为实际的user_name，将NDK目录改为实际的目录
6.cd到user_name目录运行build_ffmpeg等待编译完成即可(如果失败根据提示配置工具到环境变量)






 

