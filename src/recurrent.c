#include "recurrent.h"
#include "usr_math.h"

uint16_t recurent_linear_update(struct update_context *ctx)
{
    int16_t result;
    struct update_context_linear *lin_ctx = TO_LINEAR_CONTEXT(ctx);

    if(lin_ctx->is_convergens)
    {
        result = ctx->k * ctx->x_prev + ctx->b;
        if(result >= lin_ctx->xmax) {
            lin_ctx->is_convergens = 0;
            goto div;
        }
        ctx->x_prev++;
        return (uint16_t)result;
    }
    else
    {
div:
        result = (-1) * ctx->x_prev * ctx->k - ctx->b + (lin_ctx->xmax << 1);
        ctx->x_prev++;
        if(result <= 1) {
            lin_ctx->is_convergens = 1;
            ctx->x_prev = (-1) * (ctx->b / ctx->k);
            return 1;
        }
        return (uint16_t)result;
    }
}

#define TO_RADIAN(degree) ((degree) * (PI) / 180) 

uint16_t recurent_sin_update(struct update_context *ctx)
{
    int16_t result;
    struct update_context_trigonometric *trig_context = TO_TRIGONOMETRIC_CONTEXT(ctx);

    result = (int16_t)(ctx->k * _SIN(TO_RADIAN(ctx->x_prev)) + ctx->k + ctx->b);

    ctx->x_prev += trig_context->step;

    if(ctx->x_prev >= 360) ctx->x_prev = 0;

    return (uint16_t)result;
}
