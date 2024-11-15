
#ifndef ZW101_H_
#define ZW101_H_

#include <stdio.h>
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"




static const char *TAG = "uart_events";

extern const uint8_t package_header[];

uint8_t ZW101_HandShake();
uint8_t ZW101_Cancel();
uint8_t ZW101_ReadIndexTable();
uint8_t ZW101_AutoEnroll(uint8_t number);
uint8_t ZW101_DeletChar(uint8_t number);
uint8_t ZW101_AutoIdentify();
#endif
