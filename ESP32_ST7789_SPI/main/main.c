#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "LCD_ST7789.h"

void app_main(void)
{
    lcdInit();
    uint8_t str[] = "HELLO_LCD!";
    LCD_ShowString(0, 0, 128, 160, 16, str, RED, WHITE);
    GUI_sprintf_hz3232(40, 40, (unsigned char *)"欢", BLUE, WHITE);
    GUI_sprintf_hz32x(0, 110, (unsigned char *)"欢迎使用", BLUE, WHITE);
    vTaskDelay(pdMS_TO_TICKS(1000));
    LCD_Fill(0, 0, LCD_W, LCD_H, RED); // 填充为红色背景色
    LCD_Fill(0, 0, LCD_W, LCD_H, WHITE); // 填充为白色背景色
    LCD_Fill(0, 0, LCD_W, LCD_H, BLUE); // 填充为红色背景色
    LCD_Fill(0, 0, LCD_W, LCD_H, GREEN); // 填充为红色背景色
    while (1)
    {
        ESP_LOGI(TAG, "delay\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
