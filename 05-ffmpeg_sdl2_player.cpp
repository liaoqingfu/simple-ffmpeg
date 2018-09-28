/**********************************************
 *Date:  2018��9��21��
 *Description: ����Ƶ��������ֻ�ܲ�����Ƶ��
 **********************************************/

#include <iostream>

using namespace std;


#ifdef __cplusplus
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "SDL2/include/SDL.h"
#include "libavutil/imgutils.h"
}
#endif


bool g_quit  = false;     // ��g_quit == trueʱ�����˳�
bool g_pause =  false;    // ��g_pause == trueʱ������ͣ
int  g_frame_rate = 25;    // Ĭ��

const char *s_picture_type[] =
{
    "AV_PICTURE_TYPE_NONE", ///< Undefined
    "AV_PICTURE_TYPE_I",     ///< Intra
    "AV_PICTURE_TYPE_P",     ///< Predicted
    "AV_PICTURE_TYPE_B",     ///< Bi-dir predicted
    "AV_PICTURE_TYPE_S",     ///< S(GMC)-VOP MPEG-4
    "AV_PICTURE_TYPE_SI",    ///< Switching Intra
    "AV_PICTURE_TYPE_SP",    ///< Switching Predicted
    "AV_PICTURE_TYPE_BI"   ///< BI type
};

// �Զ���SDL�¼�
#define FRAME_REFRESH_EVENT (SDL_USEREVENT+1)
static int frameRefreshThread(void *arg)
{
    while(!g_quit)
    {
        if(!g_pause)
        {
            SDL_Event event;
            event.type = FRAME_REFRESH_EVENT;
            SDL_PushEvent(&event);
        }
        if(g_frame_rate > 0)
            SDL_Delay(1000/g_frame_rate/8);     // ������Ʋ����ٶȣ���ʱ����Ϊ��������ʹ���˱��١�
        else
            SDL_Delay(40);
    }
    return 0;
}

static char err_buf[128] = {0};
static char* av_get_err(int errnum)
{
    av_strerror(errnum, err_buf, 128);
    return err_buf;
}


