/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "pn532_i2c.h"

#define I2C_MASTER_SCL_IO 22      /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 21      /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_0  /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 100000 /*!< I2C master clock frequency */

static const char *TAG = "PN532";

static bool test_running;

i2c_master_bus_handle_t bus_handle;
i2c_master_dev_handle_t pn532_handle;

static void dev_pn532_initialization(void)
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
        .device_address = PN532_ADDRESS,
        .scl_speed_hz = 100000,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &pn532_handle));
    ESP_LOGI(TAG, "create PN532 device");
}

static void pn532_test_task(void *pvParameters)
{
    ESP_LOGI(TAG, "PN532 task start");

    dev_pn532_initialization();
    int ret;
    uint8_t ver[2];
    uint8_t cardid[8];
    uint8_t id_len;

    PN532_GPIO_Init();
    PN532_Reset();

    ret = PN532_Get_Version(ver);
    if (ret != PN532_OK)
    {
        ESP_LOGI(TAG, "Read Version ERROR.");
        while (1)
        {
        }
    }
    ESP_LOGI(TAG, "PN532 Version %d.%d.\r\n", ver[0], ver[1]);

    if ((ver[0] != 0x01) || (ver[1] != 0x06))
    {
        ESP_LOGI(TAG, "Version not match!\r\n");
    }

    ret = PN532_SAMConfig();
    if (ret != PN532_OK)
    {
        ESP_LOGI(TAG, "PN532 SAMConfig ERROR.");
        while (1)
        {
        }
    }
    // vtaskde
    // xTaskDelayUntil()
    while (test_running)
    {
        ret = PN532_ReadPassiveTargetID(cardid, &id_len);
        if (ret == PN532_OK)
        {
            ESP_LOGI(TAG, "Found Card\r\n");
            for (int i = 0; i < id_len; i++)
            {
                ESP_LOGI(TAG, "%02x ", cardid[i]);
            }
            test_running = false; 
            break;
        }
    }

    i2c_master_bus_rm_device(pn532_handle);
    i2c_del_master_bus(bus_handle);

    ESP_LOGI(TAG, "delete PN532 device");

    vTaskDelete(NULL);
}

void app_main(void)
{

    while (1)
    {

        test_running = true;

        xTaskCreate(&pn532_test_task, "pn532_test_task", 2048 * 2, NULL, 5, NULL);
        vTaskDelay(pdMS_TO_TICKS(1000)); // run the test for 10 seconds

        ESP_LOGI(TAG, "PN532 task start");
        test_running = false;                  // stop the test
        vTaskDelay(pdMS_TO_TICKS(1000)); // give the kernel some time to clean-up
        ESP_LOGI(TAG, "PN532 task end");
    }
}
