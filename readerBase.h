#ifndef READERBASE_H
#define READERBASE_H
#include "ringbuf.h"

class readerBase
{
    protected:
        // global share memory options
        key_t shm_key;
        char shm_name[32];
        char sem_name[32];
        int shm_fd;
        size_t shm_size;
        sem_t *sem_ptr;
        char *shm_ptr;
        ringbuf_basic_hdr *rbuf_hdr_ptr;
        char *data_read_ptr;
        char *data_read_end_ptr;
        size_t data_read_offset;

    protected:
        void clean_internal_data();
        virtual int init(const size_t ringbuf_hdr_size, const size_t frame_each_size);

    public:
        readerBase();
        virtual ~readerBase();
        virtual int init(const int key);
        virtual size_t get_extra_hdr(void *ringbuf_extra_hdr,size_t hdr_size);
        virtual int deinit();
        virtual size_t get_frame(void *hdr_ptr, size_t hdr_size, char **data_ptr, size_t *data_size);
        virtual bool is_frame_ready(unsigned long long fid);
};

#endif /* READERBASE_H */
