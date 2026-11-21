// SPDX-License-Identifier: MIT
#pragma once
#include <zeroshift/profile.h>

int polaris_remote_profile(const uint8_t *requested, struct zeroshift_state *state);

uint32_t polaris_mode_mask(bool right);

/* Display selection occupies the high bit of the local profile. */
#define POLARIS_LED4_FLAG 0x80
#define POLARIS_CIRQUE_MODE 4
enum polaris_display { POLARIS_OLED, POLARIS_LED4, POLARIS_DISPLAY_INACTIVE };
#ifdef POLARIS_SELECTABLE_DISPLAY
int polaris_prepare_display(void);
enum polaris_display polaris_active_display(void);
#endif
