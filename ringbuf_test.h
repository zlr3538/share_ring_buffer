#ifndef RINGBUF_HELPER_H
#define RINGBUF_HELPER_H
#include "ringbuf.h"
#include "Reader.h"

Reader *reader_factory_helper(const int key);

#endif /* RINGBUF_HELPER_H */
