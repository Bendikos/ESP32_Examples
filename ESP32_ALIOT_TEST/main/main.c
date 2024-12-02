#include <stdio.h>
#include "mqtt_client.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/FreeRTOS.h"
#include "mqtt_aliot.h"
#include "button.h"
#include "driver/gpio.h"
#include "aliot_dm.h"

SemaphoreHandle_t xSemaphore;

static const char *TAG = "mqtt";

#define DEFAULT_WIFI_SSID "vivoX80"
#define DEFAULT_WIFI_PASSWORD "00000000"

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        esp_wifi_connect();
        ESP_LOGI(TAG, "Disconnected. Trying to reconnect...");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        xSemaphoreGiveFromISR(xSemaphore, NULL);
    }
}

static EventGroupHandle_t s_button_event = NULL;
#define SHORT_EV (BIT0)

void btn_short_press(void)
{
    xEventGroupSetBits(s_button_event, SHORT_EV);
}
void button_task(void *param)
{
    button_config_t button_cfg =
        {
            .gpio_num = GPIO_NUM_15,     // gpio号
            .active_level = 0,           // 按下的电平
            .long_press_time = 3000,     // 长按时间
            .short_cb = btn_short_press, // 短按回调函数
            .long_cb = NULL,             // 长按回调函数
        };
    s_button_event = xEventGroupCreate();
    button_event_set(&button_cfg);

    gpio_config_t led_gpio =
        {
            .intr_type = GPIO_INTR_DISABLE,
            .mode = GPIO_MODE_OUTPUT,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pin_bit_mask = (1ull << GPIO_NUM_10),

        };

    gpio_config(&led_gpio);
    EventBits_t ev;
    uint8_t led_level = 0;
    while (1)
    {
        ev = xEventGroupWaitBits(s_button_event, SHORT_EV, pdTRUE, pdFALSE, pdMS_TO_TICKS(5000));
        if (ev & SHORT_EV)
        {
            led_level = led_level ? 0 : 1;
            gpio_set_level(GPIO_NUM_10, led_level);
            if (is_mqtt_connected())
            {
                aliot_post_property_int("LightSwitch", led_level);
            }
        }
    }
}

void app_main(void)
{

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());                // 用于初始化tcpip协议栈
    ESP_ERROR_CHECK(esp_event_loop_create_default()); // 创建一个默认系统事件调度循环，之后可以注册回调函数来处理系统的一些事件
    esp_netif_create_default_wifi_sta();              // 使用默认配置创建STA对象

    // 初始化WIFI
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 注册事件处理器
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);

    // WIFI配置
    wifi_config_t wifi_config =
        {
            .sta =
                {
                    .ssid = DEFAULT_WIFI_SSID,                // WIFI的SSID
                    .password = DEFAULT_WIFI_PASSWORD,        // WIFI密码
                    .threshold.authmode = WIFI_AUTH_WPA2_PSK, // 加密方式
                },
        };
    // 启动WIFI
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));               // 设置工作模式为STA
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config)); // 设置wifi配置
    ESP_ERROR_CHECK(esp_wifi_start());                               // 启动WIFI

    xSemaphore = xSemaphoreCreateBinary();
    xSemaphoreTake(xSemaphore, pdMS_TO_TICKS(10000));

    // 检测wifi是否连接
    wifi_ap_record_t ap_info;
    while (esp_wifi_sta_get_ap_info(&ap_info))
    {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    vTaskDelay(pdMS_TO_TICKS(8000));
    ESP_LOGI(TAG, "wifi_init_sta finished.");
    xTaskCreatePinnedToCore(button_task, "button", 4096, NULL, 2, NULL, 1);
    mqtt_start();
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
