#ifdef HOST_TEST
#include "pin_policy.h"
#include <assert.h>
#include <stdio.h>

static void expect_policy(const char *name, pin_policy_t expected)
{
    const board_pin_t *pin = pin_policy_find(name);
    assert(pin != 0 && pin->policy == expected);
}
int main(void)
{
    const board_pin_t *pb4;
    expect_policy("PC0", PIN_USER); expect_policy("PC3", PIN_USER);
    expect_policy("PB4", PIN_USER); expect_policy("PB8", PIN_USER);
    expect_policy("PB9", PIN_USER); expect_policy("PB12", PIN_USER); expect_policy("PC14", PIN_USER);
    expect_policy("PA2", PIN_INPUT_ONLY); expect_policy("PA3", PIN_INPUT_ONLY);
    expect_policy("PB0", PIN_LOCKED); expect_policy("PB1", PIN_LOCKED); expect_policy("PB5", PIN_LOCKED);
    expect_policy("PC16", PIN_LOCKED); expect_policy("PC17", PIN_LOCKED);
    expect_policy("PC18", PIN_LOCKED); expect_policy("PC19", PIN_LOCKED);
    expect_policy("PB10", PIN_BOARD_FUNCTION); expect_policy("PA5", PIN_BOARD_FUNCTION);
    expect_policy("PB6", PIN_BOARD_FUNCTION); assert(pin_policy_find("PD0") == 0);
    pb4 = pin_policy_find("pb4"); assert(pb4 != 0);
    assert(pin_policy_mode_request_valid(pb4, "input", 0, 0));
    assert(pin_policy_mode_request_valid(pb4, "pullup", 0, 0));
    assert(!pin_policy_mode_request_valid(pb4, "pulldown", 0, 0));
    assert(!pin_policy_mode_request_valid(pb4, "output", 0, 0));
    assert(pin_policy_mode_request_valid(pb4, "output", 1, 0));
    assert(pin_policy_mode_request_valid(pb4, "output", 1, 1));
    assert(!pin_policy_mode_request_valid(pb4, "output", 1, 2));
    assert(!pin_policy_mode_request_valid(pb4, "invalid", 0, 0));
    assert(!pin_policy_mode_request_valid(pin_policy_find("PB0"), "output", 1, 1));
    assert(!pin_policy_mode_request_valid(pin_policy_find("PA5"), "output", 1, 1));
    assert(!pin_policy_mode_request_valid(pin_policy_find("PA2"), "output", 1, 1));
    puts("pin policy tests: PASS"); return 0;
}
#endif
