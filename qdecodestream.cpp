#include "qdecodestream.h"
#include "unistd.h"
#define IMG_WIDTH  1920
#define IMG_HEIGHT  1080
#include "sys/syscall.h"
#include "sys/time.h"
#include "stdio.h"
#include "pthread.h"
/*********************************************
 * 解码类构告函数
 * 输入:
 *    int width:视频的宽度
 *    int height:视频的高度
 *********************************************
*/
QDecodeStream::QDecodeStream(int width,int height)
{
    view_param_t *video = new view_param_t;  //视频数据结构
    video->image.width = width;  //视频的宽度
    video->image.height = height;  //视频的高度
    //video->callback = pClsread->callback;  //解码完成的处理，其回调函数指针传递
    play_win = video;
    first = 0;
    debug_enable = 0;
    get_rawdata = new GetRAWData();  //取得原始数据类实例，该类绑定数据缓冲区，以及增加销费者
}
/*******************************************
 * 绑定滑窗，即绑定生产者
 * 输入:
 *    QSlidingWindow *slidingwnd : 生产者
 *******************************************
*/
void QDecodeStream::bound(QSlidingWindow *slidingwnd)
{
    get_rawdata->bound(slidingwnd);
    sliding_window = slidingwnd;
}
/*********************************************
 * 设定销费者编号(要求唯一编号)
 * 输入:
 *    int userno:用户编号
 *********************************************
*/
int QDecodeStream::set_user(int userno)
{
    user_no = userno;
    consume = get_rawdata->append_user(userno);

    ipcam_decode_init(A5S66,H264);
    decode_h264.codecCtx->width = play_win->image.width;
    decode_h264.codecCtx->height = play_win->image.height;
}
/*********************************************
 * 启动解码线程
 *********************************************
*/
void QDecodeStream::start_decode()
{
    pthread_attr_t attr;
    pthread_attr_init (&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    int id = pthread_create(&decode_pthread_id,&attr,getraw_run,this);
    pthread_attr_destroy (&attr);

}
/***************************************************
 * 设置解码完成的回调函数
 ***************************************************
*/
void QDecodeStream::set_rtn_callback(void (*callback)(char *))
{
    call_fun = callback;
}
/*****************************************************
 * 设定调试模式
 *    输入:
 *       int debug_mode:调试模式
 *****************************************************
*/
void QDecodeStream::open_debug()
{
    debug_enable  = 1;
}
/*===============================================================*/
/*
 *            private function
 *
*/
/***************************************************
 * 解码线程
 ***************************************************
*/
void *QDecodeStream::getraw_run(void *ptr)
{
    QDecodeStream *pthis = (QDecodeStream *)ptr;
    int consume_no = pthis->user_no;
    char *buffer;
    buffer = (char *)malloc(sizeof(char)*1024*1024);
    pthis->consume->read_init();
    printf("decode  pthread start! pid:%d \n",syscall(SYS_gettid));
    //将该线程进行CPU绑定
    //taskset -p mask pid: 0x3c = 0b00111100  即绑定cpu2 cpu3 cpu4 cpu5
    char *cmd_system = (char *)malloc(sizeof(char)*1024);
    sprintf(cmd_system,"/bin/taskset -p 0x3c %d",syscall(SYS_gettid));
    system(cmd_system);
    free(cmd_system);
    while(1)
    {
        int len = 0;
        if(pthis->consume == NULL)
        {
            printf("current consume %d is not exist. \n",consume_no);
            return NULL;
        }

        struct   timeval   start_time,stop_time,diff;
        if(pthis->debug_enable == 1)
        {
            gettimeofday(&start_time,0);
        }
        len = pthis->consume->read_data_to_buffer(buffer);
        if(pthis->debug_enable == 1)
        {
            gettimeofday(&stop_time,0);
            pthis->timeval_subtract(&diff,&start_time,&stop_time);
            printf("read data time:%d uS\n",diff.tv_usec);
        }
        if(len < 0)
        {
            int count = pthis->sliding_window->consume_linklist_delete(consume_no);
            pthis->sliding_window->consume_linklist_isEmpty();

            printf("delete no:%d,cur count:%d \n",consume_no,count);
            return NULL;
        }

        pthread_testcancel();
        if(pthis->debug_enable == 1)
        {
            gettimeofday(&start_time,0);
        }
        pthis->decode_main_frame((uint8_t *)buffer,len);
        if(pthis->debug_enable == 1)
        {
            gettimeofday(&stop_time,0);
            pthis->timeval_subtract(&diff,&start_time,&stop_time);
            printf("decode time:%d uS\n",diff.tv_usec);
        }

#if defined(Q_OS_WIN32)
        usleep(1000);
#elif defined(Q_OS_MACX)
        pthread_yield_np();
#elif defined(Q_OS_UNIX)
        pthread_yield();
#endif
    }

}


/***************************************************
 *int decode_main_frame(uint8_t *data, int size)
 *  解码处理主函数
 ***************************************************
 */
int QDecodeStream::decode_main_frame(uint8_t *data, int size)
/*
 * data: 待解码数据
 * size: 数据大小
*/
{
    int got_picture=0;
    int iRet;
    AVPicture pFrameRGB;
    av_init_packet(&decode_h264.avp);
    decode_h264.avp.data=data;
    decode_h264.avp.size=size;

    iRet = avcodec_decode_video2(decode_h264.codecCtx, \
                                 decode_h264.frame, \
                                 &got_picture, \
                                 &decode_h264.avp);

#if 1
    if((first == 0)  &&(got_picture))
    {
        decode_h264.img_convert_ctx = sws_getContext(decode_h264.codecCtx->width, \
                                                        decode_h264.codecCtx->height, \
                                                        decode_h264.codecCtx->pix_fmt,  \
                                                        play_win->image.width, \
                                                        play_win->image.height, \
                                                        AV_PIX_FMT_RGB24, \
                                                        SWS_BICUBIC,  /* AV_PIX_FMT_RGB24 SWS_BICUBIC AV_PIX_FMT_YUV420P */ \
                                                       NULL, NULL, NULL);

        if(decode_h264.img_convert_ctx == NULL)
            return -1;
        first = 1;
    }
    if((got_picture) && (first == 1))
    {
        avpicture_fill(&pFrameRGB,
                        one_frame_buffer, // *play_win->image.buffer, //decode_h264.frame_buffer,
                       AV_PIX_FMT_RGB24,
                       play_win->image.width,
                       play_win->image.height);

        sws_scale(decode_h264.img_convert_ctx,
                  decode_h264.frame->data,
                  decode_h264.frame->linesize,
                  0 ,
                  decode_h264.codecCtx->height,
                  pFrameRGB.data,
                  pFrameRGB.linesize);

        call_fun((char *)one_frame_buffer);
    }
    else
        printf("lost frame\n");
 #endif
    return got_picture;
}
/*＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊
 *int decoder_pps_psp(int platform,int encode)
 *  根据平台确定PPS和PSP的数据
 *＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊
 */

int QDecodeStream::decoder_pps_psp(int platform,int encode)
/*
 * platform : A5S66 HI3516A
 * encode : H264 H265
*/
{
    int count = 25;
    int consume_no = user_no;
    char *buffer;
    buffer = (char *)malloc(sizeof(char)*1024*1024);
    consume->read_init();
    while(count > 0)
    {
        count--;
        int len = 0;

        if(consume == NULL)
        {
            printf("current consume %d is not exist. \n",consume_no);
            return NULL;
        }

        len = consume->read_data_to_buffer(buffer);
        if(len < 0)
        {
            int count = sliding_window->consume_linklist_delete(consume_no);
            sliding_window->consume_linklist_isEmpty();
            printf("delete no:%d,cur count:%d \n",consume_no,count);
            return NULL;
        }


#if defined(Q_OS_WIN32)
        usleep(1000);
#elif defined(Q_OS_MACX)
        pthread_yield_np();
#elif defined(Q_OS_UNIX)
        pthread_yield();
#endif
        //根据码流获取PPS和PSP
        unsigned int *nual_head;
        nual_head = (unsigned int *)buffer;
        int pps_start,pps_len = 0;
        int psp_start,psp_len = 0;
        for(int i = 0;i < len;i += 1)
        {
            nual_head = (unsigned int *)(buffer + i);
            //printf("nual head:%x \n",*(nual_head));
            if((*nual_head) == 0x01000000)
            {
                printf("%02x \n",(unsigned char)(*(buffer+i+4)));

                if((*(buffer+i+4))  == 0x67)
                {
                    pps_start = i;
                }
                if(*(buffer + i + 4) == 0x68)
                {
                    psp_start = i;
                    pps_len = psp_start - pps_start;
                    psp_len = 8;
                    break;
                }

            }

        }
        char *pps = (char *)malloc(sizeof(char) * pps_len);
        memcpy(pps,(buffer + pps_start),pps_len);
        char *psp = (char *)malloc(sizeof(char) * psp_len);
        memcpy(psp,(buffer + psp_start),psp_len);
        decode_h264.codecCtx->extradata = (uint8_t *)malloc(sizeof(uint8_t) *(pps_len + psp_len));
        memcpy(decode_h264.codecCtx->extradata,pps,pps_len);
        memcpy(decode_h264.codecCtx->extradata + pps_len,psp,psp_len);
        decode_h264.codecCtx->extradata_size = pps_len + psp_len;
        printf("\n extern_data:");
        for(int i = 0; i < pps_len + psp_len;i++)
        {
            if(i % 4 == 0)
                printf("\n");
            printf("%02x ",decode_h264.codecCtx->extradata[i]);
        }
        printf("\n");
        delete pps;
        delete psp;
        return (pps_len + psp_len);
    }
    return 0;

}

/*
 * 初始化视频解码参数
 * platform:  0:A5s66  1:3516A(Hisi)
 * encode:    0:h.264  1:h.265   2:jpeg
 */
void QDecodeStream::ipcam_decode_init(int platform,int encode)
{
    int pps_code_size;

    AVDictionary *opts = NULL;
    avcodec_register_all();  //注册所有的文件格式和编解码库，只能调用一次

    if(encode == H264)
        decode_h264.codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    else if(encode == H265)
        decode_h264.codec = avcodec_find_decoder(AV_CODEC_ID_H265);
    else if(encode == JPEG)
        decode_h264.codec = avcodec_find_decoder(AV_CODEC_ID_MJPEG);
    decode_h264.codecCtx = avcodec_alloc_context3(decode_h264.codec);     //解码器的环境变量选择解码器
    //decode_h264.codecCtx->flags |=  CODEC_FLAG_TRUNCATED;
    int frame_thread_supported = (decode_h264.codecCtx->codec->capabilities & CODEC_CAP_FRAME_THREADS)
               && !(decode_h264.codecCtx->flags & CODEC_FLAG_TRUNCATED)
               && !(decode_h264.codecCtx->flags2 & CODEC_FLAG_LOW_DELAY)
               && !(decode_h264.codecCtx->flags2 & CODEC_FLAG2_CHUNKS);

    decode_h264.codecCtx->delay = 0;
    if(decode_h264.codecCtx->thread_count == 1)
    {
        decode_h264.codecCtx->active_thread_type = 0;
    }
    else if(frame_thread_supported && (decode_h264.codecCtx->thread_type & FF_THREAD_FRAME))
    {
        decode_h264.codecCtx->active_thread_type = FF_THREAD_FRAME;
    }
    else if((decode_h264.codecCtx->codec->capabilities & CODEC_CAP_SLICE_THREADS) &&
            (decode_h264.codecCtx->thread_type & FF_THREAD_SLICE))
    {
        decode_h264.codecCtx->active_thread_type = FF_THREAD_SLICE;
    }
    else if(!(decode_h264.codecCtx->codec->capabilities & CODEC_CAP_AUTO_THREADS))
    {
        decode_h264.codecCtx->thread_count = 1;
        decode_h264.codecCtx->active_thread_type = 0;
    }

    if(decode_h264.codecCtx->thread_count > MAX_AUTO_THREADS)
    {
        av_log(decode_h264.codecCtx,AV_LOG_WARNING,"Application has requested %d threads.Using a thread count greater than %d is not recommended.\n",
               decode_h264.codecCtx->thread_count,MAX_AUTO_THREADS);
    }


    //根据编码，决定PPS PSP的头格式数据
    pps_code_size = decoder_pps_psp(platform,encode);
    if(pps_code_size == 0)
        printf("get pps & psp head failed!\n");

/*
    //解码设置
    if(encode == H265)
    {
        av_opt_set(decode_h264.codecCtx->priv_data,"x265-params","qp=0",0);
        av_opt_set(decode_h264.codecCtx->priv_data,"preset","veryfast",0);
        av_opt_set(decode_h264.codecCtx->priv_data,"x265-params","crf=10",0);
    }
    decode_h264.codecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    decode_h264.codecCtx->time_base.den = 25;
    decode_h264.codecCtx->global_quality = 1;

    av_opt_set(decode_h264.codecCtx->priv_data,"tune","zero-latency",0);
    decode_h264.codecCtx->active_thread_type |= FF_THREAD_FRAME;
*/

//打开编码
    avcodec_open2(decode_h264.codecCtx, decode_h264.codec, &opts);
    decode_h264.frame = av_frame_alloc();
    decode_h264.parser = av_parser_init(AV_CODEC_ID_H264);
    decode_h264.img_convert_ctx = sws_alloc_context();

    sws_init_context(decode_h264.img_convert_ctx,NULL,NULL);

    //分配一帧图像的存储空间
    one_frame_buffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_RGB24,
                                                               play_win->image.width,play_win->image.height));

}

/**********************************
 * 计算时间差
 **********************************
*/
int   QDecodeStream::timeval_subtract(struct   timeval*   result,   struct   timeval*   x,   struct   timeval*   y)
{
    int   nsec;

    if   (   x->tv_sec>y->tv_sec   )
        return   -1;

    if   (   (x->tv_sec==y->tv_sec)   &&   (x->tv_usec>y->tv_usec)   )
        return   -1;

    result->tv_sec   =   (   y->tv_sec-x->tv_sec   );
    result->tv_usec   =   (   y->tv_usec-x->tv_usec   );

    if   (result->tv_usec<0)
    {
        result->tv_sec--;
        result->tv_usec+=1000000;
    }

    return   0;
}
