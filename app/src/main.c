#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/shell/shell.h>
#include <stdlib.h>

/* Map the external tracking structures matching our driver internals */
struct led_sensor_api_extension {
    int (*set_blink_period)(const struct device *dev, uint32_t period_ms);
};
extern const void *led_sensor_extension_ptr;

/* Helper function to get our specific driver binding safely */
static const struct device *get_sensor_device(const struct shell *sh)
{
    const struct device *dev = device_get_binding("LED_SENSOR");
    if (dev == NULL) {
        shell_error(sh, "Error: Device 'LED_SENSOR' binding not found.");
        return NULL;
    }
    return dev;
}

/* Subcommand: fetch -> calls sensor_sample_fetch() */
static int cmd_sensor_fetch(const struct shell *sh, size_t argc, char **argv)
{
    const struct device *dev = get_sensor_device(sh);
    if (!dev) return -ENODEV;

    shell_print(sh, "Executing sensor_sample_fetch()...");
    int ret = sensor_sample_fetch(dev);
    if (ret < 0) {
        shell_error(sh, "sensor_sample_fetch failed with error code: %d", ret);
        return ret;
    }
    shell_print(sh, "Fetch successful! LED should be ON.");
    return 0;
}

/* Subcommand: read -> calls sensor_channel_get() */
static int cmd_sensor_read(const struct shell *sh, size_t argc, char **argv)
{
    const struct device *dev = get_sensor_device(sh);
    if (!dev) return -ENODEV;

    struct sensor_value val;
    shell_print(sh, "Executing sensor_channel_get()...");
    
    int ret = sensor_channel_get(dev, SENSOR_CHAN_ALL, &val);
    if (ret < 0) {
        shell_error(sh, "sensor_channel_get failed with error code: %d", ret);
        return ret;
    }
    shell_print(sh, "Read successful! LED should be OFF.");
    return 0;
}

/* Subcommand: info -> prints name and initialization status */
static int cmd_sensor_info(const struct shell *sh, size_t argc, char **argv)
{
    const struct device *dev = get_sensor_device(sh);
    if (!dev) return -ENODEV;

    shell_print(sh, "--- Sensor Device Information ---");
    shell_print(sh, "Device Name : %s", dev->name);
    shell_print(sh, "Ready State : %s", device_is_ready(dev) ? "READY" : "NOT READY");
    return 0;
}

/* New Subcommand: set <value> -> mutates runtime parameters with validation checks */
static int cmd_sensor_set(const struct shell *sh, size_t argc, char **argv)
{
    const struct device *dev = get_sensor_device(sh);
    if (!dev) return -ENODEV;

    /* Grab our driver extension API pointer block */
    const struct led_sensor_api_extension *ext_api = led_sensor_extension_ptr;
    if (ext_api == NULL || ext_api->set_blink_period == NULL) {
        shell_error(sh, "Error: Extension API pointer is missing or unlinked.");
        return -EFAULT;
    }

    /* Convert string argument to integer safely */
    char *endptr;
    long value = strtol(argv[1], &endptr, 10);

    /* Validation Guard 1: Check if input string is a valid integer parsing block */
    if (*endptr != '\0') {
        shell_error(sh, "Error: Invalid argument '%s'. Please provide a number.", argv[1]);
        return -EINVAL;
    }

    /* Validation Guard 2: Enforce out-of-range protection (e.g., minimum 50ms, maximum 5000ms) */
    if (value < 50 || value > 5000) {
        shell_error(sh, "Error: Argument %ld is out of range. Allowed threshold: 50 to 5000 ms.", value);
        return -EINVAL;
    }

    /* Input validated successfully. Commit update directly through the custom extension API */
    int ret = ext_api->set_blink_period(dev, (uint32_t)value);
    if (ret < 0) {
        shell_error(sh, "Driver rejected parameter modification with error: %d", ret);
        return ret;
    }

    shell_print(sh, "Success: Dynamic runtime driver parameter adjusted to %ld ms.", value);
    return 0;
}

/* Define the subcommands dictionary layout using static helper macros */
SHELL_STATIC_SUBCMD_SET_CREATE(sub_sensorroot,
    SHELL_CMD(fetch, NULL, "Trigger sensor_sample_fetch().", cmd_sensor_fetch),
    SHELL_CMD(read,  NULL, "Trigger sensor_channel_get().", cmd_sensor_read),
    SHELL_CMD(info,  NULL, "Query and print device profile metadata.", cmd_sensor_info),
    
    /* Enforce exactly 2 command arguments (command name + value string parameter) using SHELL_CMD_ARG */
    SHELL_CMD_ARG(set, NULL, "Set dynamic blink period. Usage: sensorroot set <50-5000>", cmd_sensor_set, 2, 0),
    
    SHELL_SUBCMD_SET_END
);

/* Main registration anchor block */
SHELL_CMD_REGISTER(sensorroot, &sub_sensorroot, "Custom LED-based sensor driver command suite", NULL);

int main(void)
{
    return 0;
}
