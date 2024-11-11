#ifndef MY_ZW101_H_
#define MY_ZW101_H_

#include <stdio.h>
#include "driver/uart.h"
#include "driver/gpio.h"

#define MY_ZW101_PACK_BUFF_LEN (256)

// #ifdef __cplusplus
// extern "C"
// {
// #endif

// ZW101 配置结构体
typedef struct my_zw101
{
    uint32_t address;
    uint8_t pack_buffer[MY_ZW101_PACK_BUFF_LEN];
    uint16_t pack_buffer_len;
    void (*div_p)(struct my_zw101 *);
} my_zw101_node;

// UART 配置结构体
typedef struct
{
    uart_port_t uart_num;
    int rate;
    int rx_pin;
    int tx_pin;
    int buffer;
} uart_node_t;

// GPIO 配置结构体
typedef struct
{
    gpio_num_t pin;
    int status;
    gpio_config_t config;
} gpio_pin_t;

// ZW101 设备结构体
typedef struct
{
    uint32_t address;
    int div_p;
} zw101_t;

uint8_t my_zw101_get_echo(my_zw101_node *node);
uint8_t my_zw101_ps_check_sensor(my_zw101_node *node);
uint8_t my_zw101_ps_sleep(my_zw101_node *node);
uint8_t my_zw101_ps_auto_identify(my_zw101_node *node, uint8_t level, uint16_t id, uint16_t data);
uint8_t my_zw101_ps_auto_enroll(my_zw101_node *node, uint16_t id, uint8_t entries, uint16_t param);

uint8_t my_zw101_answer_check(my_zw101_node *node, uint16_t *head, uint16_t *pack_data_len);

// #ifdef __cplusplus
// }
// #endif

#endif