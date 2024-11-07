#include <stdio.h>
#include "FT6336U_Driver.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"

i2c_master_bus_handle_t bus_handle;
i2c_master_dev_handle_t touch_handle;
SemaphoreHandle_t touch_semaphore;

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    // 触发中断时释放信号量
    if (gpio_get_level(FT6336U_INT_IO) == 0)
        xSemaphoreGiveFromISR(touch_semaphore, NULL);
}

static void dev_touch_initialization(void)
{
    gpio_install_isr_service(0);
    gpio_isr_handler_add(FT6336U_INT_IO, gpio_isr_handler, NULL); // 添加中断服务程序
    ft6336u_init();
    vTaskDelay(35);
}

static void log_touch_data(const FT6336U_TOUCH_POS *touch)
{
    ESP_LOGI("TOUCH", "touch_num : %d ", touch->touch_num);
    if (touch->touch_num == 1)
        ESP_LOGI("TOUCH", "x1: %d , y1 : %d", touch->touch0_x, touch->touch0_y);
    else if (touch->touch_num == 2)
        ESP_LOGI("TOUCH", "x1: %d , y1 : %d , x2: %d , y2 : %d", touch->touch0_x, touch->touch0_y, touch->touch1_x, touch->touch1_y);
}

// 中断模式任务：响应触摸数据
void test_interrupt_touch(void *pvParameters)
{
    FT6336U_TOUCH_POS touch;

    while (1)
    {
        // 等待信号量被释放
        if (xSemaphoreTake(touch_semaphore, portMAX_DELAY) == pdTRUE)
        {
            ft6336u_read_touch_pos(&touch);
            log_touch_data(&touch);
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void app_main(void)
{
    // 创建信号量
    touch_semaphore = xSemaphoreCreateBinary();
    if (touch_semaphore == NULL)
    {
        ESP_LOGE("MAIN", "Failed to create touch_semaphore");
        return;
    }

    dev_touch_initialization();

    FT6336U_IC_INFO info;
    ft6336u_read_ic_info(&info);
    ESP_LOGI("IC_INFO", "CIPHER: 0x%lx, LIB_VERSION: 0x%x, FIRMWARE_VERSION: 0x%x, VENDOR_ID: 0x%x",
             info.CPIPHER, info.LIB_VERSION, info.FIRMWARE_VERSION, info.VENDOR_ID);

    // 创建触摸任务
    xTaskCreate(test_interrupt_touch, "Touch Interrupt Task", 2048, NULL, 10, NULL);
    while (1)
    {
        ESP_LOGI("IC_INFO","hello world!");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
    
}
