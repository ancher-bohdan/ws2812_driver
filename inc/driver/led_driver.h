#ifndef __LED_DRIVER__
#define __LED_DRIVER__

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "ws_config.h"

#define EOK             0
#define ENOMEM          -1
#define EINIT           -2
#define EPARAM          -3

#ifdef  USE_FULL_ASSERT
  #define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))

  void assert_failed(uint8_t* file, uint32_t line);
#else
  #define assert_param(expr) ((void)0U)
#endif

struct ws2812_driver_private;
struct __dma_buffer;

enum driver_state {
    DR_STATE_IDLE = 0,
    DR_STATE_SUSPEND,
    DR_STATE_RUNNING
};

enum driver_buffer_state {
    DRBUF_STATE_FREE = 0,
    DRBUF_STATE_BUSY
};

struct color_representation {
    uint16_t first;
    uint8_t second;
    uint8_t third;
};

struct driver_buffer_node {
    struct __dma_buffer *buffer;
    struct color_representation *color;
    struct driver_buffer_node *next;
    enum driver_buffer_state state;
};

struct ws2812_operation_fn_table {
    void (*hw_start_dma) (void *ptr, uint16_t size);
    void (*hw_stop_dma) ();
    void (*hw_start_timer) ();
    void (*hw_stop_timer) ();
    void (*hw_delay) (uint32_t delay);
};

struct ws2812_driver {
    
    struct driver_buffer_node *read;
    struct driver_buffer_node *write;
    struct driver_buffer_node *start;
    uint16_t buffer_size;
    uint8_t buffer_count;

    enum driver_state state;

    uint32_t led_count;

    struct ws2812_driver_private *priv;
    struct ws2812_operation_fn_table *fn_table;
    
    struct color_representation color[BUFFER_COUNT][BUFFER_SIZE];

    void (*timer_interrupt)(struct ws2812_driver *driver );
    void (*dma_interrupt)(struct ws2812_driver *driver );
    
    void (*driver_start)(struct ws2812_driver *driver);
    void (*driver_stop)(struct ws2812_driver *driver);
    void (*driver_suspend)(struct ws2812_driver *driver);
    void (*driver_resume)(struct ws2812_driver *driver);

};

int ws2812_driver_init(struct ws2812_operation_fn_table *fn, struct ws2812_driver *driver);

void __rgb2dma(struct driver_buffer_node *node, int offset);
void __hsv2dma(struct driver_buffer_node *node, int offset);

#endif //__LED_DRIVER__