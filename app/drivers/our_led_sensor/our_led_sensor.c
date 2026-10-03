#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(our_led_sensor, CONFIG_SENSOR_LOG_LEVEL);

/* 1. Define the Custom Extension API Structure */
struct led_sensor_api_extension {
    int (*set_blink_period)(const struct device *dev, uint32_t period_ms);
};

/* Immutable driver settings */
struct led_sensor_config {
    struct gpio_dt_spec led_gpio;
};

/* 2. Runtime Mutable Data Struct holding our chooseable parameter */
struct led_sensor_data {
    uint32_t blink_period_ms; // Custom runtime parameter
};

static int led_sensor_sample_fetch(const struct device *dev, enum sensor_channel chan)
{
    const struct led_sensor_config *cfg = dev->config;
    return gpio_pin_set_dt(&cfg->led_gpio, 1);
}

static int led_sensor_channel_get(const struct device *dev, enum sensor_channel chan, struct sensor_value *val)
{
    const struct led_sensor_config *cfg = dev->config;
    int ret = gpio_pin_set_dt(&cfg->led_gpio, 0);
    val->val1 = 0;
    val->val2 = 0;
    return ret;
}

/* 3. Implement the Custom Extension Function */
static int led_sensor_impl_set_blink_period(const struct device *dev, uint32_t period_ms)
{
    struct led_sensor_data *data = dev->data;
    
    if (period_ms == 0) {
        return -EINVAL; // Guard check against invalid parameters
    }

    data->blink_period_ms = period_ms;
    LOG_INF("Driver runtime parameter 'blink_period_ms' updated to: %u ms", data->blink_period_ms);
    return 0;
}

/* 4. Map both standard Sensor functions AND our custom extension */
static const struct sensor_driver_api led_sensor_api = {
    .sample_fetch = led_sensor_sample_fetch,
    .channel_get = led_sensor_channel_get,
};

/* Instantiate our custom extension table */
static const struct led_sensor_api_extension led_sensor_ext_api = {
    .set_blink_period = led_sensor_impl_set_blink_period,
};

static int led_sensor_init(const struct device *dev)
{
    const struct led_sensor_config *cfg = dev->config;
    struct led_sensor_data *data = dev->data;

    // Set a default initial value for our parameter inside runtime data
    data->blink_period_ms = 1000; 

    if (!gpio_is_ready_dt(&cfg->led_gpio)) {
        return -ENODEV;
    }
    return gpio_pin_configure_dt(&cfg->led_gpio, GPIO_OUTPUT_INACTIVE);
}

static const struct led_sensor_config config_instance = {
    .led_gpio = GPIO_DT_SPEC_GET(DT_PATH(leds, led_1), gpios),
};

/* Allocate dynamic mutable memory instance block for data */
static struct led_sensor_data data_instance;

/* We pass &led_sensor_ext_api into the custom context pointer slot of DEVICE_DEFINE */
DEVICE_DEFINE(led_sensor_device, "LED_SENSOR",
              led_sensor_init, NULL,
              &data_instance, &config_instance,
              POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY,
              &led_sensor_api);

/* Hack/Expose the extension structural pointer symbol globally for main.c usage */
const void *led_sensor_extension_ptr = &led_sensor_ext_api;
