#ifndef __RECURRENT__INIT__
#define __RECURRENT__INIT__

#include <stdint.h>

struct update_context {
    uint8_t k;
    uint16_t b;
    int16_t x_prev;

    uint16_t (*update_fnc)(struct update_context *);
};

struct update_context_linear {
    struct update_context base_ctx;
    uint8_t is_convergens;
    uint16_t xmax;
};

struct update_context_trigonometric {
    struct update_context base_ctx;
    uint8_t step;
};

typedef uint8_t (*update_fnc)(struct update_context *);

uint16_t recurent_linear_update(struct update_context *ctx);
uint16_t recurent_sin_update(struct update_context *ctx);

#define TO_LINEAR_CONTEXT(ctx)          ((struct update_context_linear *)(ctx))

#define TO_TRIGONOMETRIC_CONTEXT(ctx)   ((struct update_context_trigonometric *)(ctx))

#endif /* __RECURRENT__INIT__ */