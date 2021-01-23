#include "readerWarpper.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>

static int exit_flag = 0;

void signal_handler(int signum)
{   
    printf("catch %d\n",signum);
    exit_flag = 1;
}

int main(int argc, char* argv[])
{
    time_t t;
    signal(SIGINT,signal_handler);

    stream_info stream1 = {0};

    readerBase *data_reader = new readerBase();
    data_reader->init(1001);
    data_reader->get_extra_hdr(&stream1, sizeof(stream_info));

    printf("id:%d, format:%d, bitrate:%d, fps:%d\n",stream1.id, stream1.format, stream1.bitrate, stream1.fps);
    printf("resolution:%dx%d, desc:%s\n",stream1.width, stream1.height, stream1.desc);
    
    int frame_count = 0;
    size_t ret = -1;
    size_t data_size = 0;
    char *data = NULL;
    frame_info frame = {0};

    while(!exit_flag) {
        ret = data_reader->get_frame((char*)&frame, sizeof(frame_info), &data, &data_size);
        if(ret == 0)
            printf("fnum:%d, iframe:%d, pts:%d\n",frame.fnum, frame.iframe, frame.pts);
        else
            usleep(100000);
        data_reader->debug_ringbuf();
        // usleep(100000);
    };

    data_reader->deinit();

    printf("hello readerWarpper\n");
    return 0;
}