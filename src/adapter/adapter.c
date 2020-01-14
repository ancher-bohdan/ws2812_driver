#include "adapter/adapter.h"

uint16_t first_stub()
{
    return 255;
}

uint8_t second_stub()
{
    return 0;
}

uint8_t third_stub()
{
    return 0;
}

int init_adapter(struct ws2812_operation_fn_table *fn, struct adapter **out_adapter, enum supported_color_scheme scheme)
{
    int result = 0;
    struct adapter *adapter = (struct adapter *) malloc (sizeof(struct adapter));

    if(adapter == NULL)
    {
        return ENOMEM;
    }

    adapter->is_continue = false;
    adapter->flash_led_count = 0;
    
    adapter->originator_first_stub = first_stub;
    adapter->originator_second_stub = second_stub;
    adapter->originator_third_stub = third_stub;
    
    if(scheme == RGB)
    {
        adapter->convert_to_dma = __rgb2dma;
    }
    else if(scheme == HSV)
    {
        adapter->convert_to_dma = __hsv2dma;
    }
    
    result = ws2812_driver_init(fn, &adapter->base);
    
    *out_adapter = adapter;

    return result;
}

void adapter_process(struct adapter *adapter)
{
    adapter->is_continue = true;

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
                    adapter->base.write->color[i].first = adapter->originator_first_stub();
                    adapter->base.write->color[i].second = adapter->originator_second_stub();
                    adapter->base.write->color[i].third = adapter->originator_third_stub();

                    adapter->convert_to_dma(adapter->base.write, i);
                    adapter->flash_led_count++;
                }

                //TODO: Possible race condition. Need lock
                adapter->base.write->state = DRBUF_STATE_BUSY;
                adapter->base.write = adapter->base.write->next;
            }
        }

        adapter->flash_led_count = 0;

        while(adapter->base.state != DR_STATE_SUSPEND);

        adapter->base.write = adapter->base.start;
        adapter->base.read = adapter->base.start;

        adapter->base.fn_table->hw_delay(500);
    }
}