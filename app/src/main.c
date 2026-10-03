#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/shell/shell.h>
#include <stdlib.h>

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

/* 1. Subcommand: fetch -> calls sensor_sample_fetch() */
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

/* 2. Subcommand: read -> calls sensor_channel_get() and prints output values */
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
    shell_print(sh, "Sensor Value Output -> val1: %d, val2: %d", val.val1, val.val2);
    return 0;
}

/* 3. Subcommand: info -> prints name and initialization status */
static int cmd_sensor_info(const struct shell *sh, size_t argc, char **argv)
{
    const struct device *dev = get_sensor_device(sh);
    if (!dev) return -ENODEV;

    bool is_ready = device_is_ready(dev);
    
    shell_print(sh, "--- Sensor Device Information ---");
    shell_print(sh, "Device Name : %s", dev->name);
    shell_print(sh, "Ready State : %s", is_ready ? "READY (True)" : "NOT READY (False)");
    return 0;
}

/* Define the subcommands dictionary layout using static helper macros */
SHELL_STATIC_SUBCMD_SET_CREATE(sub_sensorroot,
    SHELL_CMD(fetch, NULL, "Trigger sensor_sample_fetch() to toggle hardware context.", cmd_sensor_fetch),
    SHELL_CMD(read,  NULL, "Trigger sensor_channel_get() and display values.", cmd_sensor_read),
    SHELL_CMD(info,  NULL, "Query and print the target device profile metadata.", cmd_sensor_info),
    SHELL_SUBCMD_SET_END
);

/* Create and register the root shell command context entry point globally */
SHELL_CMD_REGISTER(sensorroot, &sub_sensorroot, "Custom LED-based sensor driver command suite", NULL);

int main(void)
{
    /* Hand over processing authority cleanly to the thread background monitor */
    return 0;
}
