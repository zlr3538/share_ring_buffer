#include "ringbuf.h"

int ringbuf_lock(sem_t *sem_ptr)
{
    return sem_wait(sem_ptr);
}

int ringbuf_unlock(sem_t *sem_ptr)
{
    return sem_post(sem_ptr);
}

int semaphore_wait(const int sem_id, const int index)
{
    struct sembuf sem_b;
    sem_b.sem_num = index;
    sem_b.sem_op = 0; // SEM_UNDO seems no necessary.
    sem_b.sem_flg = 0;
    return semop(sem_id, &sem_b, 1);
}


