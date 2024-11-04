#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

static QueueHandle_t gpioEventQueue = NULL;
static void IRAM_ATTR intrHandler (void *arg){
    uint32_t gpio_num = (uint32_t)arg;
    xQueueSendFromISR(gpioEventQueue, &gpio_num, NULL);
}
static void gpioTaskExample(void *arg){
    uint32_t ioNum = (uint32_t) arg;
    while (1)
    {
        if (xQueueReceive(gpioEventQueue, &ioNum, portMAX_DELAY)){
            printf("GPIO[%d] intrrupted, level: %d\n", (int)ioNum, (int)gpio_get_level(ioNum));
        }
    }
    
}

void app_main(void)
{   
    
    gpio_config_t gpio18 = {
        .pin_bit_mask = 1ull << 18,
        .mode = GPIO_MODE_INPUT_OUTPUT,
        .intr_type = GPIO_INTR_ANYEDGE,
        .pull_down_en = 1,
    };
    gpio_config(&gpio18);
    gpio_set_level(18, 1);
    gpioEventQueue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(gpioTaskExample, "ExampleTask", 2048, NULL, 10, NULL);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(18, intrHandler, (void*)18);
    printf("finished");
    while (1)
    {
        printf("mian function is running! \n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    
}
