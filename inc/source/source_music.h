/*  Reglas:
1. Size of the fft conversion FN = 2^n; n will be calculated empirically; depends on size of LED strip
2. Number of samples SN = 2 * FN
3. Implement two strategy of fft conversion: sync and async. Defined in config.
4. Implement two strategy of sampling: sync and async. Defined in config.
*/

#ifndef __SOURCE_MUSIC_INIT__
#define __SOURCE_MUSIC_INIT__

#include "source/source.h"
#include "source/source_aggregator.h"

#include <stdint.h>

#define MUSIC_FLAG_FFT_CONVERT_ASYNC    0x01
#define MUSIC_FLAG_SAMPLING_ASYNC       0x02


struct music_private;

enum music_buffer_state
{
    MUSIC_STATE_FREE = 0x00,
    MUSIC_STATE_SAMPLING,
    MUSIC_STATE_NEED_CONVERT,
    MUSIC_STATE_CONVERTING,
    MUSIC_STATE_READY
};

struct music_buffer_node
{
    int16_t *buffer;
    uint32_t buf_size;
    uint32_t buf_offset;
    enum music_buffer_state state;

    struct music_buffer_node *next;
};

struct music_handler
{
    struct source base;
    struct music_buffer_node *read;
    struct music_buffer_node *write;

    uint8_t flag;
    uint16_t fft_size;

    void (*sampling_hw)(int16_t *dst, uint16_t sampling_size);
    void (*fft_convert_hw)(int16_t *buf, uint16_t fft_size);
    void (*normalise_hw)(int16_t *buf, uint16_t fft_size);
};

struct source *source_init_music(struct source_config *config);

#endif /* __SOURCE_MUSIC_INIT__ */