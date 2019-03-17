#ifndef __WS_CONFIG__
#define __WS_CONFIG__

/* ----------------------
 * Config parameters for ring buffer
 * ---------------------*/

/* Number of LED`s in len strip */
#define LED_NUMBERS     144

/* Number of chain in ring buffer */
#define BUFFER_COUNT    2

/* Size of each chain */
#define BUFFER_SIZE     4

/* How many bytes need for each led */
#define WORDS_PER_LED   24

/* Value, that represent bit 1 in __dma_buffer struct */
#define LED_CODE_ONE    59

/* Value, that represent bit 0 in __dma_buffer struct */
#define LED_CODE_ZERO   26

/* ---------------------------------
 * Macros for parameters validation
 * !!!! DO NOT MODIFY !!!!
 * -------------------------------- */

#define NUMBER_OF_BUFFERS   (LED_NUMBERS / BUFFER_SIZE)

#if (NUMBER_OF_BUFFERS * BUFFER_SIZE < LED_NUMBERS)
#warning "Not all leds in led strip will be use. Modify config parameters, pls"
#endif

#if (NUMBER_OF_BUFFERS < BUFFER_COUNT)
#error "Redundant memory allocation. Correct config parameters, pls"
#endif

#endif