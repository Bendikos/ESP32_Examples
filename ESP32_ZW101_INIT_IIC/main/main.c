/* UART Events Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "zw101.h"

// UART2 配置 tx-gpio6 rx-gpio7
#define EX_UART_NUM UART_NUM_2
#define PATTERN_CHR_NUM (1) /*!< Set the number of consecutive and identical characters received by receiver which defines a UART pattern*/

#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)

/**
 *0X00 刚初始化系统
 *0X01 读索引表
 *0X02 注册指纹
 *0X03 删除指纹
 *0X04 验证指纹
 *0X0A 取消指令
 */
uint8_t STATE = 0;

// 指纹的数量
uint8_t FINGERPRINT_NUMBER = 0;

// 数组下标是flash中指纹的ID号
uint8_t FINGERPRINT_DATA[64];

static QueueHandle_t uart2_queue;

static void uart_event_task(void *pvParameters)
{
    uart_event_t event;
    uint8_t *dtmp = (uint8_t *)malloc(RD_BUF_SIZE);
    for (;;)
    {
        // Waiting for UART event.
        if (xQueueReceive(uart2_queue, (void *)&event, (TickType_t)portMAX_DELAY))
        {
            bzero(dtmp, RD_BUF_SIZE);
            size_t buffered_size;
            uint8_t i = 0, j = 0, num = 0;
            switch (event.type)
            {
            case UART_DATA:
                if (STATE == 0X01 && event.size >= 11)
                {
                    ESP_LOGI(TAG, "读索引表-收到的字节数: %d", event.size);
                    uart_read_bytes(EX_UART_NUM, dtmp, event.size, portMAX_DELAY);
                    ESP_LOG_BUFFER_HEX(TAG, dtmp, event.size); // 打印接收到的数据
                    uint8_t data_th = 0;

                    for (i = 10; i < 18; i++)
                    {
                        data_th = dtmp[i];
                        for (j = 0; j < 8; j++)
                        {
                            FINGERPRINT_DATA[num] = (data_th >> j) & 0x01;
                            num++;
                        }
                    }
                    for (i = 0; i < 64; i++)
                    {
                        if (FINGERPRINT_DATA[i] == 1)
                        {
                            ESP_LOGI(TAG, "存在指纹-id: %d", i);
                            FINGERPRINT_NUMBER++;
                        }
                    }
                    ESP_LOGI(TAG, "指纹数量: %d", FINGERPRINT_NUMBER);
                }
                if (STATE == 0X02 && event.size >= 11)
                {
                    // ESP_LOGI(TAG, "注册指纹-收到的字节数: %d", event.size);
                    uart_read_bytes(EX_UART_NUM, dtmp, event.size, portMAX_DELAY);
                    // ESP_LOG_BUFFER_HEX(TAG, dtmp, event.size); // 打印接收到的数据
                    if (dtmp[10] == 0x00 && dtmp[11] == 0x00)
                    {
                        if (dtmp[9] == 0x00)
                            ESP_LOGI(TAG, "注册指令合法");
                    }
                    else if (dtmp[11] <= 0x0A)
                    {
                        if (dtmp[9] == 0x00 && dtmp[10] == 0x03)
                            ESP_LOGI(TAG, "第%d次指纹捕捉成功", dtmp[11]);
                        if (dtmp[9] == 0x26)
                            ESP_LOGI(TAG, "注册指纹超时");
                    }
                    if (dtmp[11] == 0xF0)
                    {
                        if (dtmp[9] == 0x00)
                            ESP_LOGI(TAG, "合并模板成功");
                    }
                    if (dtmp[11] == 0xF1)
                    {
                        if (dtmp[9] == 0x00)
                            ESP_LOGI(TAG, "已注册检测通过");
                    }
                    if (dtmp[11] == 0xF2)
                    {
                        if (dtmp[9] == 0x00)
                            ESP_LOGI(TAG, "模板存储成功");
                    }
                }
                if (STATE == 0X03 && event.size >= 11)
                {
                    ESP_LOGI(TAG, "删除指纹-收到的字节数: %d", event.size);
                    uart_read_bytes(EX_UART_NUM, dtmp, event.size, portMAX_DELAY);
                    ESP_LOG_BUFFER_HEX(TAG, dtmp, event.size); // 打印接收到的数据
                    if (dtmp[9] == 0x00)
                    {
                        ESP_LOGI(TAG, "删除指纹-成功");
                        if (FINGERPRINT_NUMBER >= 1)
                            FINGERPRINT_NUMBER--;
                    }
                    if (dtmp[9] == 0x01)
                    {
                        ESP_LOGI(TAG, "删除指纹-收包有错");
                    }
                    if (dtmp[9] == 0x10)
                    {
                        ESP_LOGI(TAG, "删除指纹-失败");
                    }
                }
                if (STATE == 0X04 && event.size >= 11)
                {
                    ESP_LOGI(TAG, "验证指纹-收到的字节数: %d", event.size);
                    uart_read_bytes(EX_UART_NUM, dtmp, event.size, portMAX_DELAY);
                    ESP_LOG_BUFFER_HEX(TAG, dtmp, event.size); // 打印接收到的数据
                    if (dtmp[10] == 0x00)
                    {
                        if (dtmp[9] == 0x00)
                            ESP_LOGI(TAG, "验证指纹-指令合法");
                    }
                    if (dtmp[10] == 0x01)
                    {
                        if (dtmp[9] == 0x00)
                            ESP_LOGI(TAG, "验证指纹-获取图像成功");
                        else if (dtmp[9] == 0x26)
                            ESP_LOGI(TAG, "验证指纹-获取图像超时");
                    }
                    if (dtmp[10] == 0x05)
                    {
                        if (dtmp[9] == 0x00)
                            ESP_LOGI(TAG, "验证指纹-搜到了指纹");
                        else if (dtmp[9] == 0x09)
                            ESP_LOGI(TAG, "验证指纹-没搜索到指纹");
                        else if (dtmp[9] == 0x24)
                            ESP_LOGI(TAG, "验证指纹-指纹库为空");
                    }
                }
                if (STATE == 0X0A && event.size >= 11)
                {
                    ESP_LOGI(TAG, "取消指令-收到的字节数: %d", event.size);
                    uart_read_bytes(EX_UART_NUM, dtmp, event.size, portMAX_DELAY);
                    ESP_LOG_BUFFER_HEX(TAG, dtmp, event.size); // 打印接收到的数据
                    if (dtmp[9] == 0x00)
                    {
                        ESP_LOGI(TAG, "取消指令-成功");
                    }
                    else if (dtmp[9] == 0x01)
                    {
                        ESP_LOGI(TAG, "取消指令-失败");
                    }
                    else if (dtmp[9] == 0x31)
                    {
                        ESP_LOGI(TAG, "取消指令-功能与加密等级不匹配");
                    }
                }
                break;

            case UART_PATTERN_DET:
                uart_get_buffered_data_len(EX_UART_NUM, &buffered_size);
                int pos = uart_pattern_pop_pos(EX_UART_NUM);
                ESP_LOGI(TAG, "[UART PATTERN DETECTED] pos: %d, buffered size: %d", pos, buffered_size);
                if (pos == -1)
                {
                    // There used to be a UART_PATTERN_DET event, but the pattern position queue is full so that it can not
                    // record the position. We should set a larger queue size.
                    // As an example, we directly flush the rx buffer here.
                    uart_flush_input(EX_UART_NUM);
                }
                else
                {
                    uart_read_bytes(EX_UART_NUM, dtmp, pos, 100 / portTICK_PERIOD_MS);
                    uint8_t pat[PATTERN_CHR_NUM + 1];
                    memset(pat, 0, sizeof(pat));
                    uart_read_bytes(EX_UART_NUM, pat, PATTERN_CHR_NUM, 100 / portTICK_PERIOD_MS);
                    if (pat[0] == 0X55)
                    {
                        ESP_LOGI(TAG, "pat[0] == 0X55 ZW101初始化完成");
                        ZW101_ReadIndexTable();
                        if (STATE == 0X00)
                            STATE = 0X01;
                    }
                }
                break;

            default:
                break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}

void app_main(void)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);

    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    // Install UART driver, and get the queue.
    uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, &uart2_queue, 0);
    uart_param_config(EX_UART_NUM, &uart_config);

    // Set UART log level
    esp_log_level_set(TAG, ESP_LOG_INFO);
    // Set UART pins (using UART0 default pins ie no changes.)
    uart_set_pin(EX_UART_NUM, 6, 7, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // Set uart pattern detect function.
    uart_enable_pattern_det_baud_intr(EX_UART_NUM, 0x55, PATTERN_CHR_NUM, 9, 100, 100);

    // Reset the pattern queue length to record at most 20 pattern positions.
    uart_pattern_queue_reset(EX_UART_NUM, 20);

    // Create a task to handler UART event from ISR
    xTaskCreate(uart_event_task, "uart_event_task", 3072, NULL, 12, NULL);

    while (1)
    {

        // ZW101_ReadSysPara();
        // ZW101_AutoIdentify();

        // if (!zw101_package(buffer1, sizeof(buffer1)))
        //     uart_write_bytes(EX_UART_NUM, buffer1, sizeof(buffer1));
        vTaskDelay(pdMS_TO_TICKS(3000));

        // STATE = 3;
        // ZW101_DeletChar(3);

        // if (STATE != 0X00)
        // {
        //     STATE = 0X0A;
        //     ZW101_Cancel();
        // }
            
        // vTaskDelay(pdMS_TO_TICKS(100));
        STATE = 0X04;
        ZW101_AutoIdentify();

        // ESP_LOGI(TAG, "指纹数量: %d", FINGERPRINT_NUMBER);
        // vTaskDelay(pdMS_TO_TICKS(6000));

        // STATE = 0X0A;
        // ZW101_Cancel();
        // STATE = 0X02;
        // ZW101_AutoEnroll(3);
    }
}
