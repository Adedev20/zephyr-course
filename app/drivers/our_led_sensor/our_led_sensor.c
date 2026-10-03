#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(our_led_sensor, CONFIG_SENSOR_LOG_LEVEL);

/* Custom container holding the GPIO properties from devicetree */
struct led_sensor_config {
    struct gpio_dt_spec led_gpio;
};

/* Turn the LED ON when app calls sensor_sample_fetch */
static int led_sensor_sample_fetch(const struct device *dev, enum sensor_channel chan)
{
    const struct led_sensor_config *cfg = dev->config;
    return gpio_pin_set_dt(&cfg->led_gpio, 1);
}

/* Turn the LED OFF when app calls sensor_channel_get */
static int led_sensor_channel_get(const struct device *dev, enum sensor_channel chan, struct sensor_value *val)
{
    const struct led_sensor_config *cfg = dev->config;
    int ret = gpio_pin_set_dt(&cfg->led_gpio, 0);
    
    val->val1 = 0; // Clear output registers
    val->val2 = 0;
    return ret;
}

static const struct sensor_driver_api led_sensor_api = {
    .sample_fetch = led_sensor_sample_fetch,
    .channel_get = led_sensor_channel_get,
};

static int led_sensor_init(const struct device *dev)
{
    const struct led_sensor_config *cfg = dev->config;
    if (!gpio_is_ready_dt(&cfg->led_gpio)) {
        return -ENODEV;
    }
    return gpio_pin_configure_dt(&cfg->led_gpio, GPIO_OUTPUT_INACTIVE);
}

/* Directly fetches the compiled hardware spec properties from your /leds/led_1 path */
static const struct led_sensor_config config_instance = {
    .led_gpio = GPIO_DT_SPEC_GET(DT_PATH(leds, led_1), gpios),
};

/* Explicit initialization macro binding */
DEVICE_DEFINE(led_sensor_device, "LED_SENSOR",
              led_sensor_init, NULL,
              NULL, &config_instance,
              POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY,
              &led_sensor_api);

