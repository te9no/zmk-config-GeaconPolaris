// SPDX-License-Identifier: MIT
#include <pb_decode.h>
#include <zmk/studio/custom.h>
#include "modules.pb.h"
#include "modules_profile.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);
static struct zmk_rpc_custom_subsystem_meta meta = {.security = ZMK_STUDIO_RPC_HANDLER_UNSECURED};
ZMK_RPC_CUSTOM_SUBSYSTEM(polaris__modules, &meta, handle);
ZMK_RPC_CUSTOM_SUBSYSTEM_RESPONSE_BUFFER(polaris__modules, polaris_Response);

static bool handle(const zmk_custom_CallRequest *raw, pb_callback_t *encode) {
    polaris_Response *resp = ZMK_RPC_CUSTOM_SUBSYSTEM_RESPONSE_BUFFER_ALLOCATE(polaris__modules, encode);
    *resp = (polaris_Response)polaris_Response_init_zero;
    polaris_Request req = polaris_Request_init_zero;
    pb_istream_t stream = pb_istream_from_buffer(raw->payload.bytes, raw->payload.size);
    uint8_t modes[1];
    bool decoded = pb_decode(&stream, polaris_Request_fields, &req);
    bool write = req.which_operation == polaris_Request_set_tag;
    if (!decoded || (!write && req.which_operation != polaris_Request_get_tag) ||
        (write && req.operation.set >= POLARIS_LED4_FLAG)) {
        resp->error = -EINVAL;
        return true;
    }
    if (write) {
        modes[0] = req.operation.set;
    }
    struct zeroshift_state state;
#ifdef POLARIS_SELECTABLE_DISPLAY
    if (!req.right && write) {
        /* Older clients omit display. Preserve the saved choice on their writes. */
        resp->error = zeroshift_profile(NULL, &state);
        if (resp->error) return true;
        if (req.display > POLARIS_LED4) {
            resp->error = -EINVAL;
            return true;
        }
        if (req.display == POLARIS_LED4 ||
            (!req.display && (state.saved[0] & POLARIS_LED4_FLAG))) modes[0] |= POLARIS_LED4_FLAG;
    } else
#endif
    if (req.display) {
        resp->error = -EINVAL;
        return true;
    }
    resp->error = req.right ? polaris_remote_profile(write ? modes : NULL, &state)
                            : zeroshift_profile(write ? modes : NULL, &state);
    if (!resp->error) {
        resp->supported = polaris_mode_mask(req.right);
        resp->saved = state.saved[0] & ~POLARIS_LED4_FLAG;
        resp->active = state.active[0] & ~POLARIS_LED4_FLAG;
#ifdef POLARIS_SELECTABLE_DISPLAY
        if (!req.right) {
            resp->saved_display = (state.saved[0] & POLARIS_LED4_FLAG) ? POLARIS_LED4 : POLARIS_OLED;
            resp->active_display = polaris_active_display();
        }
#endif
    }
    return true;
}
