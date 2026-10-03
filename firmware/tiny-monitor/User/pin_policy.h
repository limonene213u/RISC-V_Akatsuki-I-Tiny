#ifndef TINY_PIN_POLICY_H
#define TINY_PIN_POLICY_H

#include <stddef.h>
#include <stdint.h>

typedef enum { PIN_USER, PIN_INPUT_ONLY, PIN_BOARD_FUNCTION, PIN_LOCKED } pin_policy_t;
typedef struct {
    const char *name;
    char port;
    uint8_t number;
    const char *function;
    pin_policy_t policy;
    const char *reason;
    uint8_t supports_pulldown;
} board_pin_t;

const board_pin_t *pin_policy_find(const char *name);
const board_pin_t *pin_policy_table(size_t *count);
const char *pin_policy_name(pin_policy_t policy);
int pin_policy_is_user(const board_pin_t *pin);
int pin_policy_mode_request_valid(const board_pin_t *pin, const char *mode,
                                  int has_initial_value, uint32_t initial_value);

#endif
