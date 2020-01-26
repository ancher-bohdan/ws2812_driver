#ifndef __SOURCE_INIT___
#define __SOURCE_INIT___

#include <stdint.h>
#include <stdlib.h>

#define SOURCE_MAGIC_BASE   0xDEADBEEF

#define SOURCE_MAGIC_LINEAR         (SOURCE_MAGIC_BASE)
#define SOURCE_MAGIC_TRIGONOMETRIC  ((SOURCE_MAGIC_BASE) + 1)

struct source
{
    uint32_t magic;
    
    uint16_t (*get_value)(struct source *s);
    void (*reset_sequence)(struct source *s);
};

#endif /* __SOURCE_INIT___ */