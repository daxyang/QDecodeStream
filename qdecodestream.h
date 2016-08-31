#ifndef QDECODESTREAM_H
#define QDECODESTREAM_H
#include "pthread.h"
//#include "qdecodestream_global.h"
#include "decode_param.h"
#include "unistd.h"
#include "getrawdata.h"
#include "sys/time.h"
typedef unsigned int (u32);


//class QDECODESTREAMSHARED_EXPORT QDecodeStream
class QDecodeStream
{

public:

    QDecodeStream(int width,int height);
    GetRAWData *get_rawdata;
    void bound(QSlidingWindow *slidingwnd);
    int set_user(int userno);
    void set_rtn_callback(void (*callback)(char *));
    void start_decode();
    void open_debug();



private:
    static void *decode_thread_run(void *ptr);
    int decode_main_frame(uint8_t *data, int size);
    void ipcam_decode_init(int platform,int encode);
    int decoder_pps_psp(int platform,int encode);

    decode_h264_t decode_h264;  //H.264解码相关参数
    int first;
    view_param_t *play_win;   //视频显示相关参数
    uint8_t *one_frame_buffer;

    int user_no;
    char *data_buffer;


    QSlidingWindowConsume *consume;
    QSlidingWindow *sliding_window;
    pthread_t decode_pthread_id;

    static void *getraw_run(void *ptr);
    void (*call_fun)(char *);
    int debug_enable;

    int   timeval_subtract(struct   timeval*   result,   struct   timeval*   x,   struct   timeval*   y) ;

};

#endif // QDECODESTREAM_H
