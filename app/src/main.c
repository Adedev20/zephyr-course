#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main_app);

int main(void)
{
    /* Fetch the device handle using the exact string name registered in your driver */
    const struct device *dev = device_get_binding("LED_SENSOR");

    if (dev == NULL) {
        LOG_ERR("Custom LED Sensor Device link was not found.");
        return -ENODEV;
    }

    if (!device_is_ready(dev)) {
        LOG_ERR("Custom LED Sensor Device is present but not ready.");
        return -ENODEV;
    }

    LOG_INF("LED Sensor tied in perfectly. Entering runtime loop...");

    struct sensor_value val;
    while (1) {
        LOG_INF("Fetching Sample -> Turning LED ON");
        sensor_sample_fetch(dev);
        k_msleep(1000);

        LOG_INF("Channel Get -> Turning LED OFF");
        sensor_channel_get(dev, SENSOR_CHAN_ALL, &val);
        k_msleep(1000);
    }
    return 0;
}
