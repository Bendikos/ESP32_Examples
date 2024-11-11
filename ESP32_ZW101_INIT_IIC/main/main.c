#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
// #include "uart_init.h"
// #include "gpio_init.h"
#include "zw101.h"
#include "driver/uart.h"
#include "driver/gpio.h"
typedef struct queue_uart
{
    uint8_t buffer[MY_ZW101_PACK_BUFF_LEN];
    uint16_t len;
} queue_uart_node;

static const char *TAG = "main";
uart_node_t uart1_node;
static my_zw101_node zw101_node = {}; // zw101包
gpio_pin_t touch_pin;// zw101触摸感应引脚，如果触摸，则为1，反之为0
gpio_pin_t ctrl_pin;// zw101模组电源控制引脚
gpio_pin_t mag_pin;// 锁控制
static QueueHandle_t xQueue = NULL;

/**
 * zw101驱动配置函数
 */
static void zw101_div(my_zw101_node *node)
{
    static const char *TAG = "main zw101";
    ESP_LOGI(TAG, "len = [%d]", node->pack_buffer_len);
    ESP_LOG_BUFFER_HEXDUMP(TAG, node->pack_buffer, node->pack_buffer_len, ESP_LOG_INFO);
    uart_write_bytes(uart1_node.uart_num, node->pack_buffer, node->pack_buffer_len);
    vTaskDelay(pdMS_TO_TICKS(200));
}

// UART 初始化函数
static void uart_config_init(uart_node_t *node)
{
    uart_config_t uart_config = {
        .baud_rate = node->rate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    uart_param_config(node->uart_num, &uart_config);
    uart_set_pin(node->uart_num, node->tx_pin, node->rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(node->uart_num, node->buffer, 0, 0, NULL, 0);
}

/**
 * 整体初始化和配置
 */
static void config_init(void)
{

    // UART1 配置 tx-gpio22 rx-gpio23
    uart1_node.uart_num = UART_NUM_2;
    uart1_node.rate = 115200;
    uart1_node.rx_pin = GPIO_NUM_23;
    uart1_node.tx_pin = GPIO_NUM_22;
    uart1_node.buffer = 1024;

    uart_config_init(&uart1_node);

    // Touch pin 配置
    touch_pin.pin = GPIO_NUM_2;
    touch_pin.status = 0;
    touch_pin.config.pin_bit_mask = (1ULL << touch_pin.pin);
    touch_pin.config.mode = GPIO_MODE_INPUT;
    touch_pin.config.pull_up_en = GPIO_PULLUP_DISABLE;
    touch_pin.config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    touch_pin.config.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&touch_pin.config);

    // Ctrl pin 配置
    ctrl_pin.pin = GPIO_NUM_18;
    ctrl_pin.status = 0;
    ctrl_pin.config.pin_bit_mask = (1ULL << ctrl_pin.pin);
    ctrl_pin.config.mode = GPIO_MODE_OUTPUT;
    ctrl_pin.config.pull_up_en = GPIO_PULLUP_DISABLE;
    ctrl_pin.config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    ctrl_pin.config.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&ctrl_pin.config);

    // Mag pin 配置
    mag_pin.pin = GPIO_NUM_19;
    mag_pin.status = 0;
    mag_pin.config.pin_bit_mask = (1ULL << mag_pin.pin);
    mag_pin.config.mode = GPIO_MODE_OUTPUT;
    mag_pin.config.pull_up_en = GPIO_PULLUP_DISABLE;
    mag_pin.config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    mag_pin.config.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&mag_pin.config);

    // ZW101 设备配置
    zw101_node.address = 0xffffffff;
    zw101_node.div_p = zw101_div;
}

/**
 * 串口接收任务
 */
static void uart1_read_task(void *arg)
{
    // vTaskDelay(pdMS_TO_TICKS(20000000));

    static const char *TAG = "main uart1 read";
    queue_uart_node *node = (queue_uart_node *)malloc(sizeof(queue_uart_node));
    assert(node);
    uint8_t *data = (uint8_t *)malloc(uart1_node.buffer * sizeof(uint8_t));
    assert(data);
    for (;;)
    {
        int len = uart_read_bytes(uart1_node.uart_num, data, (uart1_node.buffer - 1), pdMS_TO_TICKS(50));
        if (len)
        {
            ESP_LOGI(TAG, "len = [%d]", len);
            ESP_LOG_BUFFER_HEXDUMP(TAG, data, len, ESP_LOG_INFO);
            node->len = len;
            if (node->len < MY_ZW101_PACK_BUFF_LEN)
            {
                memcpy((char *)node->buffer, (char *)data, node->len);
                xQueueSend(xQueue, node, (TickType_t)0);
            }
        }
    }
    free(node);
    free(data);
}

/**
 * 任务初始化
 */
