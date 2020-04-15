#ifndef __SOURCE_MUSIC_INIT__
#define __SOURCE_MUSIC_INIT__

#include "source/source.h"
#include "source/source_aggregator.h"

enum source_music_node_state
{
    MUSIC_NODE_FREE,
    MUSIC_NODE_NEED_CONVERT,
    MUSIC_NODE_FILL
};

struct source_music_node
{
    float *fd;
    float *td;
    uint16_t buff_size;
    uint16_t fd_offset;
    uint16_t td_offset;
    enum source_music_node_state state;
    
    struct source_music_node *next;
};

struct source_music
{
    struct source base;
    struct source_music_node *read;
    struct source_music_node *write;

    int (*hw_fft_convert)(void *in_data, void *out_data);
};

struct source *source_init_music(struct source_config *config);

int source_music_set_data_from_origin(struct source_music *dst, uint16_t *src, uint8_t size);

#endif /* __SOURCE_MUSIC_INIT__ */