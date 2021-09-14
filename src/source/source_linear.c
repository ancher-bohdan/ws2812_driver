#include "source/source_linear.h"

uint16_t get_value_linear(struct source *source);
void source_linear_reset(struct source *s);

struct source * source_init_linear(struct source_config *config)
{
    struct source_linear *result;
    struct source *s;
    struct source_config_function *config_functional = (struct source_config_function *)config;
    
    if(config->type != SOURCE_TYPE_LINEAR)
    {
        return NULL;
    }
    
    result = (struct source_linear *) malloc (sizeof(struct source_linear));
    
    if(result == NULL)
    {
        return NULL;
    }

    s = (struct source *)result;

    s->magic = SOURCE_MAGIC_LINEAR;
    
    s->get_value = get_value_linear;
    s->reset_sequence = source_linear_reset;
    
    result->step_for_b = config_functional->change_step_b;
    result->step_for_k = config_functional->change_step_k;

    result->x_n = 0;
    result->k = config_functional->k;
    result->b = config_functional->b;
    result->y_max = config_functional->y_max;

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

    result = (((linear->k * linear->x_n++) + linear->b) % (linear->y_max << 1)) - linear->y_max;

    return ((result >> 15) + result) ^ (result >> 15);
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

    linear->b += linear->step_for_b;
    linear->b %= (linear->y_max << 1);

    linear->k += linear->step_for_k;
    linear->k %= linear->y_max;
}