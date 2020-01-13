#include "driver/led_driver.h"
#include "driver/private.h"

#include <math.h>

void dma_interrupt_routine(struct ws2812_driver *driver);
void tim_interrupt_routine(struct ws2812_driver *driver);
void ws2812_driver_suspend(struct ws2812_driver *driver);
void ws2812_driver_resume(struct ws2812_driver *driver);
void ws2812_driver_start(struct ws2812_driver *driver);
void ws2812_driver_stop(struct ws2812_driver *driver);


static bool check_all_buffer_node_state(struct ws2812_driver *driver, enum driver_buffer_state expected_state)
{
    struct driver_buffer_node *iterator = driver->start;
    bool result = (iterator->state == expected_state);

    while(result && (iterator->next != driver->start))
    {
        iterator = iterator->next;
        result = (iterator->state == expected_state);
    }
    return result;
}

static struct driver_buffer_node **__alloc_ring_buffer(struct ws2812_driver *driver, struct driver_buffer_node **prev)
{
    static int recursion_count = 0;

    *prev = (struct driver_buffer_node *)malloc(sizeof(struct driver_buffer_node));

    if(*prev == NULL)
        return NULL;

    (*prev)->buffer = driver->priv->dma_buffer[recursion_count];
    (*prev)->color = driver->color[recursion_count];
    (*prev)->state = DRBUF_STATE_FREE;

    if(recursion_count != (BUFFER_COUNT - 1)) {
        recursion_count++;
        return __alloc_ring_buffer(driver, &((*prev)->next));
    } else {
        return prev;
    }
}

int ws2812_driver_init(struct ws2812_operation_fn_table *fn, struct ws2812_driver *driver)
{
    struct driver_buffer_node **tmp = NULL;

    if(fn == NULL)
    {
        return EPARAM;
    }

    driver->state = DR_STATE_IDLE;
    driver->buffer_size = BUFFER_SIZE;
    driver->buffer_count = BUFFER_COUNT;
    driver->led_count = LED_NUMBERS;
    driver->fn_table = fn;

    driver->driver_start = ws2812_driver_start;
    driver->driver_stop = ws2812_driver_stop;
    driver->driver_resume = ws2812_driver_resume;
    driver->driver_suspend = ws2812_driver_suspend;
    
    driver->timer_interrupt = tim_interrupt_routine;
    driver->dma_interrupt = dma_interrupt_routine;

    driver->priv = (struct ws2812_driver_private *)malloc(sizeof(struct ws2812_driver_private));

    if(driver->priv == NULL)
    {
        free(driver);
        return ENOMEM;
    }

    tmp = __alloc_ring_buffer(driver, &(driver->start));

    if(tmp == NULL)
    {
        //TODO: need to clean all driver_buffer_node, that have been created.
        free(driver->priv);
        free(driver);
    }

    (*tmp)->next = driver->start;
    driver->write = driver->start;
    driver->read = driver->start;

    return EOK;
}

void dma_interrupt_routine(struct ws2812_driver *driver)
{
    assert_param(driver->read->state == DRBUF_STATE_BUSY);

    driver->read->state = DRBUF_STATE_FREE;
    driver->read = driver->read->next;
    
    if(driver->read->state != DRBUF_STATE_BUSY )
    {
        if(check_all_buffer_node_state(driver, DRBUF_STATE_FREE))
        {
            driver->driver_suspend(driver);
        }
        else
        {
            assert_param(0);
        }
    }
}

void tim_interrupt_routine(struct ws2812_driver *driver)
{
    if(check_all_buffer_node_state(driver, DRBUF_STATE_BUSY))
    {
        driver->driver_resume(driver);
    }
}

void ws2812_driver_suspend(struct ws2812_driver *driver)
{
    driver->fn_table->hw_stop_dma();

    driver->state = DR_STATE_SUSPEND;

    driver->fn_table->hw_start_timer();
}

