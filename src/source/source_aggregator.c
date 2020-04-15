#include "source/source_linear.h"
#include "source/source_trigonometric.h"
#include "source/source_music.h"

static struct source *make_source_from_config(struct source_config *config)
{
    switch (config->type)
    {
    case SOURCE_TYPE_LINEAR:
        return source_init_linear(config);
        break;
    case SOURCE_TYPE_TRIGONOMETRIC:
        return source_init_trigonometric(config);
    case SOURCE_TYPE_MUSIC:
        return source_init_music(config);
    default:
        return NULL;
    }
}

struct source_aggregator *make_source_aggregator_from_config(struct source_config *first, struct source_config *second, struct source_config *third)
{
    struct source_aggregator *result = (struct source_aggregator *) malloc(sizeof(struct source_aggregator));
    
    if(result == NULL)
    {
        goto err_no_del;
    }

    result->first = make_source_from_config(first);
    if(result->first == NULL)
    {
        goto err_aggr_del;
    }

    result->second = make_source_from_config(second);
    if(result->second == NULL)
    {
        goto err_first_del;
    }

    result->third = make_source_from_config(third);
    if(result->third == NULL)
    {
        goto err_second_del;
    }

    return result;

err_second_del:
    free(result->second);
err_first_del:
    free(result->first);
err_aggr_del:
    free(result);
err_no_del:
    return NULL;
}

void source_aggregator_free(struct source_aggregator *aggregator)
{
    if(aggregator == NULL)
    {
        return;
    }

    if(aggregator->first != NULL)
    {
        free(aggregator->first);
    }

    if(aggregator->second != NULL)
    {
        free(aggregator->second);
    }

    if(aggregator->third != NULL)
    {
        free(aggregator->third);
    }

    free(aggregator);
}