// SPDX-License-Identifier: MIT
#include <zephyr/drivers/sensor.h>
#include <zephyr/input/input.h>
#include <drivers/input_processor.h>
#include <zmk/events/sensor_event.h>
#include "modules_profile.h"
#include "module_debug.h"
#include <zmk/events/position_state_changed.h>
#include <zmk/matrix_transform.h>

#define DEV(label) DEVICE_DT_GET(DT_NODELABEL(label))
enum module_mode { OFF, TB, JOY, ENC, TPD, LPPS, IQS, KEY, PAT };
BUILD_ASSERT(ZEROSHIFT_SLOT_COUNT == 1);

ZMK_MATRIX_TRANSFORM_EXTERN(DT_NODELABEL(default_transform));

static void key_event(struct input_event *event, void *unused) {
    if (event->type != INPUT_EV_KEY) return;
    int32_t position = zmk_matrix_transform_row_column_to_position(
        ZMK_MATRIX_TRANSFORM_T_FOR_NODE(DT_NODELABEL(default_transform)), 6, 0);
    if (position >= 0) {
        raise_zmk_position_state_changed((struct zmk_position_state_changed){
            .source = ZMK_POSITION_STATE_CHANGE_SOURCE_LOCAL, .position = position,
            .state = event->value != 0, .timestamp = k_uptime_get()});
    }
}
INPUT_CALLBACK_DEFINE(DEV(module_key), key_event, NULL);

static void rotation(const struct device *dev, const struct sensor_trigger *trigger) {
    struct sensor_value value;
    if (sensor_sample_fetch(dev) || sensor_channel_get(dev, trigger->chan, &value)) return;
    if (!value.val1 && !value.val2) return;
    polaris_debug_encoder();
    raise_zmk_sensor_event((struct zmk_sensor_event){
        .sensor_index = IS_ENABLED(CONFIG_SHIELD_Polaris_R_Base), .channel_data_size = 1,
        .channel_data = {{.value = value, .channel = trigger->chan}}, .timestamp = k_uptime_get()});
}

static int start_encoder(const struct device *dev) {
    static const struct sensor_trigger trigger = {.type = SENSOR_TRIG_DATA_READY, .chan = SENSOR_CHAN_ROTATION};
    int err = polaris_debug_init(dev);
    return err ? err : sensor_trigger_set(dev, &trigger, rotation);
}


/* Preserve fractional steps between optical samples and direction changes. */
#define DIAL_COUNTS_PER_TURN 1892
#define DIAL_STEPS_PER_TURN 24
static int32_t dial_steps(int32_t delta) {
    static int32_t remainder;
    int64_t scaled = (int64_t)delta * DIAL_STEPS_PER_TURN + remainder;
    int32_t steps = scaled / DIAL_COUNTS_PER_TURN;
    remainder = scaled % DIAL_COUNTS_PER_TURN;
    if (steps) polaris_debug_dial(steps);
    return steps;
}


static int dial_scroll_event(const struct device *dev, struct input_event *event, uint32_t param1,
                             uint32_t param2, struct zmk_input_processor_state *state) {
    if (event->type != INPUT_EV_REL || event->code != INPUT_REL_X) return ZMK_INPUT_PROC_STOP;
    int32_t steps = dial_steps(event->value);
    if (!steps) return ZMK_INPUT_PROC_STOP;
    event->code = INPUT_REL_WHEEL;
    event->value = -steps;
    return ZMK_INPUT_PROC_CONTINUE;
}

static const struct zmk_input_processor_driver_api dial_scroll_api = {.handle_event = dial_scroll_event};
DEVICE_DT_DEFINE(DT_NODELABEL(dial_scroll), NULL, NULL, NULL, NULL, POST_KERNEL,
                 CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &dial_scroll_api);


static int start_on_bus(const struct device *bus, const struct device *device) {
    int err = polaris_debug_init(bus);
    return err ? err : polaris_debug_init(device);
}