void ws2812_driver_resume(struct ws2812_driver *driver)
{
    driver->fn_table->hw_stop_timer();

    driver->state = DR_STATE_RUNNING;
    driver->read = driver->write = driver->start;

    driver->fn_table->hw_start_dma(driver->read->buffer, driver->buffer_count * driver->buffer_size * WORDS_PER_LED);
}

void ws2812_driver_start(struct ws2812_driver *driver)
{
    driver->state = DR_STATE_SUSPEND;
    
    driver->fn_table->hw_start_timer();
}

void ws2812_driver_stop(struct ws2812_driver *driver)
{
    if(driver->state == DR_STATE_RUNNING)
    {
        driver->fn_table->hw_stop_dma();
    }
    else if(driver->state == DR_STATE_SUSPEND)
    {
        driver->fn_table->hw_stop_timer();
    }
    driver->state = DR_STATE_IDLE;
}

void __rgb2dma(struct driver_buffer_node *node, int offset)
{
    uint8_t i;
    struct color_representation *in = &(node->color[offset]);
    struct __dma_buffer *dst = &(node->buffer[offset]);

    for(i = 0; i < 8; i++)
    {
        dst->R[7 - i] = ((in->first) & (1 << i)) ? LED_CODE_ONE : LED_CODE_ZERO;
        dst->G[7 - i] = ((in->second) & (1 << i)) ? LED_CODE_ONE : LED_CODE_ZERO;
        dst->B[7 - i] = ((in->third) & (1 << i)) ? LED_CODE_ONE : LED_CODE_ZERO;
    }
}

void __hsv2rgb(struct color_representation *src, struct color_representation *dst)
{
    double c, x, m;
    double s_scale = src->second / 100.0;
    double v_scale = src->third / 100.0;

    c = v_scale * s_scale;
    m = v_scale - c;
    x = c * (1 - fabs(fmod(src->first / 60.0f, 2) - 1));

    switch(src->first/60)
    {
        case 0 :
            dst->first = (uint16_t)((c + m) * 255);
            dst->second = (uint8_t)((x + m) * 255);
            dst->third = (uint8_t)(m * 255);
        break;
        case 1:
            dst->first = (uint16_t)((x + m) * 255);
            dst->second = (uint8_t)((c + m) * 255);
            dst->third = (uint8_t)(m * 255);
        break;
        case 2:
            dst->first = (uint16_t)(m * 255);
            dst->second = (uint8_t)((c + m) * 255);
            dst->third = (uint8_t)((x + m) * 255);
        break;
        case 3:
            dst->first = (uint16_t)(m * 255);
            dst->second = (uint8_t)((x + m) * 255);
            dst->third = (uint8_t)((c + m) * 255);
        break;
        case 4:
            dst->first = (uint16_t)((x + m) * 255);
            dst->second = (uint8_t)(m * 255);
            dst->third = (uint8_t)((c + m) * 255);
        break;
        case 5:
        case 6:
            dst->first = (uint16_t)((c + m) * 255);
            dst->second = (uint8_t)(m * 255);
            dst->third = (uint8_t)((x + m) * 255);
        break;
    }
}

void __hsv2dma(struct driver_buffer_node *node, int offset)
{
    uint8_t i;
    struct color_representation *in = &(node->color[offset]);
    struct __dma_buffer *dst = &(node->buffer[offset]);
    struct color_representation tmp;

    __hsv2rgb(in, &tmp);
    
    for(i = 0; i < 8; i++)
    {
        dst->R[7 - i] = ((tmp.first) & (1 << i)) ? LED_CODE_ONE : LED_CODE_ZERO;
        dst->G[7 - i] = ((tmp.second) & (1 << i)) ? LED_CODE_ONE : LED_CODE_ZERO;
        dst->B[7 - i] = ((tmp.third) & (1 << i)) ? LED_CODE_ONE : LED_CODE_ZERO;
    }
}
