#ifndef __SOURCE_AGGREGATOR__
#define __SOURCE_AGGREGATOR__

#include <stdint.h>
#include <stdlib.h>

#define AGGREGATOR_GET_ACTIVE_BANK(from_aggregator, for_config_num)            (((from_aggregator).flags >> ((for_config_num) % 3)) & 0x01)
#define AGGREGATOR_IS_BANK_SWITCHING_NEED(in_aggregator, for_config_num)       (((in_aggregator).flags >> (3 + ((for_config_num) % 3))) & 0x01)

#define AGGREGATOR_SWITCH_ACTIVE_BANK(in_aggregator, for_config_num)           (in_aggregator).flags ^= (0x1 << ((for_config_num) % 3))
#define AGGREGATOR_SET_BANK_SWITCHING_FLAG(in_aggregator, for_config_num)      (in_aggregator).flags |= (0x8 << ((for_config_num) % 3))

#define AGGREGATOR_CLEAR_BANK_SWITCHING_FLAG(in_aggregator, for_config_num)    (in_aggregator).flags &= ~(0x8 << ((for_config_num) % 3))

struct source;

enum source_type 
{
    SOURCE_TYPE_LINEAR,
    SOURCE_TYPE_TRIGONOMETRIC,
    SOURCE_TYPE_MUSIC
};

struct source_aggregator
{
    uint32_t flags;
    struct source *first[2];
    struct source *second[2];
    struct source *third[2];
};

struct source_config
{
    enum source_type type;
};

struct source_config_function
{
    struct source_config base;
    
    uint16_t k;
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

    void (*sampling_fnc)(int16_t *dst, uint16_t sampling_size);
    void (*fft_convert_fnc)(int16_t *buf, uint16_t fft_size);
    void (*normalise_fnc)(int16_t *buf, uint16_t size);
};

struct source *make_source_from_config(struct source_config *config);
int make_source_aggregator_from_config(struct source_aggregator *aggregator, struct source_config *first, struct source_config *second, struct source_config *third);
void source_aggregator_free(struct source_aggregator *aggregator, uint8_t config_num);
int get_source_description(char *dst, struct source *s);

extern void sampling_async_finish(struct source *handler);

#endif /* __SOURCE_AGGREGATOR__ */