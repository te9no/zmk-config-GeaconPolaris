# Public ZMK event observation only; no radio hooks or dependency patches.
if(CONFIG_ZMK_SPLIT_ESB)
  zephyr_library()
  zephyr_library_sources(src/esb_validation.c)
endif()
