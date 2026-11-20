// SPDX-License-Identifier: MIT
#include <zephyr/drivers/sensor.h>
#include <zmk/events/sensor_event.h>
#include <zeroshift/profile.h>

#define DEV(label) DEVICE_DT_GET(DT_NODELABEL(label))
enum module_mode { OFF, TB, JOY, ENC, TPD, LPPS, IQS };
BUILD_ASSERT(ZEROSHIFT_SLOT_COUNT == 1);

#ifdef CONFIG_ZMK_SPLIT_ROLE_CENTRAL
static void rotation(const struct device *dev, const struct sensor_trigger *trigger) {
    struct sensor_value value;
    if (sensor_sample_fetch(dev) || sensor_channel_get(dev, trigger->chan, &value)) return;
    if (!value.val1 && !value.val2) return;
    raise_zmk_sensor_event((struct zmk_sensor_event){
        .sensor_index = 0, .channel_data_size = 1,
        .channel_data = {{.value = value, .channel = trigger->chan}}, .timestamp = k_uptime_get()});
}

static int start_encoder(const struct device *dev) {
    static const struct sensor_trigger trigger = {.type = SENSOR_TRIG_DATA_READY, .chan = SENSOR_CHAN_ROTATION};
    int err = device_init(dev);
    return err ? err : sensor_trigger_set(dev, &trigger, rotation);
}

#endif

static int start_on_bus(const struct device *bus, const struct device *device) {
    int err = device_init(bus);
    return err ? err : device_init(device);
}

bool zeroshift_mode_supported(size_t slot, uint8_t mode) {
    if (slot != 0) return false;
    if (IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)) return mode <= LPPS;
    return mode == OFF || mode == TB || mode == TPD || mode == IQS;
}

int zeroshift_start_mode(size_t slot, uint8_t mode) {
    if (!zeroshift_mode_supported(slot, mode)) return -EINVAL;
    switch (mode) {
#ifdef CONFIG_ZMK_SPLIT_ROLE_CENTRAL
    case ENC: return start_encoder(DEV(enc));
    case JOY: {
        int err = start_encoder(DEV(joy_enc));
        return err ? err : device_init(DEV(anin0));
    }
    case TB: return start_on_bus(DEV(spi2), DEV(trackball2));
    case TPD: return start_on_bus(DEV(i2c0), DEV(glidepoint_l));
    case LPPS: {
        int err = start_on_bus(DEV(lpps_spi), DEV(adc_ads1220));
        return err ? err : device_init(DEV(lpps_axis));
    }
#else
    case TB: return start_on_bus(DEV(pmw3610_spi), DEV(trackball));
    case TPD: return start_on_bus(DEV(i2c0), DEV(glidepoint_r));
    case IQS: return start_on_bus(DEV(i2c1), DEV(iqs9151));
#endif
    default: return -EINVAL;
    }
}
