#ifndef __SOURCE_AGGREGATOR__
#define __SOURCE_AGGREGATOR__

#include <stdint.h>
#include <stdlib.h>

struct source;

enum source_type 
{
    SOURCE_TYPE_LINEAR,
    SOURCE_TYPE_TRIGONOMETRIC,
    SOURCE_TYPE_MUSIC
};

struct source_aggregator
{
    struct source *first;
    struct source *second;
    struct source *third;
};

struct source_config
{
    enum source_type type;
};

struct source_config_function
{
    struct source_config base;
    
    uint8_t k;
    uint16_t change_step_k;
    uint16_t change_step_b;
    uint16_t b;
    uint16_t y_min;
    uint16_t y_max;
};

struct source_config_trigonometric
{
    struct source_config_function base;

    float (*hw_sinus)(float radians);
};

struct source_config_music
{
    struct source_config base;

    uint8_t is_fft_conversion_async;
    uint8_t is_sampling_async;

    void (*sampling_fnc)(uint16_t *dst, uint16_t sampling_size);
    void (*fft_convert_fnc)(uint16_t *buf, uint16_t fft_size);
};

struct source_aggregator *make_source_aggregator_from_config(struct source_config *first, struct source_config *second, struct source_config *third);
void source_aggregator_free(struct source_aggregator *aggregator);

#endif /* __SOURCE_AGGREGATOR__ */