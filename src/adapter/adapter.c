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

static int adapter_set_source_originator_default(struct adapter *adapter, bool isInInit)
{
    struct source_config *c = (struct source_config *)&default_config;

    int res = make_source_aggregator_from_config(&(adapter->aggregator), c, c, c);

    if(res == EOK && isInInit)
    {
        //default originator placed on inactive bank. Need to swap banks 
        AGGREGATOR_SWITCH_ACTIVE_BANK(adapter->aggregator);
        AGGREGATOR_CLEAR_BANK_SWITCHING_FLAG(adapter->aggregator);
    }
    return res;
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
    
    adapter->aggregator.flags = 0;
    adapter->aggregator.first[0] = NULL;
    adapter->aggregator.second[0] = NULL;
    adapter->aggregator.third[0] = NULL;
    adapter->aggregator.first[1] = NULL;
    adapter->aggregator.second[1] = NULL;
    adapter->aggregator.third[1] = NULL;
    adapter_set_source_originator_default(adapter, true);
    
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

void adapter_start(struct adapter *adapter)
{
    //switch originator`s bank if config changed during adapter was stoped
    if(AGGREGATOR_IS_BANK_SWITCHING_NEED(adapter->aggregator))
    {
        AGGREGATOR_SWITCH_ACTIVE_BANK(adapter->aggregator);
        AGGREGATOR_CLEAR_BANK_SWITCHING_FLAG(adapter->aggregator);
    }

    adapter->is_continue = true;
    adapter->base.driver_start(&adapter->base);
    adapter->base.dma_swallow_workaround(&(adapter->base));
}

void adapter_set_driver_id(struct adapter *adapter, uint32_t id)
{
    adapter->base.id = id;
}

void adapter_process(struct adapter **adapter, int ifnum)
{
    int i = 0;

    while(_is_any_adapter_continue_process(adapter, ifnum))
    {
        for(i = 0; i < ifnum; i++)
        {
            if(adapter[i]->is_continue)
            {
                uint8_t active_bank = AGGREGATOR_GET_ACTIVE_BANK(adapter[i]->aggregator);

                if(adapter[i]->flash_led_count < adapter[i]->base.led_count)
                {
                    if(adapter[i]->base.read != adapter[i]->base.write 
                    || adapter[i]->base.write->state == DRBUF_STATE_FREE)
                    {
                        uint8_t j = 0;
                        for(j = 0; j < adapter[i]->base.buffer_size; j++)
                        {
                            adapter[i]->base.write->color[j].first = adapter[i]->aggregator.first[active_bank]->get_value(adapter[i]->aggregator.first[active_bank]);
                            adapter[i]->base.write->color[j].second = adapter[i]->aggregator.second[active_bank]->get_value(adapter[i]->aggregator.second[active_bank]);
                            adapter[i]->base.write->color[j].third = adapter[i]->aggregator.third[active_bank]->get_value(adapter[i]->aggregator.third[active_bank]);

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

                    if(!adapter[i]->base.fn_table->hw_delay(adapter[i]->base.id, adapter[i]->hw_delay))
                    {
                        continue;
                    }

                    adapter[i]->aggregator.first[active_bank]->reset_sequence(adapter[i]->aggregator.first[active_bank]);
                    adapter[i]->aggregator.second[active_bank]->reset_sequence(adapter[i]->aggregator.second[active_bank]);
                    adapter[i]->aggregator.third[active_bank]->reset_sequence(adapter[i]->aggregator.third[active_bank]);

                    adapter[i]->base.write = adapter[i]->base.read = adapter[i]->base.start;

                    adapter[i]->flash_led_count = 0;
                    adapter[i]->base.dma_swallow_workaround(&(adapter[i]->base));
                    
                    if(AGGREGATOR_IS_BANK_SWITCHING_NEED(adapter[i]->aggregator))
                    {
                        AGGREGATOR_SWITCH_ACTIVE_BANK(adapter[i]->aggregator);
                        AGGREGATOR_CLEAR_BANK_SWITCHING_FLAG(adapter[i]->aggregator);
                    }
                }
            }
        }
    }
}