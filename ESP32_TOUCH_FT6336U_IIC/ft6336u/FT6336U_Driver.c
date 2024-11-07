#include "FT6336U_Driver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// 初始化相关io和必要配置
void ft6336u_init()
{
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_MASTER_NUM,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = FT6336U_Slave_Addr,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &touch_handle));

    gpio_config_t io_config = {
        .pin_bit_mask = (1ULL << FT6336U_RST_IO) | (1ULL << FT6336U_INT_IO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io_config);

    gpio_set_level(FT6336U_RST_IO, 1);
    io_config.mode = GPIO_MODE_INPUT;
    io_config.intr_type = GPIO_INTR_NEGEDGE;
    gpio_config(&io_config);
}

// 复位芯片
void ft6336u_reset()
{
    gpio_set_level(FT6336U_RST_IO, 0);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    gpio_set_level(FT6336U_RST_IO, 1);
}

// 读取芯片信息
void ft6336u_read_ic_info(FT6336U_IC_INFO *info)
{
    uint8_t cmds[] = {ID_G_CIPHER_HIGH, ID_G_CIPHER_MIDE, ID_G_CIPHER_LOW};
    uint8_t temp[3];
    for (int i = 0; i < 3; ++i)
    {
        ESP_ERROR_CHECK(i2c_master_transmit_receive(touch_handle, &cmds[i], 1, &temp[i], 1, -1));
    }
    info->CPIPHER = (temp[0] << 16) | (temp[1] << 8) | temp[2];

    cmds[0] = ID_G_LIB_VERSION_H;
    cmds[1] = ID_G_LIB_VERSION_L;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(touch_handle, &cmds[0], 1, &temp[0], 1, -1));
    ESP_ERROR_CHECK(i2c_master_transmit_receive(touch_handle, &cmds[1], 1, &temp[1], 1, -1));
    info->LIB_VERSION = (temp[0] << 8) | temp[1];

    cmds[0] = ID_G_FIRMID;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(touch_handle, cmds, 1, &info->FIRMWARE_VERSION, 1, -1));

    cmds[0] = ID_G_FOCALTECH_ID;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(touch_handle, cmds, 1, &info->VENDOR_ID, 1, -1));
}

// 读取触摸数据
void ft6336u_read_touch_pos(FT6336U_TOUCH_POS *touch_pos)
{
    uint8_t cmds[] = {TD_STATUS, P1_XH, P1_XL, P1_YH, P1_YL, P2_XH, P2_XL, P2_YH, P2_YL};
    uint8_t data[9];
    for (int i = 0; i < 9; ++i)
    {
        ESP_ERROR_CHECK(i2c_master_transmit_receive(touch_handle, &cmds[i], 1, &data[i], 1, -1));
    }
    touch_pos->touch_num = data[0];
    touch_pos->touch0_x = ((data[1] & 0x0F) << 8) | data[2];
    touch_pos->touch0_y = ((data[3] & 0x0F) << 8) | data[4];
    touch_pos->touch1_x = ((data[5] & 0x0F) << 8) | data[6];
    touch_pos->touch1_y = ((data[7] & 0x0F) << 8) | data[8];
}

// 读取功耗模式
uint8_t ft6336u_read_power_mode()
{
    uint8_t mode;
    uint8_t cmd = ID_G_PMODE;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(touch_handle, &cmd, 1, &mode, 1, -1));
    return mode;
}
