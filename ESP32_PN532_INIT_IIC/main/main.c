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

uint8_t carduid[8];
// uint8_t pack[BUF_LENGTH];
// uint8_t res[BUF_LENGTH];
// uint8_t pdata[RES_LENGTH];

i2c_master_bus_handle_t bus_handle;
i2c_master_dev_handle_t pn532_handle;

SemaphoreHandle_t pn532_semaphore;

bool pn532_getuid;

static void IRAM_ATTR intrHandler(void *arg)
{
    if ((uint32_t)arg == PN532_IRQ_PIN && gpio_get_level(PN532_IRQ_PIN) == 0)
        xSemaphoreGiveFromISR(pn532_semaphore, NULL);
}

static void pn532_test_task(void *arg)
{
    // uint32_t ioNum = (uint32_t)arg;

    uint8_t id_len;
    int len;
    uint8_t buf[] = {CMD_IN_LIST_PASSIVE_TARGET, FIND_NFCCARD_MAXNUM, NFC_106K_PROTOCOL};
    uint8_t pack[BUF_LENGTH];
    uint8_t res[BUF_LENGTH];
    uint8_t pdata[RES_LENGTH];

    while (1)
    {
        // 等待信号量被释放
        if (xSemaphoreTake(pn532_semaphore, portMAX_DELAY) == pdTRUE)
        {

            i2c_master_receive(pn532_handle, res, 23, 100);
            if (res[0] == 0x01)
            {
                if (PN532_Parse(&res[1], 22, pdata) > 0)
                {
                    id_len = pdata[6];
                    memcpy(carduid, &pdata[7], id_len);
                    ESP_LOGI(TAG, "done");
                    for (int i = 0; i < id_len; i++)
                    {
                        ESP_LOGI(TAG, "%02x ", carduid[i]);
                    }
                    pn532_getuid = true;

                    ready_to_recive();
                }
                ESP_LOGI(TAG, "waitting1");
                vTaskDelay(pdMS_TO_TICKS(2000));
            }
            ESP_LOGI(TAG, "waitting2");
        }
    }
}

void app_main(void)
{
    pn532_semaphore = xSemaphoreCreateBinary();
    dev_pn532_initialization();

    xTaskCreate(pn532_test_task, "pn532_test_task", 2048, NULL, 10, NULL);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(PN532_IRQ_PIN, intrHandler, (void *)PN532_IRQ_PIN);
    ESP_LOGI(TAG, "init finished");
    pn532_getuid = false;
    while (1)
    {
        if (pn532_getuid)
        {
            for (uint8_t i = 0; i < 4; i++)
            {
                carduid[i] = 0xFF;
            }
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);

        if (pn532_getuid)
        {
            for (uint8_t i = 0; i < 4; i++)
            {
                if (carduid[i] != 0xFF)
                    printf("%02x", carduid[i]);
            }
        }

        printf("\n");
    }
    //     i2c_master_bus_rm_device(pn532_handle);
    //     i2c_del_master_bus(bus_handle);
}
