#include "os_mem.h"
#include "esp_heap_caps.h"
#include <stdlib.h>
void *os_mem_malloc(size_t size)
{
    if (size > 4096) { return heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT); }
    return malloc(size);
}
void os_mem_free(void *ptr) { free(ptr); }
void *os_mem_calloc(size_t num, size_t size)
{
    if ((num * size) > 4096) { return heap_caps_calloc(num, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT); }
    return calloc(num, size);
}
size_t os_mem_get_free(void)
{
    return heap_caps_get_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}
