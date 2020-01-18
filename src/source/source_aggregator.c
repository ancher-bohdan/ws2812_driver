#include "source/source_aggregator.h"

struct source_aggregator *make_source_aggregator_from_config(struct source_config *first, struct source_config *second, struct source_config *third)
{
    struct source_aggregator *result = (struct source_aggregator *) malloc(sizeof(struct source_aggregator));
    
    if(result == NULL)
    {
        return NULL;
    }

    result->first = source_init_linear(first);
    if(result->first == NULL)
    {
        free(result);
        return NULL;
    }

    result->second = source_init_linear(second);
    if(result->second == NULL)
    {
        free(result->first);
        free(result);
        return NULL;
    }

    result->third = source_init_linear(third);
    if(result->third == NULL)
    {
        free(result->first);
        free(result->second);
        free(result);
        return NULL;
    }

    return result;
}