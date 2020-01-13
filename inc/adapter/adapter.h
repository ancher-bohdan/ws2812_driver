#ifndef __ADAPTER_INIT__
#define __ADAPTER_INIT__

#include "driver/led_driver.h"

struct adapter
{
    struct ws2812_driver base;

    uint32_t flash_led_count;

    bool is_continue;

    void (*convert_to_dma)(struct driver_buffer_node *node, int offset);

    //TODO: Replace stubs by originator object, when originator layer will be ready
    uint16_t (*originator_first_stub)();
    uint8_t (*originator_second_stub)();
    uint8_t (*originator_third_stub)();
};

enum supported_color_scheme {
    RGB = 0,
    HSV
};

int init_adapter(struct ws2812_operation_fn_table *fn, struct adapter **out_adapter, enum supported_color_scheme scheme);
void adapter_process(struct adapter *adapter);

#endif /*__ADAPTER_INIT__ */