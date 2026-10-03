# Add the driver C source file straight into the main application execution context
if(CONFIG_OUR_LED_SENSOR)
    target_sources(app PRIVATE our_led_sensor.c)
endif()
