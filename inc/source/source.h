#ifndef __SOURCE_INIT___
#define __SOURCE_INIT___

#include <stdint.h>
#include <stdlib.h>

struct source
{
    uint32_t magic;
    
    uint16_t (*get_value)(struct source *s);
    void (*reset_sequence)(struct source *s);
};

#endif /* __SOURCE_INIT___ */