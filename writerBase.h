#ifndef WRITERBASE_H
#define WRITERBASE_H
#include "ringbuf.h"

/* share memory layout
 * -------------------------------------------------------------------
 * | ringbuf_basic_hdr | data.... {frame_basic_hdr | frame_data} ... |
 * -------------------------------------------------------------------
 */

class writerBase
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
        char *data_write_ptr;
        char *data_write_end_ptr;
        size_t data_write_offset;

    protected:
        void clean_internal_data();
        int assign_for_new_frame(const size_t size);
        int update_for_new_frame();

    public:
        writerBase();
        ~writerBase();
        int init(const int key, const size_t share_data_size, const char *ringbuf_hdr,
                const size_t ringbuf_hdr_size, const size_t frame_hdr_size);
        int deinit();
        size_t push_frame(const char *frame_hdr, const char *frame_data,
                const size_t data_size);
        void debug_ringbuf();
};

#endif /* WRITERBASE_H */
