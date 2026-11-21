// SPDX-License-Identifier: MIT
#include <stdlib.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/sys/atomic.h>
#include <zmk/activity.h>
#include <zmk/event_manager.h>
#include <zmk/events/activity_state_changed.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/split/transport/central.h>
#include <lvgl_zephyr.h>
#include "led4_palette.h"
#include "modules_profile.h"

LOG_MODULE_REGISTER(polaris_display);
#define DEV(label) DEVICE_DT_GET(DT_NODELABEL(label))
static const struct gpio_dt_spec gate = GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), led4_enable_gpios);
static atomic_t active = ATOMIC_INIT(POLARIS_DISPLAY_INACTIVE);
static atomic_t left_charge = ATOMIC_INIT(-1), right_charge = ATOMIC_INIT(-1);
static bool powered, last_connected;
static int64_t started;

enum polaris_display polaris_active_display(void) { return atomic_get(&active); }

static bool connected(void) {
    STRUCT_SECTION_FOREACH(zmk_split_transport_central, transport) {
        if (!transport->api->get_status) continue;
        struct zmk_split_transport_status s = transport->api->get_status();
        if (s.enabled && s.available && s.connections == ZMK_SPLIT_TRANSPORT_CONNECTIONS_STATUS_ALL_CONNECTED)
            return true;
    }
    return false;
}

/* HSL palette to RGB with a conservative peak drive for the status LEDs. */
static struct led_rgb rgb(struct led4_hsl c) {
    int chroma = (100 - abs(2 * c.l - 100)) * c.s / 100;
    int x = chroma * (60 - abs((int)(c.h % 120) - 60)) / 60;
    int sector = c.h / 60, base = c.l - chroma / 2;
    int r = sector == 0 || sector == 5 ? chroma : sector == 1 || sector == 4 ? x : 0;
    int g = sector == 1 || sector == 2 ? chroma : sector == 0 || sector == 3 ? x : 0;
    int b = sector == 3 || sector == 4 ? chroma : sector == 2 || sector == 5 ? x : 0;
    return (struct led_rgb){.r = (base + r) / 3, .g = (base + g) / 3, .b = (base + b) / 3};
}

static void frame(struct k_work *work);
static K_WORK_DELAYABLE_DEFINE(frames, frame);
static void frame(struct k_work *work) {
    if (zmk_activity_get_state() != ZMK_ACTIVITY_ACTIVE) {
        gpio_pin_set_dt(&gate, 0);
        powered = false;
        return;
    }
    int64_t now = k_uptime_get();
    bool link = connected();
    if (!powered) {
        gpio_pin_set_dt(&gate, 1);
        started = now;
        powered = true;
        last_connected = link;
        k_work_reschedule(&frames, K_MSEC(5));
        return;
    }
    if (link != last_connected && now - started >= 4000) started = now - 2000;
    last_connected = link;
    struct led_rgb pixels[4];
    for (size_t i = 0; i < ARRAY_SIZE(pixels); ++i) {
        pixels[i] = rgb(led4_pixel(now - started, i, link, atomic_get(&left_charge),
                                    link ? atomic_get(&right_charge) : -1));
    }
    int rc = led_strip_update_rgb(DEV(led_strip), pixels, ARRAY_SIZE(pixels));
    if (rc) {
        gpio_pin_set_dt(&gate, 0);
        atomic_set(&active, POLARIS_DISPLAY_INACTIVE);
        LOG_ERR("LED4 update failed (%d)", rc);
        return;
    }
    k_work_reschedule(&frames, K_MSEC(60));
}

static int event_handler(const zmk_event_t *eh) {
    const struct zmk_battery_state_changed *local = as_zmk_battery_state_changed(eh);
    if (local) atomic_set(&left_charge, local->state_of_charge);
    const struct zmk_peripheral_battery_state_changed *remote = as_zmk_peripheral_battery_state_changed(eh);
    if (remote && !remote->source) atomic_set(&right_charge, remote->state_of_charge);
    if (as_zmk_activity_state_changed(eh) && polaris_active_display() == POLARIS_LED4)
        k_work_reschedule(&frames, K_NO_WAIT);
    return ZMK_EV_EVENT_BUBBLE;
}
ZMK_LISTENER(polaris_display_events, event_handler);
ZMK_SUBSCRIPTION(polaris_display_events, zmk_battery_state_changed);
ZMK_SUBSCRIPTION(polaris_display_events, zmk_peripheral_battery_state_changed);
ZMK_SUBSCRIPTION(polaris_display_events, zmk_activity_state_changed);

static void disconnected(struct bt_conn *conn, uint8_t reason) {
    struct bt_conn_info info;
    if (!bt_conn_get_info(conn, &info) && info.role == BT_CONN_ROLE_CENTRAL)
        atomic_set(&right_charge, -1);
}
BT_CONN_CB_DEFINE(polaris_display_conn) = {.disconnected = disconnected};

/* Called after settings_load(), before the first OLED work item. */
int polaris_prepare_display(void) {
    struct zeroshift_state state;
    int rc = zeroshift_profile(NULL, &state);
    if (rc || state.saved[0] == POLARIS_CIRQUE_MODE) return 0;
    if (state.saved[0] & POLARIS_LED4_FLAG) {
        rc = gpio_pin_configure_dt(&gate, GPIO_OUTPUT_INACTIVE);
        if (!rc) rc = device_init(DEV(spi3));
        if (!rc) rc = device_init(DEV(led_strip));
        if (!rc) {
            atomic_set(&active, POLARIS_LED4);
            k_work_reschedule(&frames, K_NO_WAIT);
        }
    } else {
        rc = device_init(DEV(i2c1));
        if (!rc) rc = device_init(DEV(oled));
        if (!rc) rc = lvgl_init();
        if (!rc) {
            atomic_set(&active, POLARIS_OLED);
            return 1;
        }
    }
    if (rc) LOG_ERR("Display initialization failed (%d)", rc);
    return 0;
}