static void task_init(void)
{
    xQueue = xQueueCreate(1, sizeof(queue_uart_node));
    xTaskCreate(uart1_read_task, "uart read", 4 * 1024, NULL, 3, NULL);
}

void app_main(void)
{
    static queue_uart_node node;
    ESP_LOGI(TAG, "main start");
    config_init();
    task_init();
    ESP_LOGI(TAG, "init end");
    // 开启电源
    ctrl_pin.status = 1;
    gpio_set_level(ctrl_pin.pin, ctrl_pin.status);
    vTaskDelay(pdMS_TO_TICKS(200));
    // 校验传感器是否正常工作
    my_zw101_ps_check_sensor(&zw101_node);

    vTaskDelay(pdMS_TO_TICKS(200));
    while (xQueueReceive(xQueue, &node, (TickType_t)0) == pdTRUE)
    {
        memcpy(zw101_node.pack_buffer, node.buffer, node.len);
        zw101_node.pack_buffer_len = node.len;
        uint16_t head, len;
        if (my_zw101_answer_check(&zw101_node, &head, &len) == 0)
        {
            ESP_LOGI(TAG, "answer check success");
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    vTaskDelay(pdMS_TO_TICKS(200));

    // 进入休眠状态
    // my_zw101_ps_sleep(&zw101_node);
    // while (xQueueReceive(xQueue, &node, (TickType_t)0) == pdTRUE)
    // {
    //     memcpy(zw101_node.pack_buffer, node.buffer, node.len);
    //     zw101_node.pack_buffer_len = node.len;
    //     uint16_t head, len;
    //     if (my_zw101_answer_check(&zw101_node, &head, &len) == 0)
    //     {
    //         ESP_LOGI(TAG, "answer check success");
    //     }
    //     vTaskDelay(pdMS_TO_TICKS(100));
    // }
    // ESP_LOGI(TAG, "close power");
    // 关闭电源

    ctrl_pin.status = 0;
    gpio_set_level(ctrl_pin.pin, ctrl_pin.status);

    for (;;)
    {
        if (ctrl_pin.status == 0)
        {
            touch_pin.status = gpio_get_level(touch_pin.pin);
            if (touch_pin.status == 1 || true)
            {
                ESP_LOGI(TAG, "pin status = [%d]", touch_pin.status);
                ctrl_pin.status = 1;
                gpio_set_level(ctrl_pin.pin, ctrl_pin.status); // 开启电源
                vTaskDelay(pdMS_TO_TICKS(100));
                my_zw101_ps_auto_identify(&zw101_node, 1, 0xffff, 0x0000); // 验证指纹
                uint8_t zw101_ps_auto_flag = 0;                            // 验证是否成功，标识0为成功，1为失败
                while (xQueueReceive(xQueue, &node, (TickType_t)0) == pdTRUE)
                {
                    memcpy(zw101_node.pack_buffer, node.buffer, node.len);
                    zw101_node.pack_buffer_len = node.len;
                    uint16_t head, len;
                    if (my_zw101_answer_check(&zw101_node, &head, &len) == 0)
                    {
                        ESP_LOGI(TAG, "answer check read");
                        ESP_LOGI(TAG, "head = [%d]", head);
                        // 校验
                        if (zw101_node.pack_buffer[head] == 0x00)
                        {
                            // todo处理数据，可以增加身份显示功能和验证符合百分比信息
                            if (zw101_node.pack_buffer[head + 1] == 0x00)
                            {
                                ESP_LOGI(TAG, "answer check 0x00");
                            }
                            else if (zw101_node.pack_buffer[head + 1] == 0x01)
                            {
                                ESP_LOGI(TAG, "answer check 0x01");
                            }
                            else if (zw101_node.pack_buffer[head + 1] == 0x05)
                            {
                                ESP_LOGI(TAG, "answer check 0x05");
                            }
                        }
                        else
                        {
                            zw101_ps_auto_flag = 1;
                        }
                    }
                    vTaskDelay(pdMS_TO_TICKS(100));
                }
                // 休眠模块
                my_zw101_ps_sleep(&zw101_node);
                while (xQueueReceive(xQueue, &node, (TickType_t)0) == pdTRUE)
                {
                    memcpy(zw101_node.pack_buffer, node.buffer, node.len);
                    zw101_node.pack_buffer_len = node.len;
                    uint16_t head, len;
                    if (my_zw101_answer_check(&zw101_node, &head, &len) == 0)
                    {
                        ESP_LOGI(TAG, "zw101 sleep...");
                    }
                    vTaskDelay(pdMS_TO_TICKS(100));
                }
                // 关闭电源
                ctrl_pin.status = 0;
                gpio_set_level(ctrl_pin.pin, ctrl_pin.status);
                // todo开启门或关闭门
                if (zw101_ps_auto_flag == 0)
                {
                    ESP_LOGI(TAG, "open door");
                }
                else
                {
                    ESP_LOGI(TAG, "not open door");
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    vQueueDelete(xQueue);
}