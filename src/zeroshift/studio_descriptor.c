// SPDX-License-Identifier: MIT
#include <errno.h>
#include <pb_encode.h>
#include <zephyr/sys/byteorder.h>
#include <zmk/studio/custom.h>
#include "modules_profile.h"

/* The public slots are independent. NVS and the split relay use one packed byte. */
static const char descriptor[] =
#include "descriptor.inc"
;
static uint8_t reply[8];
static const uint8_t *response;
static size_t response_size;
static struct zmk_rpc_custom_subsystem_meta meta = {.security = ZMK_STUDIO_RPC_HANDLER_UNSECURED};
ZMK_RPC_CUSTOM_SUBSYSTEM(zeroshift__v1, &meta, handle);

static bool encode(pb_ostream_t *stream, const pb_field_t *field, void *const *arg) {
    return pb_encode_tag_for_field(stream, field) && pb_encode_string(stream, response, response_size);
}

static int profile(const uint8_t *data, size_t size) {
    if (size < 2 || data[1] > 1) return -EINVAL;
    bool right = data[1];
    bool display = false;
#ifdef POLARIS_SELECTABLE_DISPLAY
    display = !right;
#endif
    size_t slots = display ? 2 : 1;
    bool write = data[0] == 2;
    if (write ? size != 2 + slots : data[0] != 1 || size != 2) return -EINVAL;

    uint8_t packed = 0;
    if (write) {
        packed = data[2];
        if (packed >= 32 || !(polaris_mode_mask(right) & BIT(packed))) return -EINVAL;
        if (display) {
            if (data[3] > 1 || (data[3] && packed == POLARIS_CIRQUE_MODE)) return -EINVAL;
            if (data[3]) packed |= POLARIS_LED4_FLAG;
        }
    }
    struct zeroshift_state state;
    const uint8_t *requested = write ? &packed : NULL;
    int rc = right ? polaris_remote_profile(requested, &state) : zeroshift_profile(requested, &state);
    if (rc) return rc;
    reply[4] = state.saved[0] & ~POLARIS_LED4_FLAG;
    reply[4 + slots] = state.active[0] & ~POLARIS_LED4_FLAG;
#ifdef POLARIS_SELECTABLE_DISPLAY
    if (display) {
        reply[5] = !!(state.saved[0] & POLARIS_LED4_FLAG);
        reply[7] = polaris_active_display() == POLARIS_LED4;
    }
#endif
    response_size += 2 * slots;
    return 0;
}

static bool handle(const zmk_custom_CallRequest *req, pb_callback_t *out) {
    const uint8_t *data = req->payload.bytes;
    size_t size = req->payload.size;
    out->funcs.encode = encode;
    if (size == 1 && data[0] == 0) {
        response = (const uint8_t *)descriptor;
        response_size = sizeof(descriptor) - 1;
    } else {
        response = reply;
        response_size = 4;
        sys_put_le32((uint32_t)profile(data, size), reply);
    }
    return true;
}
