#include "writerWarpper.h"
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
    srand((unsigned) time(&t));
    signal(SIGINT,signal_handler);

    stream_info stream1 = {
        .id = 1,
        .format = 0x4,
        .bitrate = 10000,
        .fps = 30,
        .width = 1920,
        .height = 1080,
        .desc = {'t','e','d','d','y'}
    };

    writerBase *data_writer = new writerBase();
    data_writer->init(1001, 5 * 1000 * 1000, (char*)&stream1, sizeof(stream_info), sizeof(frame_info));
    
    printf("id:%d, format:%d, bitrate:%d, fps:%d\n",stream1.id, stream1.format, stream1.bitrate, stream1.fps);
    printf("resolution:%dx%d, desc:%s\n",stream1.width, stream1.height, stream1.desc);

    int frame_count = 1;

    while(!exit_flag) {
        int data_size = 1000 * 2000 * (rand() % 100) / 100;
        char *data = new char[data_size];
        frame_info frame = {
            .fnum = frame_count++,
            .iframe = rand() % 2,
            .pts = (int)time(NULL)
        };
        printf("fnum:%d, size:%d iframe:%d, pts:%d \n",frame.fnum, data_size, frame.iframe, frame.pts);
        data_writer->push_frame((char*)&frame, data, data_size);
        delete(data);
        data_writer->debug_ringbuf();
        // usleep(200000);
    };

    data_writer->deinit();

    printf("hello writerWarpper\n");
    return 0;
}