// SPDX-License-Identifier: MIT
#include <zephyr/init.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "modules_profile.h"
#include "module_debug.h"

LOG_MODULE_REGISTER(polaris_module, LOG_LEVEL_DBG);

static const char *const modes[] = {
    "off", "pmw3610", "stick", "encoder", "cirque", "lpps", "iqs9151", "1u", "pat9125"
};
static const char *failed_device = "-";
static int start_rc;
static atomic_t moves, wheels, buttons, rotations, dial_cw, dial_ccw;

int polaris_debug_init(const struct device *dev) {
    int rc = device_init(dev);
    LOG_INF("init %s rc=%d", dev->name, rc);
    if (rc) failed_device = dev->name;
    return rc;
}

void polaris_debug_dial(int32_t steps) {
    if (steps > 0) atomic_add(&dial_cw, steps);
    else atomic_add(&dial_ccw, -steps);
}

void polaris_debug_result(int rc) { start_rc = rc; }
void polaris_debug_encoder(void) { atomic_inc(&rotations); }

static void count_event(struct input_event *evt, void *unused) {
    if (evt->type == INPUT_EV_KEY) atomic_inc(&buttons);
    if (evt->type != INPUT_EV_REL) return;
    switch (evt->code) {
    case INPUT_REL_X:
    case INPUT_REL_Y: atomic_inc(&moves); break;
    case INPUT_REL_WHEEL:
    case INPUT_REL_HWHEEL: atomic_inc(&wheels); break;
    }
}

#define COUNT_FROM(label) INPUT_CALLBACK_DEFINE_NAMED(DEVICE_DT_GET(DT_NODELABEL(label)), count_event, NULL, diag_##label)
COUNT_FROM(module_key);
COUNT_FROM(module_dial);
COUNT_FROM(lpps_axis);
#ifdef CONFIG_ZMK_SPLIT_ROLE_CENTRAL
static atomic_t iqs_xy, iqs_wheel, iqs_button;

static void count_remote_iqs(struct input_event *evt, void *unused) {
    if (evt->type == INPUT_EV_KEY) {
        atomic_inc(&iqs_button);
    } else if (evt->type == INPUT_EV_REL && evt->value != 0) {
        if (evt->code == INPUT_REL_X || evt->code == INPUT_REL_Y) {
            atomic_inc(&iqs_xy);
        } else if (evt->code == INPUT_REL_WHEEL || evt->code == INPUT_REL_HWHEEL) {
            atomic_inc(&iqs_wheel);
        }
    }
}

INPUT_CALLBACK_DEFINE_NAMED(DEVICE_DT_GET(DT_NODELABEL(iqs_split)), count_remote_iqs, NULL, diag_remote_iqs);
COUNT_FROM(left_iqs);
COUNT_FROM(trackball2);
COUNT_FROM(glidepoint_l);
COUNT_FROM(anin0);
#else
COUNT_FROM(trackball);
COUNT_FROM(glidepoint_r);
COUNT_FROM(iqs9151);
#endif

static const char *mode_name(uint8_t mode) {
    mode &= ~POLARIS_LED4_FLAG;
    return mode < ARRAY_SIZE(modes) ? modes[mode] : "unknown";
}

static void report(struct k_work *work);
static K_WORK_DELAYABLE_DEFINE(status_work, report);

static void report(struct k_work *work) {
    struct zeroshift_state state;
    int rc = zeroshift_profile(NULL, &state);
    if (rc) {
        LOG_INF("profile rc=%d", rc);
    } else {
        LOG_INF("%s saved=%s active=%s start_rc=%d failed_device=%s",
                IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL) ? "left" : "right",
                mode_name(state.saved[0]), mode_name(state.active[0]), start_rc, failed_device);
        if (atomic_get(&dial_cw) || atomic_get(&dial_ccw)) {
            LOG_INF("dial steps: positive=%ld negative=%ld", (long)atomic_get(&dial_cw),
                    (long)atomic_get(&dial_ccw));
        }
#ifdef CONFIG_ZMK_SPLIT_ROLE_CENTRAL
        LOG_INF("right IQS received: nonzero_xy=%ld nonzero_wheel=%ld button=%ld",
                (long)atomic_get(&iqs_xy), (long)atomic_get(&iqs_wheel), (long)atomic_get(&iqs_button));
#endif
        LOG_INF("local input totals: xy=%ld wheel=%ld button=%ld encoder=%ld",
                (long)atomic_get(&moves), (long)atomic_get(&wheels),
                (long)atomic_get(&buttons), (long)atomic_get(&rotations));
    }
    k_work_reschedule(&status_work, K_SECONDS(5));
}

static int start_report(void) {
    k_work_schedule(&status_work, K_SECONDS(2));
    return 0;
}
SYS_INIT(start_report, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
