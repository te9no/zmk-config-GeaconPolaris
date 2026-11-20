// SPDX-License-Identifier: MIT
#pragma once
#include <zephyr/device.h>

#ifdef POLARIS_MODULE_DEBUG
int polaris_debug_init(const struct device *dev);
void polaris_debug_encoder(void);
void polaris_debug_dial(int32_t steps);
void polaris_debug_result(int rc);
#else
#define polaris_debug_init device_init
#define polaris_debug_dial(steps) ((void)0)
#define polaris_debug_encoder() ((void)0)
#define polaris_debug_result(rc) ((void)0)
#endif
