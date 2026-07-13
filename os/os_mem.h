#ifndef OS_MEM_H
#define OS_MEM_H
#include <stddef.h>
void *os_mem_malloc(size_t size);
void os_mem_free(void *ptr);
void *os_mem_calloc(size_t num, size_t size);
size_t os_mem_get_free(void);
#endif
