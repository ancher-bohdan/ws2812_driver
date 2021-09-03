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

static bool _is_any_adapter_continue_process(struct adapter **adapters, int ifnum)
{
    int i = 0;
    for(i = 0; i < ifnum; i++)
    {
        if(adapters[i]->is_continue)
        {
            return true;
        }
    }
    return false;
}

struct adapter* adapter_init(struct ws2812_operation_fn_table *fn, enum supported_color_scheme scheme, uint32_t led_count, uint32_t delay)
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
    
    if(ws2812_driver_init(fn, led_count, &adapter->base))
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

void adapter_start(struct adapter *adapter)
{
    adapter->is_continue = true;
    adapter->base.driver_start(&adapter->base);
}

void adapter_process(struct adapter **adapter, int ifnum)
{
    int i = 0;

    for(i = 0; i < ifnum; i++)
    {
        if(adapter[i]->aggregator == NULL)
        {
            if(adapter_set_source_originator_default(adapter[i]))
            {
                return;
            }
        }
        adapter[i]->base.dma_swallow_workaround(&(adapter[i]->base));
    }

    while(_is_any_adapter_continue_process(adapter, ifnum))
    {
        for(i = 0; i < ifnum; i++)
        {
            if(adapter[i]->is_continue)
            {                
                if(adapter[i]->flash_led_count < adapter[i]->base.led_count)
                {
                    if(adapter[i]->base.read != adapter[i]->base.write 
                    || adapter[i]->base.write->state == DRBUF_STATE_FREE)
                    {
                        uint8_t j = 0;
                        for(j = 0; j < adapter[i]->base.buffer_size; j++)
                        {
                            adapter[i]->base.write->color[j].first = adapter[i]->aggregator->first->get_value(adapter[i]->aggregator->first);
                            adapter[i]->base.write->color[j].second = adapter[i]->aggregator->second->get_value(adapter[i]->aggregator->second);
                            adapter[i]->base.write->color[j].third = adapter[i]->aggregator->third->get_value(adapter[i]->aggregator->third);

                            adapter[i]->convert_to_dma(adapter[i]->base.write, j);
                            adapter[i]->flash_led_count++;
                        }

                        adapter[i]->base.write->state = DRBUF_STATE_BUSY;
                        adapter[i]->base.write = adapter[i]->base.write->next;
                    }
                }
                else // data for all leds in current ledstrip is ready. Perform finish routine
                {
                    if(adapter[i]->base.state != DR_STATE_SUSPEND)
                    {
                        continue;
                    }

                    if(!adapter[i]->base.fn_table->hw_delay(adapter[i]->hw_delay))
                    {
                        continue;
                    }

                    adapter[i]->aggregator->first->reset_sequence(adapter[i]->aggregator->first);
                    adapter[i]->aggregator->second->reset_sequence(adapter[i]->aggregator->second);
                    adapter[i]->aggregator->third->reset_sequence(adapter[i]->aggregator->third);

                    adapter[i]->base.write = adapter[i]->base.read = adapter[i]->base.start;

                    adapter[i]->flash_led_count = 0;
                    adapter[i]->base.dma_swallow_workaround(&(adapter[i]->base));
                }
            }
        }
    }
}