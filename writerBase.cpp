#include "writerBase.h"

writerBase::writerBase()
{
    clean_internal_data();
}

writerBase::~writerBase()
{

}

void writerBase::clean_internal_data()
{
    shm_key = 0;
    memset(shm_name,0,32);
    memset(sem_name,0,32);
    shm_fd = -1;
    shm_size = 0;
    sem_ptr = NULL;
    shm_ptr = NULL;
    rbuf_hdr_ptr = NULL;
    data_write_ptr = NULL;
    data_write_end_ptr = NULL;
    data_write_offset = 0;
}

int writerBase::assign_for_new_frame(const size_t data_size)
{
    int frame_total_size = rbuf_hdr_ptr->frame_hdr_size + data_size;
    int remain_size = 0;
    int short_size = 0;

RE_CAL:
    //cal remain_size
    if (rbuf_hdr_ptr->rear_offset - rbuf_hdr_ptr->front_offset > 0) {
        remain_size = rbuf_hdr_ptr->share_data_size - rbuf_hdr_ptr->rear_offset;
    }
    else if (rbuf_hdr_ptr->rear_offset - rbuf_hdr_ptr->front_offset == 0) {
        if(rbuf_hdr_ptr->front_offset == 0 && rbuf_hdr_ptr->rear_offset == 0)
            remain_size = rbuf_hdr_ptr->share_data_size;
        else
            remain_size = 0;
    }
    else {
        remain_size = rbuf_hdr_ptr->front_offset - rbuf_hdr_ptr->rear_offset;
    }
    
    sem_wait(sem_ptr);
    
    //check if need to free some space for new frame
    if (remain_size - frame_total_size >=0) {
        //do not need free space
        data_write_offset = rbuf_hdr_ptr->rear_offset;
    } else {
        //maybe tail space is not enough
        if (rbuf_hdr_ptr->rear_offset - rbuf_hdr_ptr->front_offset > 0) {
            rbuf_hdr_ptr->rear_offset = 0;
            goto RE_CAL;
        }
        //tyr to free space some frames
        short_size = frame_total_size - remain_size;
        while(short_size > 0) {
            frame_basic_hdr *front_frame_hdr = (frame_basic_hdr *)((char *)data_write_ptr + rbuf_hdr_ptr->front_offset);
            short_size -= front_frame_hdr->frame_hdr_size + front_frame_hdr->frame_data_size;
            rbuf_hdr_ptr->front_offset = front_frame_hdr->frame_end_offset;
            rbuf_hdr_ptr->frame_count -= 1;
        }
        data_write_offset = rbuf_hdr_ptr->rear_offset;
    }
    
    sem_post(sem_ptr);
    
    return data_write_offset;
}

int writerBase::update_for_new_frame()
{
    sem_wait(sem_ptr);
    
    rbuf_hdr_ptr->rear_offset = data_write_offset;
    rbuf_hdr_ptr->frame_count += 1;
    
    sem_post(sem_ptr);
    
    return data_write_offset;
}

