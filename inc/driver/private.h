#ifndef __DRIVER_PRIVATE__
#define __DRIVER_PRIVATE__

#include <stdint.h>

#include "ws_config.h"

struct color_representation;

struct __dma_buffer {
#ifdef DMA_TRANSACTION_32
    uint32_t G[8];
    uint32_t R[8];
    uint32_t B[8];
#elif DMA_TRANSACTION_16
    uint16_t G[8];
    uint16_t R[8];
    uint16_t B[8];
#else
    #error "Define size of DMA transaction"
#endif
};


struct ws2812_driver_private
{
    struct __dma_buffer dma_buffer[BUFFER_COUNT][BUFFER_SIZE];
};

#endif /* __DRIVER_PRIVATE__ */