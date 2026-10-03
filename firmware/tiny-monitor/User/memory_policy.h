#ifndef MEMORY_POLICY_H
#define MEMORY_POLICY_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t start;
    uint32_t end;
    const char *name;
    const char *access;
} memory_region_t;

int memory_range_valid(uint32_t address, uint32_t count, uint32_t width, uint32_t *bytes);
int memory_read_allowed(uint32_t address, uint32_t count, uint32_t width, const char **reason);
int memory_write_allowed(uint32_t address, uint32_t count, uint32_t width, const char **reason);
const memory_region_t *memory_physical_regions(size_t *count);
const memory_region_t *memory_mmio_regions(size_t *count);

#endif
