#include "readerBase.h"

readerBase::readerBase()
{
    clean_internal_data();
}

readerBase::~readerBase()
{

}

void readerBase::clean_internal_data()
{
    shm_key = 0;
    memset(shm_name,0,32);
    memset(sem_name,0,32);
    shm_fd = -1;
    shm_size = 0;
    sem_ptr = NULL;
    shm_ptr = NULL;
    rbuf_hdr_ptr = NULL;
    data_read_ptr = NULL;
    data_read_end_ptr = NULL;
    data_read_offset = 0;
}

int readerBase::init(const int key)
{
    int ret = -1;
    do {
        if ((shm_fd != -1) && (shm_ptr)) {
            erro("already set up reader:(%d)\n", key);
            break;
        }
        snprintf(shm_name,32,"ringbuf_shm_%d",key);
        snprintf(sem_name,32,"ringbuf_sem_%d",key);
        shm_fd = shm_open(shm_name, O_RDWR, S_IRUSR | S_IWUSR);
        if (shm_fd < 0) {
            erro("fail to open shm:%s\n", strerror(errno));
            ret = errno;
            break;
        }
        shm_key = key;
        shm_ptr = (char *)mmap(NULL, sizeof(ringbuf_basic_hdr), PROT_READ, MAP_SHARED, shm_fd, 0);
        if (!shm_ptr) {
            erro("fail to map hdr shm:%s\n", strerror(errno));
            ret = errno;
            break;
        }
        rbuf_hdr_ptr = (ringbuf_basic_hdr *)shm_ptr;
        //dbug("rbuf_hdr_ptr %u %u %u %u\n",
                //rbuf_hdr_ptr->ringbuf_hdr_size,
                //rbuf_hdr_ptr->write_index,
                //rbuf_hdr_ptr->frame_array_num,
                //rbuf_hdr_ptr->frame_each_size);
        shm_size = rbuf_hdr_ptr->share_data_size + rbuf_hdr_ptr->ringbuf_hdr_size;
        ret = munmap(shm_ptr, sizeof(ringbuf_basic_hdr));
        if (ret < 0) {
            erro("fail to munmap hdr shm:%s\n", strerror(errno));
            ret = errno;
            break;
        }
        
        shm_ptr = (char *)mmap(NULL, shm_size, PROT_WRITE | PROT_READ, MAP_SHARED, shm_fd, 0);
        if (!shm_ptr) {
            erro("fail to map shm:%s\n", strerror(errno));
            ret = errno;
            break;
        }
        rbuf_hdr_ptr = (ringbuf_basic_hdr *)shm_ptr;
        data_read_ptr = shm_ptr + rbuf_hdr_ptr->ringbuf_hdr_size;
        data_read_end_ptr = shm_ptr + shm_size;
        data_read_offset = 0;

        sem_ptr = sem_open(sem_name, O_RDWR);
        if (!sem_ptr) {
            erro("fail to open sem:%s\n", strerror(errno));
            ret = errno;
            break;
        }
        info("attach key(%d) with %u+%u+%u at %p sem: %p start:%p end:%p\n",
                key, (unsigned int)rbuf_hdr_ptr->share_data_size,
                (unsigned int)(rbuf_hdr_ptr->ringbuf_hdr_size - sizeof(ringbuf_basic_hdr)),
                (unsigned int)(rbuf_hdr_ptr->frame_hdr_size - sizeof(frame_basic_hdr)),
                shm_ptr, sem_ptr,
                data_read_ptr,
                data_read_end_ptr);
        ret = 0;
    } while (false);
    return ret;
}

size_t readerBase::get_extra_hdr(void *ringbuf_extra_hdr,size_t hdr_size)
{
    size_t ret = -1;
    
    if ((shm_fd != -1) && (shm_ptr)) {
        if (hdr_size == rbuf_hdr_ptr->ringbuf_hdr_size - sizeof(ringbuf_basic_hdr)) {
            memcpy((char*)ringbuf_extra_hdr,rbuf_hdr_ptr->ringbuf_extra_hdr,hdr_size);
            ret = hdr_size;
        } else {
            erro("ringbuf extra hdr size invalid: %lu,%lu\n", rbuf_hdr_ptr->ringbuf_hdr_size - sizeof(ringbuf_basic_hdr),
                    hdr_size);
        }
    } else {
        erro("need to init reader first\n");
    }
    return ret;
}

int readerBase::deinit()
{
    if ((shm_fd != -1) && (shm_ptr)) {
        sem_wait(sem_ptr);
        sem_post(sem_ptr);
        sem_close(sem_ptr);
        munmap(shm_ptr, shm_size);
        /*
         * Mark the segment to be destroyed.  The segment will actually
         * be destroyed only after the last process detaches it (i.e.,
         * when the shm_nattch member of the associated structure
         * shmid_ds is zero).
         */
        clean_internal_data();
    } else {
        erro("need to init reader first\n");
    }
    return 0;
}

size_t readerBase::get_frame(void *hdr_ptr, size_t hdr_size, char **data_ptr, size_t *data_size)
{
    int ret = -1;
    do {
        if ((shm_fd != -1) && (shm_ptr)) {
            if(rbuf_hdr_ptr->frame_count == 0)
            {
                erro("no frame available \n");
                break;
            }
            sem_wait(sem_ptr);
            
            frame_basic_hdr *front_basic_hdr = (frame_basic_hdr *)((char *)data_read_ptr + rbuf_hdr_ptr->front_offset);
            if (front_basic_hdr->magic != MAGIC_HEADER) {
                rbuf_hdr_ptr->front_offset = 0;
                frame_basic_hdr *front_basic_hdr = (frame_basic_hdr *)((char *)data_read_ptr + rbuf_hdr_ptr->front_offset);
            }
            if (hdr_size == front_basic_hdr->frame_hdr_size - sizeof(frame_basic_hdr)) {
                memcpy((char*)hdr_ptr,front_basic_hdr->frame_extra_hdr,hdr_size);
            } else {
                erro("frame extra hdr size invalid: %lu,%lu\n", 
                        front_basic_hdr->frame_hdr_size - sizeof(frame_basic_hdr), hdr_size);
                sem_post(sem_ptr);
                break;
            }
            
            data_read_offset = rbuf_hdr_ptr->front_offset;
            data_read_offset += front_basic_hdr->frame_hdr_size;
            *data_ptr = (char *)data_read_ptr + data_read_offset;
            *data_size = front_basic_hdr->frame_data_size;
            data_read_offset += front_basic_hdr->frame_data_size;

            rbuf_hdr_ptr->front_offset = data_read_offset;
            rbuf_hdr_ptr->frame_count -= 1;
            
            sem_post(sem_ptr);
        } else {
            erro("need to init reader first\n");
            break;
        }
        ret = 0;
    } while (false);
    return ret;
}

bool readerBase::is_frame_ready(unsigned long long fid=0)
{
    bool ret = false;
    
    if ((shm_fd != -1) && (shm_ptr)) {
        if (!fid && rbuf_hdr_ptr->frame_count)
            ret = true;
        else if (fid && fid < rbuf_hdr_ptr->frame_count)
            ret = true;
    } else {
        erro("need to init reader first\n");
    }
    
    return ret;
}

void readerBase::debug_ringbuf()
{
    info("ringbuf_basic_hdr front:%d, rear:%d, count:%d\n",
        rbuf_hdr_ptr->front_offset,
        rbuf_hdr_ptr->rear_offset,
        rbuf_hdr_ptr->frame_count);
    
    return;
}