#ifdef CONFIG_ZMK_SPLIT_ROLE_CENTRAL
#include <zephyr/drivers/pinctrl.h>
PINCTRL_DT_DEV_CONFIG_DECLARE(DT_NODELABEL(i2c0));
PINCTRL_DT_STATE_PINS_DEFINE(DT_PATH(zephyr_user), iqs_default);
#ifndef PINCTRL_SKIP_SLEEP
PINCTRL_DT_STATE_PINS_DEFINE(DT_PATH(zephyr_user), iqs_sleep);
#endif
static const struct pinctrl_state left_iqs_states[] = {
    PINCTRL_DT_STATE_INIT(iqs_default, PINCTRL_STATE_DEFAULT),
#ifndef PINCTRL_SKIP_SLEEP
    PINCTRL_DT_STATE_INIT(iqs_sleep, PINCTRL_STATE_SLEEP),
#endif
};

static int start_left_iqs(void) {
    int rc = pinctrl_update_states(PINCTRL_DT_DEV_CONFIG_GET(DT_NODELABEL(i2c0)),
                                   left_iqs_states, ARRAY_SIZE(left_iqs_states));
    return rc ? rc : start_on_bus(DEV(i2c0), DEV(left_iqs));
}
#endif

uint32_t polaris_mode_mask(bool right) {
    uint32_t common = BIT(OFF) | BIT(TB) | BIT(ENC) | BIT(TPD) | BIT(LPPS) | BIT(KEY) | BIT(PAT) | BIT(IQS);
    return common | (right ? 0 : BIT(JOY));
}

bool zeroshift_mode_supported(size_t slot, uint8_t mode) {
#ifdef POLARIS_SELECTABLE_DISPLAY
    if (mode & POLARIS_LED4_FLAG) {
        mode &= ~POLARIS_LED4_FLAG;
        if (mode == TPD) return false;
    }
#endif
    return slot == 0 && mode <= PAT &&
           (polaris_mode_mask(IS_ENABLED(CONFIG_SHIELD_Polaris_R_Base)) & BIT(mode));
}

static int start_mode(size_t slot, uint8_t mode) {
    if (!zeroshift_mode_supported(slot, mode)) return -EINVAL;
#ifdef POLARIS_SELECTABLE_DISPLAY
    mode &= ~POLARIS_LED4_FLAG;
    if (mode == OFF) return 0;
#endif
    switch (mode) {
    case KEY: return polaris_debug_init(DEV(module_key));
    case LPPS: {
        int err = start_on_bus(DEV(lpps_spi), DEV(adc_ads1220));
        return err ? err : polaris_debug_init(DEV(lpps_axis));
    }
    case ENC: return start_encoder(DEV(enc));
#ifdef CONFIG_ZMK_SPLIT_ROLE_CENTRAL
    case IQS: return start_left_iqs();
    case JOY: {
        int err = start_encoder(DEV(joy_enc));
        return err ? err : polaris_debug_init(DEV(anin0));
    }
    case TB: return start_on_bus(DEV(spi2), DEV(trackball2));
    case TPD: return start_on_bus(DEV(i2c0), DEV(glidepoint_l));
#else
    case TB: return start_on_bus(DEV(pmw3610_spi), DEV(trackball));
    case TPD: return start_on_bus(DEV(i2c0), DEV(glidepoint_r));
    case IQS: return start_on_bus(DEV(i2c1), DEV(iqs9151));
#endif
    case PAT: {
        int ret = start_on_bus(DEV(dial_bus), DEV(module_dial));
        return ret ? ret : polaris_debug_init(DEV(module_key));
    }
    default: return -EINVAL;
    }
}

int zeroshift_start_mode(size_t slot, uint8_t mode) {
    int rc = start_mode(slot, mode);
    polaris_debug_result(rc);
    return rc;
}
