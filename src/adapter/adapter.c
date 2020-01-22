#include "adapter/adapter.h"

static struct source_config default_config =
{
    .k = 0,
    .b = 100,
    .change_step = 0,
    .y_max = 255,
    .y_min = 0,
};

static int adapter_set_source_originator_default(struct adapter *adapter)
{
    adapter->aggregator = make_source_aggregator_from_config(&default_config, &default_config, &default_config);

    if(adapter->aggregator == NULL)
    {
        return ENOMEM;
    }

    return EOK;
}

struct adapter* adapter_init(struct ws2812_operation_fn_table *fn, enum supported_color_scheme scheme)
{
    struct adapter *adapter = (struct adapter *) malloc (sizeof(struct adapter));

    if(adapter == NULL)
    {
        return NULL;
    }

    adapter->is_continue = false;
    adapter->flash_led_count = 0;
    adapter->aggregator = NULL;
    
    if(scheme == RGB)
    {
        adapter->convert_to_dma = __rgb2dma;
    }
    else if(scheme == HSV)
    {
        adapter->convert_to_dma = __hsv2dma;
    }
    
    if(ws2812_driver_init(fn, &adapter->base))
    {
        free(adapter);
        return NULL;
    }

    return adapter;
}

int adapter_set_source_originator_from_config(struct adapter *adapter, struct source_config *first, struct source_config *second, struct source_config *third)
{
    if(adapter->aggregator != NULL)
    {
        source_aggregator_free(adapter->aggregator);
    }

    adapter->aggregator = make_source_aggregator_from_config(first, second, third);

    if(adapter->aggregator == NULL)
    {
        return ENOMEM;
    }

    return EOK;

}

void adapter_process(struct adapter *adapter)
{
    adapter->is_continue = true;

    if(adapter->aggregator == NULL)
    {
        if(adapter_set_source_originator_default(adapter))
        {
            return;
        }
    }

    adapter->base.driver_start(&adapter->base);

    while(adapter->is_continue)
    {
        adapter->base.dma_swallow_workaround(&adapter->base);

        while(adapter->flash_led_count < adapter->base.led_count)
        {
            if(adapter->base.read != adapter->base.write
            || adapter->base.write->state == DRBUF_STATE_FREE)
            {
                uint8_t i = 0;
                for(i = 0; i < adapter->base.buffer_size; i++)
                {
                    adapter->base.write->color[i].first = adapter->aggregator->first->get_value(adapter->aggregator->first);
                    adapter->base.write->color[i].second = adapter->aggregator->second->get_value(adapter->aggregator->second);
                    adapter->base.write->color[i].third = adapter->aggregator->third->get_value(adapter->aggregator->third);

                    adapter->convert_to_dma(adapter->base.write, i);
                    adapter->flash_led_count++;
                }

                //TODO: Possible race condition. Need lock
                adapter->base.write->state = DRBUF_STATE_BUSY;
                adapter->base.write = adapter->base.write->next;
            }
        }

        adapter->flash_led_count = 0;

        adapter->aggregator->first->reset_sequence(adapter->aggregator->first);
        adapter->aggregator->second->reset_sequence(adapter->aggregator->second);
        adapter->aggregator->third->reset_sequence(adapter->aggregator->third);

        while(adapter->base.state != DR_STATE_SUSPEND);

        adapter->base.write = adapter->base.start;
        adapter->base.read = adapter->base.start;

        adapter->base.fn_table->hw_delay(5);
    }
}