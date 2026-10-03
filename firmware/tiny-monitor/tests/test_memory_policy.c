#include "memory_policy.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    const char *reason = 0;
    uint32_t bytes = 0;

    assert(memory_range_valid(UINT32_C(0x20000000), 4U, 4U, &bytes) && bytes == 16U);
    assert(!memory_range_valid(UINT32_C(0xfffffffc), 2U, 4U, &bytes));
    assert(memory_read_allowed(UINT32_C(0x00000000), 1U, 4U, &reason));
    assert(memory_read_allowed(UINT32_C(0x0800f7fc), 1U, 4U, &reason));
    assert(!memory_read_allowed(UINT32_C(0x0800f800), 1U, 4U, &reason));
    assert(memory_read_allowed(UINT32_C(0x20001000), 1U, 4U, &reason));
    assert(!memory_read_allowed(UINT32_C(0x20001400), 1U, 4U, &reason));
    assert(memory_write_allowed(UINT32_C(0x20000000), 1024U, 4U, &reason));
    assert(!memory_write_allowed(UINT32_C(0x20000ffc), 2U, 4U, &reason));
    assert(!memory_write_allowed(UINT32_C(0x20001000), 1U, 4U, &reason));
    assert(!memory_write_allowed(UINT32_C(0x08000000), 1U, 4U, &reason));
    assert(!memory_write_allowed(UINT32_C(0x40010800), 1U, 4U, &reason));
    assert(memory_read_allowed(UINT32_C(0x40010808), 1U, 4U, &reason));
    assert(!memory_read_allowed(UINT32_C(0x40013804), 1U, 4U, &reason));
    assert(!memory_read_allowed(UINT32_C(0x20000001), 1U, 2U, &reason));
    puts("memory policy tests passed");
    return 0;
}
