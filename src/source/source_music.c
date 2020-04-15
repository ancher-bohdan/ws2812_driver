#include "source/source_music.h"
#include ".config.h"

#include <string.h>

#define SOURCE_MUSIC_BUFFER_COUNT   2
#define SOURCE_MUSIC_FBUFFER_SIZE   CONFIG_LED_NUMBERS
#define SOURCE_MUSIC_TBUFFER_SIZE   CONFIG_LED_NUMBERS

static struct source_music_node** alloc_buffer_nodes(struct source_music_node **prev)
{
    static int recursion_count = 0;
    static uint16_t *TD = NULL;
    static uint16_t *FD = NULL;

    *prev = (struct source_music_node *)malloc(sizeof(struct source_music_node));

    if(*prev == NULL)
        return NULL;

    if(TD == NULL)
        TD = (uint16_t *) malloc (SOURCE_MUSIC_TBUFFER_SIZE * SOURCE_MUSIC_BUFFER_COUNT);
    
    if(FD == NULL)
        FD = (uint16_t *) malloc (SOURCE_MUSIC_FBUFFER_SIZE * SOURCE_MUSIC_BUFFER_COUNT);

    (*prev)->td = TD + (SOURCE_MUSIC_TBUFFER_SIZE * recursion_count);
    (*prev)->fd = FD + (SOURCE_MUSIC_FBUFFER_SIZE * recursion_count);
    (*prev)->fd_offset = 0;
    (*prev)->td_offset = 0;
    (*prev)->buff_size = SOURCE_MUSIC_FBUFFER_SIZE;
    (*prev)->state = MUSIC_NODE_FREE;

    if(recursion_count != (SOURCE_MUSIC_BUFFER_COUNT - 1)) {
        recursion_count++;
        return alloc_buffer_nodes(&((*prev)->next));
    } else {
        TD = FD = NULL;
        recursion_count = 0;
        return prev;
    }
}

static uint16_t get_value_music(struct source *source)
{
    struct source_music *internal = (struct source_music *)source;

    if(source->magic != SOURCE_MAGIC_MUSIC)
    {
        return 0;
    }

    if(internal->read->fd_offset == internal->read->buff_size)
    {
        internal->read->fd_offset = 0;
        internal->read->state = MUSIC_NODE_FREE;
        internal->read = internal->read->next;
    }

    while(internal->read->state != MUSIC_NODE_FILL) { }

    return (uint16_t)(internal->read->fd[internal->read->fd_offset++]);
}

static void reset_sequence_music(struct source* source)
{
    (void*)source;
}

struct source *source_init_music(struct source_config *config)
{
    struct source *result;
    struct source_music *internal;
    struct source_config_music *music_config = (struct source_config_music *)config;

    if(config->type != SOURCE_TYPE_MUSIC)
    {
        return NULL;
    }

    internal = (struct source_music *)malloc(sizeof(struct source_music));
    result = (struct source *)internal;

    if(internal == NULL)
    {
        return NULL;
    }

    result->magic = SOURCE_MAGIC_MUSIC;
    result->get_value = get_value_music;
    result->reset_sequence = reset_sequence_music;

    internal->hw_fft_convert = music_config->hw_convert;
    internal->read = *(alloc_buffer_nodes(&(internal->write)));
    internal->read->next = internal->write;
    internal->read = internal->write;

    return result;
}

int source_music_set_data_from_origin(struct source_music *dst, uint16_t *src, uint8_t size)
{
    int byte_count = 0;

    if(dst->base.magic != SOURCE_MAGIC_MUSIC)
    {
        return 0;
    }

    if(dst->write->state != MUSIC_NODE_FREE)
    {
        return 0;
    }

    if(dst->write->td_offset == dst->write->buff_size)
    {
        dst->write->state = MUSIC_NODE_NEED_CONVERT;
        dst->hw_fft_convert(dst->write->td, dst->write->fd);
        dst->write->state = MUSIC_NODE_FILL;
        dst->write->td_offset = 0;
        dst->write = dst->write->next;
    }

    byte_count = size > dst->write->buff_size - dst->write->td_offset ? dst->write->buff_size - dst->write->td_offset : size;

    memcpy( dst->write->td + dst->write->td_offset, src, byte_count ); 

    return byte_count;
}