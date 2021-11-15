#include "source/source_music.h"

#include <string.h>

#define MUSIC_BUFFER_COUNT  2

#define IS_FFT_ASYNC(flag)              ((flag) & (MUSIC_FLAG_FFT_CONVERT_ASYNC))
#define IS_SAMPLING_ASYNC(flag)         ((flag) & (MUSIC_FLAG_SAMPLING_ASYNC))

#define SET_FFT_ASYNC_FLAG(flags)       ((flags) | (MUSIC_FLAG_FFT_CONVERT_ASYNC))
#define SET_SAMPLING_ASYNC_FLAG(flags)  ((flags) | (MUSIC_FLAG_SAMPLING_ASYNC))

#define wait_util(condition)            while(condition)

void sampling_async_finish(void *args);

static uint16_t find_correct_fft_size(uint16_t led_count)
{
    uint16_t result = 1;

    while(result < led_count)
    {
        result <<= 1;
    }

    return (result >> 1);
}

static struct music_buffer_node **__alloc_ring_buffer(struct music_handler *handler, struct music_buffer_node **prev)
{
    static int recursion_count = 0;

    *prev = (struct music_buffer_node *)malloc(sizeof(struct music_buffer_node));

    if(*prev == NULL)
        return NULL;

    (*prev)->buffer = (int16_t *)malloc(sizeof(int16_t) * handler->fft_size * 2);
    if((*prev)->buffer == NULL)
    {
        return NULL;
    }

    (*prev)->state = MUSIC_STATE_FREE;
    (*prev)->buf_offset = 0;

    if(recursion_count != (MUSIC_BUFFER_COUNT - 1)) {
        recursion_count++;
        return __alloc_ring_buffer(handler, &((*prev)->next));
    } else {
        recursion_count = 0;
        return prev;
    }
}

static uint16_t get_value_music(struct source *source)
{
    struct music_handler *internal = (struct music_handler *)source;

    if(source->magic != SOURCE_MAGIC_MUSIC)
    {
        return 0;
    }

    if(internal->read->buf_offset == internal->music_node_buffer_size)
    {
        internal->read->buf_offset = 0;
        internal->read->state = MUSIC_STATE_FREE;
        internal->read = internal->read->next;

        if(IS_SAMPLING_ASYNC(internal->flag))
        {
            if(internal->write->state == MUSIC_STATE_FREE)
            {
                internal->write->state = MUSIC_STATE_SAMPLING;
                internal->sampling_hw(internal->write->buffer, internal->fft_size * 2, sampling_async_finish, source);
            }
        }
    }

    if(!IS_SAMPLING_ASYNC(internal->flag))
    {
        if(internal->read->state == MUSIC_STATE_FREE)
        {
            internal->sampling_hw(internal->read->buffer, internal->fft_size * 2, sampling_async_finish, source);

            internal->read->state = MUSIC_STATE_NEED_CONVERT;
        }
    }

    if(internal->read->state == MUSIC_STATE_SAMPLING)
    {
        return SOURCE_INVALID_VALUE;
    }

    if(internal->read->state == MUSIC_STATE_NEED_CONVERT)
    {
        internal->fft_convert_hw(internal->read->buffer, internal->fft_size);
        if(internal->normalise_hw != NULL)
        {
            internal->normalise_hw(internal->read->buffer, internal->fft_size);
        }

        internal->read->state = MUSIC_STATE_READY;
    }

    return (uint16_t)(internal->read->buffer[internal->read->buf_offset++]);
}

static void reset_sequence_music(struct source* source)
{

}

struct source *source_init_music(struct source_config *config)
{
    struct source *result;
    struct music_handler *internal;
    struct source_config_music *music_config = (struct source_config_music *)config;

    if(config->type != SOURCE_TYPE_MUSIC)
    {
        goto err;
    }

    internal = (struct music_handler *)malloc(sizeof(struct music_handler));
    if(internal == NULL)
    {
        goto err;
    }
    result = (struct source *)internal;

    result->magic = SOURCE_MAGIC_MUSIC;
    result->get_value = get_value_music;
    result->reset_sequence = reset_sequence_music;

    internal->source_name = music_config->source_name;
    internal->fft_size = find_correct_fft_size(music_config->initial_led_count);
    internal->music_node_buffer_size = music_config->initial_led_count;
    internal->flag = 0;
    if(music_config->is_fft_conversion_async)
    {
        internal->flag = SET_FFT_ASYNC_FLAG(internal->flag);
    }

    if(music_config->is_sampling_async)
    {
        internal->flag = SET_SAMPLING_ASYNC_FLAG(internal->flag);
    }

    internal->read = *(__alloc_ring_buffer(internal, &(internal->write)));
    if(internal->read == NULL)
    {
        goto dealloc;
    }

    internal->read->next = internal->write;
    internal->read = internal->write;

    internal->fft_convert_hw = music_config->fft_convert_fnc;
    internal->sampling_hw = music_config->sampling_fnc;
    internal->normalise_hw = music_config->normalise_fnc;

    if(IS_SAMPLING_ASYNC(internal->flag))
    {
        internal->write->state = MUSIC_STATE_SAMPLING;
        internal->sampling_hw(internal->write->buffer, internal->fft_size * 2, sampling_async_finish, internal);
    }

    return result;

dealloc:
    free(internal);
err:
    return NULL;
}

void sampling_async_finish(void *args)
{
    struct source *handler = (struct source *)args;
    struct music_handler *internal = (struct music_handler *)handler;

    if(handler->magic != SOURCE_MAGIC_MUSIC)
    {
        return;
    }

    if(internal->write->state == MUSIC_STATE_SAMPLING)
    {
        internal->write->state = MUSIC_STATE_NEED_CONVERT;

        internal->write = internal->write->next;

        if(internal->write->state == MUSIC_STATE_FREE)
        {
            internal->write->state = MUSIC_STATE_SAMPLING;
            internal->sampling_hw(internal->write->buffer, internal->fft_size * 2, sampling_async_finish, args);
        }
    }
}