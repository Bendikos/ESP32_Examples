#include "zw101.h"
#include "string.h"

const uint8_t package_header[] = {
    0xEF, // 0xEF
    0x01, // 0x01
    0xFF, // 0xFF
    0xFF, // 0xFF
    0xFF, // 0xFF
    0xFF, // 0xFF
    0X01, // 0x01
    0x00,
};

/**
 * ZW101取消指令
 */
uint8_t ZW101_Cancel()
{
    uint8_t data[12] = {0};
    memcpy(data, package_header, 8);
    data[8] = 0x03;
    data[9] = 0X30;
    data[10] = 0X00;
    data[11] = 0X34;
    ESP_LOG_BUFFER_HEX(TAG, data, 12); // 打印接收到的数据
    // uart_write_bytes(UART_NUM_2, data, 17);
    return 0;
}
/**
 * ZW101握手指令
 */
uint8_t ZW101_HandShake()
{
    uint8_t data[12] = {0};
    memcpy(data, package_header, 8);
    data[8] = 0x03;
    data[9] = 0X35;
    data[10] = 0X00;
    data[11] = 0X39;
    ESP_LOG_BUFFER_HEX(TAG, data, 12); // 打印接收到的数据
    // uart_write_bytes(UART_NUM_2, data, 17);
    return 0;
}
/**
 * ZW101读索引表
 */
uint8_t ZW101_ReadIndexTable()
{
    uint8_t data[13] = {0};
    memcpy(data, package_header, 8);
    data[8] = 0x04;
    data[9] = 0X1F;
    data[10] = 0X00;
    data[11] = 0X00;
    data[12] = 0X24;
    uart_write_bytes(UART_NUM_2, data, 13);
    return 0;
}

/**
 * ZW101自动注册模板
 */
uint8_t ZW101_AutoEnroll(uint8_t number)
{
    uint8_t data[17] = {0};
    memcpy(data, package_header, 8);
    data[8] = 0x08;
    data[9] = 0X31;
    data[10] = 0X00;
    data[11] = number;
    data[12] = 0X05;
    data[13] = 0X00;
    data[14] = 0X09;
    data[15] = 0X00;
    data[16] = 0X48 + number;
    ESP_LOG_BUFFER_HEX(TAG, data, 17); // 打印接收到的数据
    // uart_write_bytes(UART_NUM_2, data, 17);
    return 0;
}

/**
 * ZW101删除模板
 */
uint8_t ZW101_DeletChar(uint8_t number)
{
    uint8_t data[16] = {0};
    memcpy(data, package_header, 8);
    data[8] = 0x07;
    data[9] = 0X0C;
    data[10] = 0X00;
    data[11] = 0X00;
    data[12] = 0X00;
    data[13] = number;
    data[14] = 0X00;
    data[15] = 0X14 + number;
    ESP_LOG_BUFFER_HEX(TAG, data, 16); // 打印接收到的数据
    // uart_write_bytes(UART_NUM_2, data, 17);
    return 0;
}

/**
 * ZW101自动验证指纹
 */
uint8_t ZW101_AutoIdentify()
{
    uint8_t data[17] = {0};
    memcpy(data, package_header, 8);
    data[8] = 0x08;
    data[9] = 0X32;
    data[10] = 0X01;
    data[11] = 0XFF;
    data[12] = 0XFF;
    data[13] = 0X00;
    data[14] = 0X00;
    data[15] = 0X02;
    data[16] = 0X3A;
    ESP_LOG_BUFFER_HEX(TAG, data, 17); // 打印接收到的数据
    // uart_write_bytes(UART_NUM_2, data, 17);
    return 0;
}
