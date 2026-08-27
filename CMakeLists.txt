# Public ZMK event observation only; no radio hooks or dependency patches.
if(CONFIG_ZMK_SPLIT_ESB AND CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
  zephyr_library()
  zephyr_library_sources(src/esb_validation.c)
endif()
