// SPDX-License-Identifier: MIT
#pragma once
#include <zeroshift/profile.h>

int polaris_remote_profile(const uint8_t *requested, struct zeroshift_state *state);

uint32_t polaris_mode_mask(bool right);
