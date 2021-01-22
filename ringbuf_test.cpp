#include "ringbuf_helper.h"
#include "H264Reader.h"
#include "MJPGReader.h"
#include "PCMReader.h"
#include "AACReader.h"

Reader *reader_factory_helper(const int key)
{
    Reader *ret = NULL;
    do {
        if (CHECK_VIDEO_KEY(key) || CHECK_AUDIO_KEY(key)) {
            Reader br(key);
            if (!br.init()) {
                ringbuf_basic_hdr *hdr = br.get_ringbuf_basic_hdr();
                if (hdr) {
                    switch (hdr->type) {
                        case CODEC_H264:
                            ret = new H264Reader(key);
                            break;
                        case CODEC_MJPG:
                            ret = new MJPGReader(key);
                            break;
                        case CODEC_PCM:
                            ret = new PCMReader(key);
                            break;
                        case CODEC_AAC:
                            ret = new AACReader(key);
                            break;
                        default:
                            erro("codec not support %u\n", hdr->type);
                            break;
                    }
                }
                br.deinit();
            } else {
                erro("fail to init Reader\n");
            }
        } else {
            erro("key need to be created by GENE_VIDEO/AUDIO_KEY\n");
        }
    } while (false);
    return ret;
}
