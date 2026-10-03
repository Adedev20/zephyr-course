#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main_app);

/* Map the external tracking structures matching our driver internals */
struct led_sensor_api_extension {
    int (*set_blink_period)(const struct device *dev, uint32_t period_ms);
};
extern const void *led_sensor_extension_ptr;

int main(void)
{
    const struct device *dev = device_get_binding("LED_SENSOR");

    if (dev == NULL || !device_is_ready(dev)) {
        LOG_ERR("Custom LED Sensor Device link was not found or is not ready.");
        return -ENODEV;
    }

    /* Grab our extension API block pointer */
    const struct led_sensor_api_extension *ext_api = led_sensor_extension_ptr;
    struct sensor_value val;
    uint32_t current_delay = 1000;

    LOG_INF("Modifying dynamic driver parameter via custom extension API...");
    
    // Call our extension API function directly!
    ext_api->set_blink_period(dev, 500); 
    current_delay = 500; // Update local delay tracking to 0.5s

    while (1) {
        LOG_INF("Fetching Sample -> Turning LED ON");
        sensor_sample_fetch(dev);
        k_msleep(current_delay);

        LOG_INF("Channel Get -> Turning LED OFF");
        sensor_channel_get(dev, SENSOR_CHAN_ALL, &val);
        k_msleep(current_delay);
    }
    return 0;
}
