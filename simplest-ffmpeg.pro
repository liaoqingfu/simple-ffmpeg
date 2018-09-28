TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    01-ffmpeg_helloworld.cpp \
    03-ffmpeg_audio_decoder.cpp \
    02-ffmpeg_demuxing.cpp \
    04-ffmpeg_video_decoder.cpp \
    06-ffmpeg_audio_encoder.cpp \
    05-ffmpeg_sdl2_player.cpp \
    07-ffmpeg_video_encoder.cpp \
    08-ffmpeg_muxing.cpp
win32 {
INCLUDEPATH += $$PWD/ffmpeg-4.0.2-win32-dev/include
INCLUDEPATH += $$PWD/SDL2/include

LIBS += $$PWD/ffmpeg-4.0.2-win32-dev/lib/avformat.lib   \
        $$PWD/ffmpeg-4.0.2-win32-dev/lib/avcodec.lib    \
        $$PWD/ffmpeg-4.0.2-win32-dev/lib/avdevice.lib   \
        $$PWD/ffmpeg-4.0.2-win32-dev/lib/avfilter.lib   \
        $$PWD/ffmpeg-4.0.2-win32-dev/lib/avutil.lib     \
        $$PWD/ffmpeg-4.0.2-win32-dev/lib/postproc.lib   \
        $$PWD/ffmpeg-4.0.2-win32-dev/lib/swresample.lib \
        $$PWD/ffmpeg-4.0.2-win32-dev/lib/swscale.lib    \
        $$PWD/SDL2/lib/x86/SDL2.lib
}

HEADERS += \
    ffmpeg_muxing.h
