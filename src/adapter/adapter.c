#include "adapter/adapter.h"
#include "source/source.h"


static struct source_config_function default_config =
{
    .base.type = SOURCE_TYPE_LINEAR,
    .k = 0,
    .b = 100,
    .change_step_b = 0,
    .change_step_k = 0,
    .y_max = 255,
    .y_min = 0,
};

static int adapter_set_source_originator_default(struct adapter *adapter)
{
    struct source_config *c = (struct source_config *)&default_config;

    adapter->aggregator = make_source_aggregator_from_config(c, c, c);

    if(adapter->aggregator == NULL)
    {
        return ENOMEM;
    }

    return EOK;
}

struct adapter* adapter_init(struct ws2812_operation_fn_table *fn, enum supported_color_scheme scheme, uint32_t delay)
{
    struct adapter *adapter = (struct adapter *) malloc (sizeof(struct adapter));

    if(adapter == NULL)
    {
        return NULL;
    }

    adapter->is_continue = false;
    adapter->flash_led_count = 0;
    adapter->hw_delay = delay;

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
    struct ws2812_driver *driver = (struct ws2812_driver *)adapter;

    adapter->is_continue = true;

    if(adapter->aggregator == NULL)
    {
        if(adapter_set_source_originator_default(adapter))
        {
            return;
        }
    }

    driver->driver_start(driver);

    while(adapter->is_continue)
    {
        driver->dma_swallow_workaround(driver);

        while(adapter->flash_led_count < driver->led_count)
        {
            if(driver->read != driver->write
            || driver->write->state == DRBUF_STATE_FREE)
            {
                uint8_t i = 0;
                for(i = 0; i < adapter->base.buffer_size; i++)
                {
                    driver->write->color[i].first = adapter->aggregator->first->get_value(adapter->aggregator->first);
                    driver->write->color[i].second = adapter->aggregator->second->get_value(adapter->aggregator->second);
                    driver->write->color[i].third = adapter->aggregator->third->get_value(adapter->aggregator->third);

                    adapter->convert_to_dma(driver->write, i);
                    adapter->flash_led_count++;
                }

                //TODO: Possible race condition. Need lock
                driver->write->state = DRBUF_STATE_BUSY;
                driver->write = driver->write->next;
            }
        }

        adapter->flash_led_count = 0;

        adapter->aggregator->first->reset_sequence(adapter->aggregator->first);
        adapter->aggregator->second->reset_sequence(adapter->aggregator->second);
        adapter->aggregator->third->reset_sequence(adapter->aggregator->third);

        while(adapter->base.state != DR_STATE_SUSPEND);

        driver->write = driver->read = driver->start;

        driver->fn_table->hw_delay(adapter->hw_delay);
    }
}