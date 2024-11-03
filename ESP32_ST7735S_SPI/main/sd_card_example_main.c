#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "LCD_ST7735S.h"

// // SPI 发送数据，选择设备句柄
// esp_err_t spi_send_data(spi_device_handle_t spi_handle, const char *data, size_t length)
// {
//     spi_transaction_t trans = {
//         .length = length * 8,  // 转换为比特
//         .tx_buffer = data,
//     };

//     esp_err_t ret = spi_device_transmit(spi_handle, &trans);
//     if (ret != ESP_OK) {
//         ESP_LOGE(TAG, "Failed to send data");
//     }

//     return ret;
// }

// // SPI 接收数据，选择设备句柄
// esp_err_t spi_receive_data(spi_device_handle_t spi_handle, char *data, size_t length)
// {
//     spi_transaction_t trans = {
//         .length = length * 8,
//         .rxlength = length * 8,
//         .rx_buffer = data,
//     };

//     esp_err_t ret = spi_device_transmit(spi_handle, &trans);
//     if (ret != ESP_OK) {
//         ESP_LOGE(TAG, "Failed to receive data");
//     }

//     return ret;
// }

void app_main(void)
{
    // esp_err_t ret = init_spi_bus();
    // if (ret != ESP_OK) {
    //     ESP_LOGE(TAG, "SPI initialization failed");
    //     return;
    // }

    // const char *send_data1 = "Hello, Device 1!";
    // ESP_LOGI(TAG, "Sending data to Device 1: %s", send_data1);

    // ret = spi_send_data(spi_handle1, send_data1, strlen(send_data1));
    // if (ret == ESP_OK) {
    //     ESP_LOGI(TAG, "Data sent to Device 1 successfully");
    // }
    // // 读取设备1的数据
    // char recv_data1[EXAMPLE_MAX_CHAR_SIZE] = {0};
    // ret = spi_receive_data(spi_handle1, recv_data1, sizeof(recv_data1) - 1);
    // if (ret == ESP_OK) {
    //     ESP_LOGI(TAG, "Data received from Device 1: %s", recv_data1);
    // }

    // 释放 SPI 总线
    // spi_bus_remove_device(spi_handle1);
    // spi_bus_free(SPI2_HOST);
    lcdInit();
    uint8_t str[] = "HELLO_LCD!";
    LCD_ShowString(0, 0, 128, 160, 16, str, RED, WHITE);
    GUI_sprintf_hz3232(40, 40, (unsigned char *)"欢", BLUE, WHITE);
    GUI_sprintf_hz32x(0, 110, (unsigned char *)"欢迎使用", BLUE, WHITE);

    while (1)
    {
        ESP_LOGI(TAG, "delay\n");
        vTaskDelay(pdMS_TO_TICKS(1000));

    }
}
