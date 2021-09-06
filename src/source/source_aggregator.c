#include "source/source_linear.h"
#include "source/source_trigonometric.h"
#include "source/source_music.h"

static struct source *make_source_from_config(struct source_config *config)
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
    uint8_t inactive_bank = !AGGREGATOR_GET_ACTIVE_BANK(*aggregator);

    source_aggregator_free(aggregator);

    aggregator->first[inactive_bank] = make_source_from_config(first);
    if(aggregator->first[inactive_bank] == NULL)
    {
        goto err_no_del;
    }

    aggregator->second[inactive_bank] = make_source_from_config(second);
    if(aggregator->second[inactive_bank] == NULL)
    {
        goto err_first_del;
    }

    aggregator->third[inactive_bank] = make_source_from_config(third);
    if(aggregator->third[inactive_bank] == NULL)
    {
        goto err_second_del;
    }

    AGGREGATOR_SET_BANK_SWITCHING_FLAG(*aggregator);

    return 0;

err_second_del:
    free(aggregator->second[inactive_bank]);
err_first_del:
    free(aggregator->first[inactive_bank]);
err_no_del:
    return -1;
}

void source_aggregator_free(struct source_aggregator *aggregator)
{
    uint8_t inactive_bank = !AGGREGATOR_GET_ACTIVE_BANK(*aggregator);
    
    if(aggregator == NULL)
    {
        return;
    }

    if(aggregator->first[inactive_bank] != NULL)
    {
        free(aggregator->first[inactive_bank]);
    }

    if(aggregator->second[inactive_bank] != NULL)
    {
        free(aggregator->second[inactive_bank]);
    }

    if(aggregator->third[inactive_bank] != NULL)
    {
        free(aggregator->third[inactive_bank]);
    }
}