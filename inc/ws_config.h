#ifndef __WS_CONFIG__
#define __WS_CONFIG__

#include ".config.h"

/* ----------------------
 * Config parameters for ring buffer
 * ---------------------*/

/* Number of LED`s in len strip */
#define LED_NUMBERS     CONFIG_LED_NUMBERS

/* Number of chain in ring buffer */
#ifdef CONFIG_BUFFER_COUNT_2
#define BUFFER_COUNT    2
#elif CONFIG_BUFFER_COUNT_4
#define BUFFER_COUNT    4
#endif

/* Size of each chain */
#define BUFFER_SIZE     CONFIG_BUFFER_SIZE

/* How many bytes need for each led */
#define WORDS_PER_LED   CONFIG_NUMBERS_PER_LED

/* Value, that represent bit 1 in __dma_buffer struct */
#define LED_CODE_ONE    CONFIG_LED_CODE_ONE

/* Value, that represent bit 0 in __dma_buffer struct */
#define LED_CODE_ZERO   CONFIG_LED_CODE_ZERO

#endif