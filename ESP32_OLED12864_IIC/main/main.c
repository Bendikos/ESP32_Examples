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
#include "ssd1306.h"
#include "ssd1306_fonts.h"

#define I2C_MASTER_SCL_IO 22      /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 21      /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_0  /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 100000 /*!< I2C master clock frequency */

static const char *TAG = "ssd1306 test";

static bool test_running;

time_t g_now;

i2c_master_bus_handle_t bus_handle;
i2c_master_dev_handle_t dev_handle;

extern uint8_t s_chDisplayBuffer[128][8];

/******** Test Function ****************/
esp_err_t ssd1306_show_time(i2c_master_dev_handle_t dev_handle)
{
    struct tm timeinfo;
    char strftime_buf[64];
    time(&g_now);
    g_now++;
    setenv("TZ", "GMT-8", 1);
    tzset();
    localtime_r(&g_now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);

    ssd1306_draw_3216char(dev_handle, 0, 16, strftime_buf[11]);
    ssd1306_draw_3216char(dev_handle, 16, 16, strftime_buf[12]);
    ssd1306_draw_3216char(dev_handle, 32, 16, strftime_buf[13]);
    ssd1306_draw_3216char(dev_handle, 48, 16, strftime_buf[14]);
    ssd1306_draw_3216char(dev_handle, 64, 16, strftime_buf[15]);
    ssd1306_draw_1616char(dev_handle, 80, 32, strftime_buf[16]);
    ssd1306_draw_1616char(dev_handle, 96, 32, strftime_buf[17]);
    ssd1306_draw_1616char(dev_handle, 112, 32, strftime_buf[18]);

    char *day = strftime_buf;
    day[3] = '\0';
    ssd1306_draw_string(dev_handle, 87, 16, (const uint8_t *)day, 14, 1);
    ssd1306_draw_string(dev_handle, 0, 52, (const uint8_t *)"MUSIC", 12, 0);
    ssd1306_draw_string(dev_handle, 52, 52, (const uint8_t *)"MENU", 12, 0);
    ssd1306_draw_string(dev_handle, 98, 52, (const uint8_t *)"PHONE", 12, 0);

    return ssd1306_refresh_gram(dev_handle);
}

esp_err_t ssd1306_show_signs(i2c_master_dev_handle_t dev_handle)
{
    ssd1306_clear_screen(dev_handle, 0x00);

    ssd1306_draw_bitmap(dev_handle, 0, 2, &c_chSingal816[0], 16, 8);
    ssd1306_draw_bitmap(dev_handle, 24, 2, &c_chBluetooth88[0], 8, 8);
    ssd1306_draw_bitmap(dev_handle, 40, 2, &c_chMsg816[0], 16, 8);
    ssd1306_draw_bitmap(dev_handle, 64, 2, &c_chGPRS88[0], 8, 8);
    ssd1306_draw_bitmap(dev_handle, 90, 2, &c_chAlarm88[0], 8, 8);
    ssd1306_draw_bitmap(dev_handle, 112, 2, &c_chBat816[0], 16, 8);

    return ssd1306_refresh_gram(dev_handle);
}


static void dev_ssd1306_initialization(void)
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
        .device_address = OLED_ADDRESS,
        .scl_speed_hz = 100000,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));
    ssd1306_init(dev_handle);
    ssd1306_show_signs(dev_handle);
}

static void ssd1306_test_task(void *pvParameters)
{
    ESP_LOGI(TAG, "OLED task start");

    dev_ssd1306_initialization();
    
    while (test_running)
    {
        ssd1306_show_time(dev_handle);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    ESP_LOGI(TAG, "OLED cleaning-up...");

    ssd1306_clear_screen(dev_handle, 0);

    ssd1306_refresh_gram(dev_handle);

    i2c_master_bus_rm_device(dev_handle);
    i2c_del_master_bus(bus_handle);

    vTaskDelete(NULL);
}

void app_main(void)
{

    while (1)
    {

        test_running = true;

        xTaskCreate(&ssd1306_test_task, "ssd1306_test_task", 2048 * 2, NULL, 5, NULL);
        vTaskDelay(10000 / portTICK_PERIOD_MS); // run the test for 10 seconds

        ESP_LOGI(TAG, "OLED task start");
        test_running = false; // stop the test
        vTaskDelay(1000 / portTICK_PERIOD_MS); // give the kernel some time to clean-up
        ESP_LOGI(TAG, "OLED task end");
    }
}
