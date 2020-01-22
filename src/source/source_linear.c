#include "source/source_linear.h"

uint16_t get_value_linear(struct source *source);
void source_linear_reset(struct source *s);

struct source * source_init_linear(struct source_config *config)
{
    struct source_linear *result = (struct source_linear *) malloc (sizeof(struct source_linear));
    
    if(result == NULL)
    {
        return NULL;
    }

    result->base.magic = SOURCE_MAGIC_LINEAR;
    
    result->base.get_value = get_value_linear;
    result->base.reset_sequence = source_linear_reset;
    
    result->base.step = config->change_step;

    result->is_converge = 1;
    result->x_n = 0;
    result->k = config->k;
    result->b = config->b;
    result->y_max = config->y_max;
    result->y_max_initial = config->y_max;

    return (struct source *)result;
}

uint16_t get_value_linear(struct source *source)
{
    int16_t result = 0;

    struct source_linear *linear;
    
    if(source->magic != SOURCE_MAGIC_LINEAR)
    {
        return 0;
    }

    linear = (struct source_linear *) source;

    if(linear->is_converge)
    {
        result = linear->k * linear->x_n + linear->b;
        if(result > linear->y_max)
        {
            linear->y_max = linear->k * (linear->x_n - 1 ) + linear->b;
            linear->is_converge = 0;
            linear->x_n = 1;
            goto diverge;
        }

        linear->x_n++;

        if(result == linear->y_max)
        {
            linear->is_converge = 0;
            linear->x_n = 1;
        }
        
        return (uint16_t)result;
    }
    else
    {
diverge:
        result = (-1) * linear->k * linear->x_n + linear->y_max;
        linear->x_n++;
        if(result == 0)
        {
            linear->is_converge = 1;
            linear->x_n = (-1) * (linear->b / linear->k);
            linear->x_n++;
        }
        else if(result < 1)
        {
            linear->is_converge = 1;
            linear->x_n = (-1) * (linear->b / linear->k);
            return 0;
        }
        
        return (uint16_t)result;
    }
}

void source_linear_reset(struct source *s)
{
    struct source_linear *linear;

    if(s->magic != SOURCE_MAGIC_LINEAR)
    {
        return;
    }

    linear = (struct source_linear *)s;

    linear->x_n = 0;
    linear->is_converge = 1;
    linear->y_max = linear->y_max_initial;

    linear->b += s->step;
    linear->b %= (linear->y_max_initial << 1);

    if(linear->b > linear->y_max_initial)
    {
        do
        {
            linear->y_max = linear->k * (--(linear->x_n)) + linear->b;
        } while (linear->y_max > linear->y_max_initial);
        
        linear->x_n *= (-1);
        linear->is_converge = 0;
    }
}