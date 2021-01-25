#ifndef RINGBUF_H
#define RINGBUF_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stddef.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>

#ifndef ULOG_DEBUG
#define ULOG_DEBUG
#define dbug(fmt, ...) fprintf(stderr, fmt, ## __VA_ARGS__)
#define info(fmt, ...) fprintf(stderr, fmt, ## __VA_ARGS__)
#define note(fmt, ...) fprintf(stderr, fmt, ## __VA_ARGS__)
#define warn(fmt, ...) fprintf(stderr, fmt, ## __VA_ARGS__)
#define erro(fmt, ...) fprintf(stderr, fmt, ## __VA_ARGS__)
#define crit(fmt, ...) fprintf(stderr, fmt, ## __VA_ARGS__)
#endif

#define MAGIC_HEADER (0x20210124)

typedef struct {
    unsigned int magic;
    int fid;
    struct timeval fpts;
    size_t frame_hdr_size;
    size_t frame_data_size;
    size_t frame_data_offset;
    size_t frame_end_offset;
    char frame_extra_hdr[0];
} frame_basic_hdr;

typedef struct {
    size_t share_data_size;
    size_t ringbuf_hdr_size;
    size_t frame_hdr_size;
    int front_offset;
    int rear_offset;
    int frame_count;
    int frame_id;
    char ringbuf_extra_hdr[0];
} ringbuf_basic_hdr;

int semaphore_lock(const int sem_id, const int index);
int semaphore_wait(const int sem_id, const int index);
int semaphore_unlock(const int sem_id, const int index);



#endif
