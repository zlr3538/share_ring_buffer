#ifndef AACREADER_H
#define AACREADER_H
#include "Reader.h"

class AACReader : public Reader
{
    protected:
        ringbuf_aac_hdr *rbuf_aac_hdr;

    public:
        AACReader(const int key);
        virtual ~AACReader();
        virtual int init();
        virtual int get_extra_hdr(void *ringbuf_extra_hdr);
};

#endif /* AACREADER_H */
