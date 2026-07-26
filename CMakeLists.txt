if(CONFIG_I2C)
  zephyr_library()
  zephyr_library_sources(src/deferred_oled_clear.c)
endif()
