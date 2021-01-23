#ifndef WRITERWARPPER_H
#define WRITERWARPPER_H
#include "writerBase.h"

typedef struct {
    int id;
    int format;
    int bitrate;
    int fps;
    int width;
    int height;
    char desc[32];
}stream_info;

typedef struct {
    int fnum;
    int iframe;
    int pts;
}frame_info;

#endif /* WRITERWARPPER_H */
