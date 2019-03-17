#ifndef WS2812_H_
#define WS2812_H_

#include "ws_config.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define EOK             0
#define ENOMEM          -1
#define EINIT           -2

#define TR_ALL_LEDSTRIP 0xFFFFFFFF


enum __led_buffer_state {
    LB_STATE_FREE = 0,
    LB_STATE_IN_PROGRESS,
    LB_STATE_BUSY,

    LB_COUNT_STATES,
};

struct __dma_buffer {
    uint32_t G[8];
    uint32_t R[8];
    uint32_t B[8];
};

struct __abstract {
    uint16_t first;
    uint8_t second;
    uint8_t third;
};

struct __rgb_buffer {
        uint16_t r;
        uint8_t g;
        uint8_t b;
};

struct __hsv_buffer {
        uint16_t h;
        uint8_t s;
        uint8_t v;
};

union __color {
    struct __rgb_buffer rgb;
    struct __hsv_buffer hsv;
    struct __abstract abstract;
};

struct __led_buffers {
    struct __dma_buffer dma_buffer[BUFFER_COUNT][BUFFER_SIZE];
    union  __color col[BUFFER_COUNT][BUFFER_SIZE];
};

struct __led_buffer_node {
    struct __dma_buffer *buffer;
    union  __color *col;
    struct __led_buffer_node *next;
    enum __led_buffer_state state;
};

struct ws2812_operation {
    void (*__start_dma_fnc)(void *ptr, uint16_t size);          /* Function for start dma transaction. This function is architecture dependent
                                                                    so it must be implemented by user and passed during driver initialisation.
                                                                    This function will be called by driver, when the former is ready to start DMA.
                                                                    It will pass pointer to data, that must be transfer via DMA and number of bytes */
    
    void (*__stop_dma_fnc)();                                   /* Function for stop or abort dma transaction */

    void (*to_dma)(union __color *in, struct __dma_buffer *dst); /* Pointer to function that can convert numerical representation of led color
                                                                    to DMA-ready form. Depends of actual meaning of first data. */
};

struct ws2812_list_handler {
struct __led_buffer_node *read;         /* Pointer to buffer node, that will be reading */
    struct __led_buffer_node *write;    /* Pointer to buffer node, that will be writing */
    struct __led_buffers buffer;        /* Actual memory for buffer`s nodes */

    struct ws2812_operation wops;       /* Function table of main operations with ring buffer */
};

enum supported_colors {
    RGB = 0,
    HSV
};

#ifdef  USE_FULL_ASSERT
  #define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))

  void assert_failed(uint8_t* file, uint32_t line);
#else
  #define assert_param(expr) ((void)0U)
#endif

int ws2812_initialise(void (*start_dma)(void *ptr, uint16_t size), void (*stop_dma)());
int ws2812_transfer_recurrent(char *r_exp, char *g_exp, char *b_exp, enum supported_colors scheme, uint32_t count);
void ws2812_interrupt();

#endif /* WS2812_H_ */
