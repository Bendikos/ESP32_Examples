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
#include "freertos/queue.h"
#include "driver/gpio.h"

#define I2C_MASTER_SCL_IO 22      /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 21      /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_0  /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 100000 /*!< I2C master clock frequency */

i2c_master_bus_handle_t bus_handle;
i2c_master_dev_handle_t pn532_handle;

int ret;
uint8_t ver[2];
uint8_t cardid[8];
uint8_t id_len;

uint8_t buf[] = {CMD_IN_LIST_PASSIVE_TARGET, FIND_NFCCARD_MAXNUM, NFC_106K_PROTOCOL};
uint8_t pack[BUF_LENGTH];
uint8_t res[BUF_LENGTH];
uint8_t pdata[RES_LENGTH];

static QueueHandle_t gpioEventQueue = NULL;

static void IRAM_ATTR intrHandler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    xQueueSendFromISR(gpioEventQueue, &gpio_num, NULL);
}

static void pn532_test_task(void *arg)
{
    uint32_t ioNum = (uint32_t)arg;

    while (1)
    {
        if (xQueueReceive(gpioEventQueue, &ioNum, portMAX_DELAY))
        {
            i2c_master_receive(pn532_handle, res, 23, 100);
            if (res[0] == 0x01)
            {
                ret = PN532_Parse(&res[1], 22, pdata);
                if (ret > 0)
                {
                    id_len = pdata[6];
                    memcpy(cardid, &pdata[7], id_len);
                    ESP_LOGI(TAG, "done");
                    for (int i = 0; i < id_len; i++)
                    {
                        ESP_LOGI(TAG, "%02x ", cardid[i]);
                    }
                }
                ret = PN532_Package(buf, 3, pack);
                if (PN532_Write_WaitAck(pack, ret, 1000) == PN532_OK)
                    vTaskDelay(pdMS_TO_TICKS(200));
            }
        }
    }
}
void dev_pn532_initialization(void)
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
        .device_address = PN532_I2C_ADDRESS,
        .scl_speed_hz = 100000,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &pn532_handle));
    ESP_LOGI(TAG, "create PN532 device");

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
    ESP_LOGI(TAG, "PN532 Version %d.%d", ver[0], ver[1]);

    if ((ver[0] != 0x01) || (ver[1] != 0x06))
    {
        ESP_LOGI(TAG, "Version not match!");
    }

    ret = PN532_SAMConfig();
    if (ret != PN532_OK)
    {
        ESP_LOGI(TAG, "PN532 SAMConfig ERROR.");
        while (1)
        {
        }
    }

    ret = PN532_Package(buf, 3, pack);
    if (PN532_Write_WaitAck(pack, ret, 1000) == PN532_OK)
        ESP_LOGI(TAG, "ready to receive data");

    gpio_config_t irq_gpio_config = {
        .pin_bit_mask = (1ULL << PN532_IRQ_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    gpio_config(&irq_gpio_config);
    gpio_set_level(PN532_IRQ_PIN, 1);
    gpioEventQueue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(pn532_test_task, "pn532_test_task", 2048, NULL, 10, NULL);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(PN532_IRQ_PIN, intrHandler, (void *)PN532_IRQ_PIN);
    ESP_LOGI(TAG, "finished");

    // xTaskCreate(&pn532_test_task, "pn532_test_task", 2048 * 2, NULL, 5, NULL);

    // if (PN532_Response(res, 23, -1) != PN532_OK)
    //     return PN532_TIMEOUT;

    // if (res[0] == 0x01)
    // {
    //     ret = PN532_Parse(&res[1], 22, pdata);
    //     if (ret > 0)
    //     {
    //         id_len = pdata[6];
    //         memcpy(cardid, &pdata[7], id_len);
    //         return PN532_OK;
    //     }
    // }
}

void app_main(void)
{

    dev_pn532_initialization();
    while (1)
    {
        
        printf("mian function is running! \n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    //     i2c_master_bus_rm_device(pn532_handle);
    //     i2c_del_master_bus(bus_handle);
}
