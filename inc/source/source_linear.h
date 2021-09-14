#ifndef __SOURCE_LINEAR__
#define __SOURCE_LINEAR__

#include "source/source_aggregator.h"
#include "source/source.h"

struct source_linear
{
    struct source base;
    
    uint16_t b;
    uint16_t y_max;
    uint16_t step_for_b;
    uint16_t step_for_k;
    uint16_t x_n;
    uint16_t k;
};

struct source * source_init_linear(struct source_config *config);

#endif