int ffmpeg_05_sdl2_player(int argc, char *argv[])
{
    AVFormatContext *pFormatCtx = NULL;
    int             i, videoindex, audioindex, titleindex;
    AVCodecContext  *pCodecCtx;
    AVCodec         *pCodec;
    AVFrame         *pFrame;
    AVStream	    *avStream;
    AVPacket        *packet;
    int             ret;
    char filePath[256] = {0};
    bool isFileEnd = false; // �ļ��Ƿ��ȡ����

    // SDL �ⲿ����Ҫ����ʾ��أ������Ȳ��ù�ע
    int screen_w, screen_h = 0;
    SDL_Window   *screen;
    SDL_Renderer *sdlRenderer;
    SDL_Texture  *sdlTexture;
    SDL_Rect     sdlRect;
    SDL_Event    event;
    SDL_Thread   *refreshThread;

    //strcpy(filePath, "axin.flv");
    strcpy(filePath, "E:\\���ԿƼ�\\�γ�����\\C++������\\Martin 2018-04-08 Linux ����������-����붮��Epoll�淨.mp4");

    //strcpy(filePath, "source.200kbps.768x320.flv");
    // ����⸴�������ڴ棬ʹ��avformat_close_input�ͷ�
    pFormatCtx = avformat_alloc_context();
    if (!pFormatCtx)
    {
        printf("[error] Could not allocate context.\n");
        return -1;
    }

    // ����url����������ѡ��ƥ��Ľ⸴����
    ret = avformat_open_input(&pFormatCtx, filePath, NULL, NULL);
    if(ret != 0)
    {
        printf("[error]avformat_open_input: %s\n", av_get_err(ret));
        return -1;
    }

    // ��ȡý���ļ��Ĳ������ݰ��Ի�ȡ������Ϣ
    ret = avformat_find_stream_info(pFormatCtx, NULL);
    if(ret < 0)
    {
        printf("[error]avformat_find_stream_info: %s\n", av_get_err(ret));

        avformat_close_input(&pFormatCtx);
        return -1;
    }

    // ���ҳ��ĸ�������video/audio/subtitles
    videoindex = audioindex = titleindex = -1;
//    for(i = 0; i < pFormatCtx->nb_streams; i++)   // �ϰ汾�ķ�ʽ
//    {
//        avStream = pFormatCtx->streams[i];
//        switch(avStream->codecpar->codec_type)
//        {
//        case AVMEDIA_TYPE_VIDEO:
//            videoindex = i;
//            // ����֡�ʣ��Կ�����Ƶˢ�¼��
//            g_frame_rate = avStream->avg_frame_rate.num / avStream->avg_frame_rate.den;
//            break;
//        case AVMEDIA_TYPE_AUDIO:
//            audioindex = i;
//            break;
//        case AVMEDIA_TYPE_SUBTITLE:
//            titleindex = i;
//            break;
//        }
//    }
    // �Ƽ��ķ�ʽ
    videoindex = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if(videoindex == -1)
    {
        printf("Didn't find a video stream.\n");
        avformat_close_input(&pFormatCtx);
        return -1;
    }
    avStream =  pFormatCtx->streams[videoindex];
    g_frame_rate = avStream->avg_frame_rate.num / avStream->avg_frame_rate.den;

    // ����������������ڴ棬ʹ��avcodec_free_context���ͷ�
    pCodecCtx = avcodec_alloc_context3(NULL);
    if(!pCodecCtx)
    {
        printf("[error]avcodec_alloc_context3() fail\n");
        avformat_close_input(&pFormatCtx);
        return -1;
    }
    // �������еı��������ϢAVCodecParameters������AVCodecContex
    ret = avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoindex]->codecpar);
    if(ret < 0) {
        // printf("[error]avcodec_parameters_to_context: %s\n", av_err2str(ret));
        avcodec_free_context(&pCodecCtx);
        avformat_close_input(&pFormatCtx);
        return -1;
    }
    // ���ҽ�����
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec == NULL) {
        printf("Video Codec not found.\n");
        avcodec_free_context(&pCodecCtx);
        avformat_close_input(&pFormatCtx);
        return -1;
    }
    // �򿪽�����
    ret = avcodec_open2(pCodecCtx, pCodec, NULL);
    if(ret < 0) {
        //printf("[error]avcodec_open2: %s\n", av_err2str(ret));
        avcodec_free_context(&pCodecCtx);
        avformat_close_input(&pFormatCtx);
        return -1;
    }

    // ����ļ���Ϣ
    printf("-------------File Information-------------\n");
    av_dump_format(pFormatCtx, 0, filePath, 0);
    printf("------------------------------------------\n");

    // ����һ��Frame
    pFrame = av_frame_alloc();   /* av_frame_free(&pFrame); */

    // ��ʼSDL������, �Ȳ������
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0)
    {
        printf("Could not initialize SDL - %s\n", SDL_GetError());
        av_frame_free(&pFrame);
        avcodec_free_context(&pCodecCtx);
        avformat_close_input(&pFormatCtx);
        return -1;
    }

    screen_w = pCodecCtx->width;
    screen_h = pCodecCtx->height;
    screen   = SDL_CreateWindow("Simple Player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                screen_w, screen_h, SDL_WINDOW_RESIZABLE);  //SDL_WINDOW_OPENGL
    if(!screen)
    {
        printf("SDL: could not create window - %s\n", SDL_GetError());

        av_frame_free(&pFrame);
        avcodec_free_context(&pCodecCtx);
        avformat_close_input(&pFormatCtx);
        return -1;
    }

    sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
    sdlTexture  = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,
                                    pCodecCtx->width, pCodecCtx->height);
    sdlRect.x = 0;
    sdlRect.y = 0;
    sdlRect.w = screen_w;
    sdlRect.h = screen_h;
    // ��ʼSDL���������

    // �������ݰ�
    packet = av_packet_alloc();
    av_init_packet(packet);

    // ����һ��ˢ���̣߳�ˢ���̶߳��巢��ˢ���źŸ����̣߳��������߳̽������ʾ����
    refreshThread = SDL_CreateThread(frameRefreshThread, NULL, NULL);

    while(!g_quit)      // ��ѭ��
    {
        if(SDL_WaitEvent(&event) != 1)
            continue;

        switch(event.type)
        {
        case SDL_KEYDOWN:
            if(event.key.keysym.sym == SDLK_ESCAPE)
                g_quit = true;
            if(event.key.keysym.sym == SDLK_SPACE)
                g_pause = !g_pause;
            break;

        case SDL_QUIT:    /* Window is closed */
            g_quit = true;
            break;

        case FRAME_REFRESH_EVENT:
read_packet_again:                  // �����ȡ�����Ƿ�video packet�������¶�ȡ
            if((ret = av_read_frame(pFormatCtx, packet)) < 0)
            {
                // û�и�����ɶ�
                isFileEnd = true;
                printf("read file end\n");
            }

            if(ret == 0 && packet->stream_index != videoindex)
            {
                av_packet_unref(packet);        // �ͷ��ڴ�
                goto read_packet_again;
            }

            if(ret == 0)
            {
                // ����Ҫ���������
                ret = avcodec_send_packet(pCodecCtx, packet);
                if( ret != 0 )
                {
                    av_packet_unref(packet);
                    break;
                }
            }
            else
            {
                // ˢ�հ���ȥ
                printf("flush packet\n");
                av_packet_unref(packet);        // �����ռ���ڴ����ͷ�
                ret = avcodec_send_packet(pCodecCtx, packet);
            }

            // ��ȡ������֡
            do
            {
                /*     0:                 success, a frame was returned
                *      AVERROR(EAGAIN):   output is not available in this state - user must try
                *                         to send new input
                *      AVERROR_EOF:       the decoder has been fully flushed, and there will be
                *                         no more output frames
                *      AVERROR(EINVAL):   codec not opened, or it is an encoder
                *      other negative values: legitimate decoding errors
                */
                ret = avcodec_receive_frame(pCodecCtx, pFrame);
                if(ret == 0)
                {  // �ɹ���ȡ��������֡
                    printf("frame type = %s\n",s_picture_type[pFrame->pict_type]);
                    SDL_UpdateYUVTexture(sdlTexture, &sdlRect,
                                         pFrame->data[0], pFrame->linesize[0],
                            pFrame->data[1], pFrame->linesize[1],
                            pFrame->data[2], pFrame->linesize[2]);
                    SDL_RenderClear(sdlRenderer);
                    SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);
                    SDL_RenderPresent(sdlRenderer);
                }
                else if(ret == AVERROR(EAGAIN))
                {
                    // û��֡�ɶ����ȴ���һ��������ٶ�ȡ
                    break;
                }
                else if(ret == AVERROR_EOF)
                {
                    // ����������֡���Ѿ�����ȡ
                    avcodec_flush_buffers(pCodecCtx);
                    printf("avcodec_flush_buffers\n");
                    g_quit = true;          // �˳�����
                    break;
                }
                else if(ret < 0)
                {
                    printf("if(ret < 0)\n");
                    break;
                }
            } while(ret != AVERROR(EAGAIN));
//            ����AVPacket buf���ü���
//            if(packet->buf)        // ��ӡreferenc-counted�����뱣֤���������Чָ��
//                printf("ref_count(packet) = %d\n", av_buffer_get_ref_count(packet->buf));
            av_packet_unref(packet);        // �ͷ��ڴ�

            break;
        default:
            //printf("unknow sdl event.......... event.type = %x\n", event.type);
            break;
        } /* End of switch */
    } /* End of while(!g_quit) */


    SDL_DestroyRenderer(sdlRenderer);
    SDL_Quit();
    av_packet_free(&packet);
    av_frame_free(&pFrame);
    avcodec_close(pCodecCtx);
    avcodec_free_context(&pCodecCtx);
    avformat_close_input(&pFormatCtx);

    printf("�������\n");

    //system("pause");
    return 0;
}
