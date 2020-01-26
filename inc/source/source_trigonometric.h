#ifndef __SOURCE_TRIGONOMETRIC__
#define __SOURCE_TRIGONOMETRIC__

#include "source/source_aggregator.h"
#include "source/source.h"

struct source_trigonometric
{
    struct source base;
    uint8_t k;
    uint16_t b;
    uint16_t y_max;
    uint16_t y_max_initial;
    uint16_t step_for_b;
    uint16_t step_for_xn;
    int16_t x_n;

    float (*sinus_calculate)(float radians);
};

struct source *source_init_trigonometric(struct source_config *config);

#endif