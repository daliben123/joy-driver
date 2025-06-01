#pragma once

#include <zephyr/device.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct touch_callback {
    void (*func)(int8_t x, int8_t y);
    struct touch_callback *next;
};

void touchpad_add_touch_callback(struct touch_callback *callback);
void touchpad_gpio_irq(uint gpio, uint32_t events);
void touchpad_init(void);

#ifdef __cplusplus
}
#endif
