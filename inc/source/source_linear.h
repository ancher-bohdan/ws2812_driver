#ifndef __SOURCE_LINEAR__
#define __SOURCE_LINEAR__

#include "source/source_aggregator.h"

#define SOURCE_MAGIC_LINEAR     0xDEADBEEF

struct source_linear
{
    struct source base;
    
    uint16_t b;
    uint16_t y_max;
    uint16_t y_max_initial;
    int16_t x_n;
    uint8_t is_converge;
    uint8_t k;
};

#endif