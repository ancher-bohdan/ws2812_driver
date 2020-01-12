#include "adapter/adapter.h"

int init_adapter(struct ws2812_operation_fn_table *fn, struct adapter *adapter, enum supported_color_scheme scheme)
{
    adapter = (struct adapter *) malloc (sizeof(struct adapter));
    if(adapter == NULL)
    {
        return ENOMEM;
    }

    adapter->is_continue = false;
    adapter->flash_led_count = 0;
    
    if(scheme == RGB)
    {
        adapter->convert_to_dma = __rgb2dma;
    }
    else if(scheme == HSV)
    {
        adapter->convert_to_dma = __hsv2dma;
    }
    

    return ws2812_driver_init(fn, &adapter->base);
}

void adapter_process(struct adapter *adapter)
{
    adapter->is_continue = true;

    adapter->base.driver_start(&adapter->base);

    while(adapter->is_continue)
    {
        while(adapter->flash_led_count < adapter->base.led_count)
        {
            if(adapter->base.read != adapter->base.write
            || adapter->base.write->state == DRBUF_STATE_FREE)
            {
                uint8_t i = 0;
                for(i = 0; i < adapter->base.buffer_size; i++)
                {
                    adapter->base.write->color[i].first = adapter->originator_first_stub();
                    adapter->base.write->color[i].second = adapter->originator_second_stub();
                    adapter->base.write->color[i].third = adapter->originator_third_stub();

                    adapter->convert_to_dma(&(adapter->base.write->color[i]), adapter->base.write->buffer);
                    adapter->flash_led_count++;
                }

                adapter->base.write->state = DRBUF_STATE_BUSY;
                adapter->base.write = adapter->base.write->next;
            }
        }

        adapter->flash_led_count = 0;

        while(adapter->base.state != DR_STATE_SUSPEND);

        adapter->base.fn_table->hw_delay(500);
    }
}