#ifndef __ADAPTER_INIT__
#define __ADAPTER_INIT__

#include "driver/led_driver.h"
#include "source/source_aggregator.h"

#define A_EOK       0
#define A_NOT_PERM  -1

enum adapter_state
{
    ADAPTER_STATE_INIT = 0,
    ADAPTER_STATE_RUNNING,
    ADAPTER_STATE_STOP_REQUESTED,
    ADAPTER_STATE_STOP,
    ADAPTER_STATE_CLEAR_REQUESTED,
    ADAPTER_STATE_CLEARING,
    ADAPTER_STATE_SET_HSV_SCHEME_REQUESTED,
    ADAPTER_STATE_SET_RGB_SCHEME_REQUESTED
};

struct adapter
{
    struct ws2812_driver base;

    uint32_t flash_led_count;
    uint32_t hw_delay;
    uint32_t requested_led_count;

    struct source *backup_sources[3];

    bool is_continue;
    enum adapter_state state;

    void (*convert_to_dma)(struct driver_buffer_node *node, int offset);

    struct source_aggregator aggregator;
};

enum supported_color_scheme {
    RGB = 0,
    HSV
};

struct adapter* adapter_init(struct ws2812_operation_fn_table *fn, enum supported_color_scheme scheme, uint32_t led_count, uint32_t delay);

void adapter_start(struct adapter *adapter);
void adapter_stop(struct adapter *adapter);

void adapter_set_driver_id(struct adapter *adapter, uint32_t id);
void adapter_set_hw_delay(struct adapter *adapter, uint32_t delay);
int adapter_set_if_up(struct adapter *adapter);
int adapter_set_if_down(struct adapter *adapter);
int adapter_set_color_scheme(struct adapter *adapter, enum supported_color_scheme scheme);
int adapter_set_led_count(struct adapter *adapter, uint32_t ledcount);

void adapter_process(struct adapter **adapters, int ifnum);

#endif /*__ADAPTER_INIT__ */