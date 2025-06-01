/* SPDX-License-Identifier: BSD-3-Clause */

#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(i2c_device_example, CONFIG_I2C_LOG_LEVEL);

/* 设备私有数据结构 */
struct i2c_device_example_data {
    const struct device *i2c;
    uint8_t i2c_addr;
    uint8_t state;
};

/* 设备配置结构 */
struct i2c_device_example_config {
    uint8_t i2c_addr;
    const char *i2c_bus;
};

/* 读取设备寄存器 */
static int i2c_device_example_read_reg(const struct device *dev, uint8_t reg, uint8_t *val)
{
    struct i2c_device_example_data *data = dev->data;
    uint8_t buf[1] = {reg};

    int ret = i2c_write_read_dt(data->i2c, buf, 1, val, 1);
    if (ret < 0) {
        LOG_ERR("Failed to read register 0x%02X: %d", reg, ret);
        return ret;
    }

    return 0;
}

/* 写入设备寄存器 */
static int i2c_device_example_write_reg(const struct device *dev, uint8_t reg, uint8_t val)
{
    struct i2c_device_example_data *data = dev->data;
    uint8_t buf[2] = {reg, val};

    int ret = i2c_write_dt(data->i2c, buf, 2);
    if (ret < 0) {
        LOG_ERR("Failed to write register 0x%02X: %d", reg, ret);
        return ret;
    }

    return 0;
}

/* 设备初始化函数 */
static int i2c_device_example_init(const struct device *dev)
{
    struct i2c_device_example_data *data = dev->data;
    const struct i2c_device_example_config *config = dev->config;

    data->i2c = device_get_binding(config->i2c_bus);
    if (!data->i2c) {
        LOG_ERR("I2C bus %s not found", config->i2c_bus);
        return -ENODEV;
    }

    data->i2c_addr = config->i2c_addr;

    /* 设备探测 */
    if (!i2c_is_ready_dt(data->i2c)) {
        LOG_ERR("Device not ready on bus %s at address 0x%02X",
                config->i2c_bus, config->i2c_addr);
        return -ENODEV;
    }

    LOG_INF("I2C device example initialized on bus %s at address 0x%02X",
            config->i2c_bus, config->i2c_addr);

    /* 设备初始化序列 */
    int ret = i2c_device_example_write_reg(dev, 0x00, 0x01);
    if (ret < 0) {
        LOG_ERR("Failed to initialize device: %d", ret);
        return ret;
    }

    data->state = 1;
    return 0;
}

/* 设备API实现 */
static int i2c_device_example_get_value(const struct device *dev, uint8_t *value)
{
    if (!device_is_ready(dev)) {
        LOG_ERR("Device not ready");
        return -ENODEV;
    }

    return i2c_device_example_read_reg(dev, 0x01, value);
}

static int i2c_device_example_set_config(const struct device *dev, uint8_t config)
{
    if (!device_is_ready(dev)) {
        LOG_ERR("Device not ready");
        return -ENODEV;
    }

    return i2c_device_example_write_reg(dev, 0x02, config);
}

/* 设备API结构体 */
static const struct i2c_device_example_api {
    int (*get_value)(const struct device *dev, uint8_t *value);
    int (*set_config)(const struct device *dev, uint8_t config);
} i2c_device_example_api_funcs = {
    .get_value = i2c_device_example_get_value,
    .set_config = i2c_device_example_set_config,
};

/* 设备数据实例 */
static struct i2c_device_example_data i2c_device_example_data;

/* 设备配置实例 */
static const struct i2c_device_example_config i2c_device_example_config = {
    .i2c_addr = 0x42,
    .i2c_bus = CONFIG_I2C_DEVICE_EXAMPLE_I2C_BUS_NAME,
};

/* 注册设备 */
DEVICE_DT_INST_DEFINE(0,
                      i2c_device_example_init,
                      NULL,
                      &i2c_device_example_data,
                      &i2c_device_example_config,
                      POST_KERNEL,
                      CONFIG_I2C_DEVICE_EXAMPLE_INIT_PRIORITY,
                      &i2c_device_example_api_funcs);    
