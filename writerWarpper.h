#ifndef AACWRITER_H
#define AACWRITER_H
#include "Writer.h"

class AACWriter : public Writer
{
    protected:
        ringbuf_aac_hdr *rbuf_aac_hdr;

    public:
        AACWriter();
        virtual ~AACWriter();
        virtual int init(const int key, const size_t share_data_size, const size_t frame_array_num);
        virtual int set_extra_hdr(const void *ringbuf_extra_hdr);
        virtual codec_type get_writer_codec_type();
};

#endif /* AACWRITER_H */
