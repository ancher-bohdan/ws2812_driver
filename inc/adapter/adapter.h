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

struct adapter* adapter_init(struct ws2812_operation_fn_table *fn, enum supported_color_scheme scheme);
int adapter_set_source_originator_from_config(struct adapter *adapter, struct source_config *first, struct source_config *second, struct source_config *third);
void adapter_process(struct adapter *adapter);

#endif /*__ADAPTER_INIT__ */