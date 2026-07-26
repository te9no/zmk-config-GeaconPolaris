#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/pm/device_runtime.h>

LOG_MODULE_REGISTER(polaris_oled, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_CHOSEN(zephyr_display) &&                                                        \
    DT_NODE_HAS_PROP(DT_CHOSEN(zephyr_display), zephyr_deferred_init)

static void deferred_oled_clear(void *unused1, void *unused2, void *unused3)
{
    const struct device *display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    const struct device *bus = DEVICE_DT_GET(DT_BUS(DT_CHOSEN(zephyr_display)));
    static uint8_t clear_buffer[128 * 32 / 8];
    const struct display_buffer_descriptor clear_desc = {
        .buf_size = sizeof(clear_buffer),
        .width = 128,
        .height = 32,
        .pitch = 128,
    };
    int ret;

    ARG_UNUSED(unused1);
    ARG_UNUSED(unused2);
    ARG_UNUSED(unused3);

    k_sleep(K_SECONDS(10));

    ret = pm_device_runtime_get(bus);
    if (ret < 0) {
        LOG_ERR("Failed to resume OLED I2C bus (%d)", ret);
        return;
    }

    ret = i2c_recover_bus(bus);
    if (ret < 0) {
        LOG_WRN("OLED I2C bus recovery failed (%d)", ret);
    }

    ret = device_init(display);
    if (ret < 0) {
        LOG_ERR("Deferred OLED initialization failed (%d)", ret);
        return;
    }

    ret = display_write(display, 0, 0, &clear_desc, clear_buffer);
    if (ret < 0) {
        LOG_ERR("Failed to clear OLED framebuffer (%d)", ret);
        return;
    }

    display_blanking_off(display);
    LOG_INF("OLED framebuffer cleared");
}

K_THREAD_DEFINE(polaris_oled_thread, 1024, deferred_oled_clear, NULL, NULL, NULL, 10, 0, 0);

#endif
