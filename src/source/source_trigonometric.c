#include "source/source_trigonometric.h"

uint16_t get_value_trigonometric(struct source *source);
void reset_sequence_trigonometric(struct source *source);

struct source *source_init_trigonometric(struct source_config *config)
{
    struct source_trigonometric *result;
    struct source *s;
    struct source_config_function *config_function = (struct source_config_function *)config;
    struct source_config_trigonometric *config_trigonometric = (struct source_config_trigonometric *)config;

    if(config->type != SOURCE_TYPE_TRIGONOMETRIC)
    {
        return NULL;
    }

    result = (struct source_trigonometric *)malloc(sizeof(struct source_trigonometric));

    if(result == NULL)
    {
        return NULL;
    }

    s = (struct source *)result;

    s->magic = SOURCE_MAGIC_TRIGONOMETRIC;
    s->get_value = get_value_trigonometric;
    s->reset_sequence = reset_sequence_trigonometric;

    result->b = config_function->b;
    result->k = config_function->k;
    result->step_for_b = config_function->change_step_b;
    result->step_for_xn = config_function->change_step_k;
    result->y_max = config_function->y_max;
    result->sinus_calculate = config_trigonometric->hw_sinus;
    
    result->x_n = 0;

    return s;
}

uint16_t get_value_trigonometric(struct source *source)
{
    int16_t result;
    struct source_trigonometric *trigonometric;

    if(source->magic != SOURCE_MAGIC_TRIGONOMETRIC)
    {
        return 0;
    }

    trigonometric = (struct source_trigonometric *)source;

    result = (int16_t)(trigonometric->k * trigonometric->sinus_calculate(trigonometric->x_n + trigonometric->b) 
                    + trigonometric->k);

    trigonometric->x_n += trigonometric->step_for_xn;

    if(trigonometric->x_n > 360) trigonometric->x_n = 0;

    return (uint16_t)result;
}

void reset_sequence_trigonometric(struct source *source)
{
    struct source_trigonometric *t;

    if(source->magic != SOURCE_MAGIC_TRIGONOMETRIC)
    {
        return;
    }

    t = (struct source_trigonometric *)source;

    t->x_n = 0;

    t->b += t->step_for_b;

    t->b %= 360;
}