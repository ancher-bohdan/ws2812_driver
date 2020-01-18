#ifndef __ADAPTER_INIT__
#define __ADAPTER_INIT__

#include "driver/led_driver.h"
#include "source/source_aggregator.h"

struct adapter
{
    struct ws2812_driver base;

    uint32_t flash_led_count;

    bool is_continue;

    void (*convert_to_dma)(struct driver_buffer_node *node, int offset);

    struct source_aggregator *aggregator;
};

enum supported_color_scheme {
    RGB = 0,
    HSV
};

int init_adapter(struct ws2812_operation_fn_table *fn, struct adapter **out_adapter, enum supported_color_scheme scheme, struct source_aggregator *aggregator);
void adapter_process(struct adapter *adapter);

#endif /*__ADAPTER_INIT__ */