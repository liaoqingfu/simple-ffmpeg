#include <iostream>

using namespace std;

#ifdef __cplusplus
extern "C"
{
// ����ffmpegͷ�ļ�
#include "libavutil/avutil.h"
}
#endif

int ffmpeg_01_helloworld()
{
    cout << "Hello FFMPEG, av_version_info is  " <<  av_version_info() << endl;

    cout << "\navutil_configuration is \n" << avutil_configuration() << endl;

    return 0;
}
