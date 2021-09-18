#include "source/source_linear.h"
#include "source/source_trigonometric.h"
#include "source/source_music.h"

#include <stdio.h>

struct source *make_source_from_config(struct source_config *config)
{
    switch (config->type)
    {
    case SOURCE_TYPE_LINEAR:
        return source_init_linear(config);
    case SOURCE_TYPE_TRIGONOMETRIC:
        return source_init_trigonometric(config);
    case SOURCE_TYPE_MUSIC:
        return source_init_music(config);
    default:
        return NULL;
    }
}

int make_source_aggregator_from_config(struct source_aggregator *aggregator, struct source_config *first, struct source_config *second, struct source_config *third)
{
    uint8_t inactive_bank;

    if(first != NULL)
    {
        inactive_bank = !AGGREGATOR_GET_ACTIVE_BANK(*aggregator, 0);

        source_aggregator_free(aggregator, 0);

        aggregator->first[inactive_bank] = make_source_from_config(first);
        if(aggregator->first[inactive_bank] == NULL)
        {
            return -1;
        }
        
        AGGREGATOR_SET_BANK_SWITCHING_FLAG(*aggregator, 0);
    }

    if(second != NULL)
    {
        inactive_bank = !AGGREGATOR_GET_ACTIVE_BANK(*aggregator, 1);

        source_aggregator_free(aggregator, 1);

        aggregator->second[inactive_bank] = make_source_from_config(second);
        if(aggregator->second[inactive_bank] == NULL)
        {
            return -1;
        }

        AGGREGATOR_SET_BANK_SWITCHING_FLAG(*aggregator, 1);
    }

    if(third != NULL)
    {
        inactive_bank = !AGGREGATOR_GET_ACTIVE_BANK(*aggregator, 2);

        source_aggregator_free(aggregator, 2);

        aggregator->third[inactive_bank] = make_source_from_config(third);
        if(aggregator->third[inactive_bank] == NULL)
        {
            return -1;
        }

        AGGREGATOR_SET_BANK_SWITCHING_FLAG(*aggregator, 2);
    }

    return 0;
}

void source_aggregator_free(struct source_aggregator *aggregator, uint8_t config_num)
{
    uint8_t inactive_bank = !AGGREGATOR_GET_ACTIVE_BANK(*aggregator, config_num);
    
    if(aggregator == NULL)
    {
        return;
    }

    switch(config_num)
    {
        case 0:
            if(aggregator->first[inactive_bank] != NULL)
            {
                free(aggregator->first[inactive_bank]);
            }
            break;
        case 1:
            if(aggregator->second[inactive_bank] != NULL)
            {
                free(aggregator->second[inactive_bank]);
            }
            break;
        case 2:
            if(aggregator->third[inactive_bank] != NULL)
            {
                free(aggregator->third[inactive_bank]);
            }
            break;
        default:
            return;

    }
}

int get_source_description(char *dst, struct source *s)
{
    struct source_linear *lin;
    struct source_trigonometric *trig;
    switch(s->magic)
    {
        case SOURCE_MAGIC_LINEAR:
            lin = (struct source_linear *)s;
            return sprintf(dst, "LINEAR: %dx+%d; y_max=%d; db=%d; dk=%d", lin->k, lin->b, lin->y_max, lin->step_for_b, lin->step_for_k);
            break;
        case SOURCE_MAGIC_TRIGONOMETRIC:
            trig = (struct source_trigonometric *)s;
            return sprintf(dst, "TRIGONOMETRIC: %d*sinx+%d; step_for_x=%d", trig->k, trig->b, trig->step_for_xn);
            break;
        case SOURCE_MAGIC_MUSIC:
            return sprintf(dst, "MUSIC");
            break;
    }
    return 0;
}