int writerBase::init(const int key, const size_t share_data_size, const char *ringbuf_hdr,
        const size_t ringbuf_hdr_size, const size_t frame_hdr_size)
{
    int ret = -1;
    do {
        if ((shm_fd != -1) && (shm_ptr)) {
            erro("already set up writer:(%u)\n", key);
            break;
        }
        if (!share_data_size || !ringbuf_hdr_size || !frame_hdr_size) {
            erro("invalid parameters to init writer %u %u %u\n",
                    (unsigned int)share_data_size, (unsigned int)ringbuf_hdr_size,
                    (unsigned int)frame_hdr_size);
            break;
        }
        snprintf(shm_name,32,"ringbuf_shm_%d",key);
        snprintf(sem_name,32,"ringbuf_sem_%d",key);
        shm_size = share_data_size + sizeof(ringbuf_basic_hdr) + ringbuf_hdr_size;
        shm_fd = shm_open(shm_name, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
        if (shm_fd < 0) {
            erro("fail to create shm:%s\n", strerror(errno));
            ret = errno;
            break;
        }
        shm_key = key;
        ftruncate(shm_fd, shm_size);
        shm_ptr = (char *)mmap(NULL, shm_size, PROT_WRITE | PROT_READ, MAP_SHARED, shm_fd, 0);
        if (!shm_ptr) {
            erro("fail to map shm:%s\n", strerror(errno));
            ret = errno;
            break;
        }
        
        rbuf_hdr_ptr = (ringbuf_basic_hdr *)shm_ptr;
        rbuf_hdr_ptr->share_data_size = share_data_size;
        rbuf_hdr_ptr->ringbuf_hdr_size = sizeof(ringbuf_basic_hdr) + ringbuf_hdr_size;
        rbuf_hdr_ptr->frame_hdr_size = sizeof(frame_basic_hdr) + frame_hdr_size;
        rbuf_hdr_ptr->front_offset = 0;
        rbuf_hdr_ptr->rear_offset = 0;
        rbuf_hdr_ptr->frame_count = 0;
        memcpy(rbuf_hdr_ptr->ringbuf_extra_hdr,ringbuf_hdr,ringbuf_hdr_size);
        
        data_write_ptr = shm_ptr + rbuf_hdr_ptr->ringbuf_hdr_size;
        data_write_end_ptr = shm_ptr + shm_size;
        data_write_offset = 0;

        sem_ptr = sem_open(sem_name, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);
        if (!sem_ptr) {
            erro("fail to create sem:%s\n", strerror(errno));
            ret = errno;
            break;
        }
        info("create key(%d) with %u+%u+%u at %p sem: %p start:%p end:%p\n",
                key, (unsigned int)share_data_size,
                (unsigned int)ringbuf_hdr_size,
                (unsigned int)frame_hdr_size,
                shm_ptr, sem_ptr,
                data_write_ptr,
                data_write_end_ptr);
        ret = 0;
    } while (false);

    if(ret < 0)
        clean_internal_data();

    return ret;
}

int writerBase::deinit()
{
    if ((shm_fd != -1) && (shm_ptr)) {
        sem_wait(sem_ptr);
        sem_close(sem_ptr);
        sem_unlink(sem_name);
        munmap(shm_ptr, shm_size);
        /*
         * Mark the segment to be destroyed.  The segment will actually
         * be destroyed only after the last process detaches it (i.e.,
         * when the shm_nattch member of the associated structure
         * shmid_ds is zero).
         */
        shm_unlink(shm_name);
        clean_internal_data();
    }
    return 0;
}

size_t writerBase::push_frame(const char *frame_hdr, const char *frame_data,
        const size_t data_size)
{
    int ret = -1;
    do {
        if ((shm_fd != -1) && (shm_ptr)) {
            assign_for_new_frame(data_size);
            
            frame_basic_hdr *fam_basic_hdr = (frame_basic_hdr *)((char *)data_write_ptr + data_write_offset);
            fam_basic_hdr->fid = rbuf_hdr_ptr->frame_count;
            fam_basic_hdr->frame_hdr_size = rbuf_hdr_ptr->frame_hdr_size;
            fam_basic_hdr->frame_data_size = data_size;
            fam_basic_hdr->frame_data_offset = data_write_offset + fam_basic_hdr->frame_hdr_size;
            fam_basic_hdr->frame_end_offset = fam_basic_hdr->frame_data_offset + data_size;
            memcpy(fam_basic_hdr->frame_extra_hdr,frame_hdr,rbuf_hdr_ptr->frame_hdr_size - sizeof(frame_basic_hdr));
            // update correct index;

            //dbug("%p = %p + (%u x %u)\n", fam_basic_hdr, frame_array_ptr, write_index, rbuf_hdr_ptr->frame_each_size);
            //dbug("before: %p check:%u data:%u current:%u size:%u\n", fam_basic_hdr, fam_basic_hdr->fcheckid_offset, fam_basic_hdr->foffset, data_write_offset, size);
            memcpy((char *)data_write_ptr+data_write_offset, (char *)fam_basic_hdr, fam_basic_hdr->frame_hdr_size);
            data_write_offset += fam_basic_hdr->frame_hdr_size;
            memcpy((char *)data_write_ptr+data_write_offset, (char *)frame_data, data_size);
            data_write_offset += data_size;
            //dbug("offset: %p check:%u data:%u current:%u size:%u\n", fam_basic_hdr, fam_basic_hdr->fcheckid_offset, fam_basic_hdr->foffset, data_write_offset, size);

            update_for_new_frame();
        } else {
            erro("need to init writer first\n");
            break;
        }
        ret = data_size;
    } while (false);
    return ret;
}

