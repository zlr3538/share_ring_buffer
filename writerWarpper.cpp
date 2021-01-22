#include "AACWriter.h"

AACWriter::AACWriter()
{
    rbuf_aac_hdr = NULL;
}

AACWriter::~AACWriter()
{

}

int AACWriter::init(const int key, const size_t share_data_size, const size_t frame_array_num)
{
    int ret = -1;
    do {
        if (key != GENE_AUDIO_KEY(STREAM_AUDIO_AAC)) {
            warn("key should be create by GENE_AUDIO_KEY(%d)\n", STREAM_AUDIO_AAC);
        }
        if (Writer::init(key, share_data_size, sizeof(ringbuf_aac_hdr),
                    frame_array_num, sizeof(frame_basic_hdr)) != 0) {
            break;
        }
        // already cann Writer::init to allocate rbuf_aac_hdr size
        rbuf_aac_hdr = (ringbuf_aac_hdr *)rbuf_basic_hdr;
        ret = 0;
    } while (false);
    return ret;
}

int AACWriter::set_extra_hdr(const void *ringbuf_extra_hdr)
{
    int ret = -1;
    do {
        if (!ringbuf_extra_hdr) {
            erro("ringbuf extra hdr is null\n");
            break;
        }
        if ((shm_id != -1) && (shm_ptr)) {
            //ringbuf_extra_hdr_aac *extra_aac_hdr = (ringbuf_extra_hdr_aac *)ringbuf_extra_hdr;
            memcpy((char *)&rbuf_aac_hdr->extra_hdr, (char *)ringbuf_extra_hdr, sizeof(ringbuf_extra_hdr_aac));
        } else {
            erro("need to init aac writer first\n");
            break;
        }
        ret = 0;
    } while (false);
    return ret;
}

codec_type AACWriter::get_writer_codec_type()
{
    return CODEC_AAC;
}
