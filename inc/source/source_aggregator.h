#ifndef __SOURCE_AGGREGATOR__
#define __SOURCE_AGGREGATOR__

#include <stdint.h>
#include <stdlib.h>

struct source
{
    uint32_t magic;
    
    uint16_t (*get_value)(struct source *s);
    void (*reset_sequence)(struct source *s);
};

struct source_aggregator
{
    struct source *first;
    struct source *second;
    struct source *third;
};

struct source_config
{
    uint8_t k;
    uint16_t b;
    uint16_t y_min;
    uint16_t y_max;
    uint8_t x_step;

};

struct source * source_init_linear(struct source_config *config);
struct source_aggregator *make_source_aggregator_from_config(struct source_config *first, struct source_config *second, struct source_config *third);

#endif /* __SOURCE_AGGREGATOR__ */