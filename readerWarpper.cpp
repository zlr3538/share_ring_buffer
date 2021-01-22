#include "AACReader.h"

AACReader::AACReader(const int key): Reader(key)
{
    rbuf_aac_hdr = NULL;
}

AACReader::~AACReader()
{

}

int AACReader::init()
{
    int ret = -1;
    do {

        if (key != GENE_AUDIO_KEY(STREAM_AUDIO_AAC)) {
            warn("key should be create by GENE_AUDIO_KEY(%d)\n", STREAM_AUDIO_AAC);
        }
        if (Reader::init(sizeof(ringbuf_aac_hdr), sizeof(frame_basic_hdr)) != 0) {
            break;
        }
        rbuf_aac_hdr = (ringbuf_aac_hdr *)rbuf_basic_hdr;
        ret = 0;
    } while (false);
    return ret;
}

int AACReader::get_extra_hdr(void *ringbuf_extra_hdr)
{
    int ret = -1;
    do {
        if (!ringbuf_extra_hdr) {
            erro("ringbuf extra hdr is null\n");
            break;
        }
        if ((shm_id != -1) && (shm_ptr)) {
            memcpy((char *)ringbuf_extra_hdr, (char *)&rbuf_aac_hdr->extra_hdr, sizeof(ringbuf_extra_hdr_aac));
        } else {
            erro("need to init aac writer first\n");
            break;
        }
        ret = 0;
    } while (false);
    return ret;
}
