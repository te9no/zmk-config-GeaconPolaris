/* SPDX-License-Identifier: MIT */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/position_state_changed.h>

LOG_MODULE_REGISTER(polaris_esb_validation, LOG_LEVEL_INF);

static atomic_t remote_position_events;
static atomic_t remote_battery_events;
static atomic_val_t last_reported_position_events;

static void report_position_count(struct k_work *work) {
    atomic_val_t count = atomic_get(&remote_position_events);
    if (count != last_reported_position_events) {
        LOG_INF("ESB source=0 position_events=%ld received", (long)count);
        last_reported_position_events = count;
    }
}

static K_WORK_DELAYABLE_DEFINE(position_report_work, report_position_count);

static int observe_remote_events(const zmk_event_t *event) {
    const struct zmk_position_state_changed *position =
        as_zmk_position_state_changed(event);
    if (position && position->source == 0) {
        /* Count only: never log key position, keycode, state, or raw payload. */
        atomic_inc(&remote_position_events);
        /* schedule (not reschedule) caps reporting to once per five seconds */
        k_work_schedule(&position_report_work, K_SECONDS(5));
        return ZMK_EV_EVENT_BUBBLE;
    }

    const struct zmk_peripheral_battery_state_changed *battery =
        as_zmk_peripheral_battery_state_changed(event);
    if (battery && battery->source == 0) {
        atomic_inc(&remote_battery_events);
        LOG_INF("ESB source=0 battery=%u%% received; battery_events=%ld position_events=%ld",
                battery->state_of_charge, (long)atomic_get(&remote_battery_events),
                (long)atomic_get(&remote_position_events));
    }
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(polaris_esb_validation, observe_remote_events);
ZMK_SUBSCRIPTION(polaris_esb_validation, zmk_position_state_changed);
ZMK_SUBSCRIPTION(polaris_esb_validation, zmk_peripheral_battery_state_changed);
