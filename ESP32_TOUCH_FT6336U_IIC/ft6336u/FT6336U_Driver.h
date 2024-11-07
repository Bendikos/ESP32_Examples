#ifndef __FT6336U_DRIVER_H_
#define __FT6336U_DRIVER_H_

#include <stdint.h>
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define I2C_MASTER_SCL_IO 22      /*!< GPIO for I2C master clock */
#define I2C_MASTER_SDA_IO 21      /*!< GPIO for I2C master data */
#define I2C_MASTER_NUM I2C_NUM_0  /*!< I2C port number */
#define I2C_MASTER_FREQ_HZ 100000 /*!< I2C master clock frequency */

extern i2c_master_bus_handle_t bus_handle;
extern i2c_master_dev_handle_t touch_handle;

#define FT6336U_RST_IO 18       // 复位低电平有效
#define FT6336U_INT_IO 19       // 中断低电平有效
#define FT6336U_Slave_Addr 0x38 // I2C从机地址

// 芯片寄存器
#define TD_STATUS 0x02
#define P1_XH 0x03
#define P1_XL 0x04
#define P1_YH 0x05
#define P1_YL 0x06
#define P2_XH 0x09
#define P2_XL 0x0A
#define P2_YH 0x0B
#define P2_YL 0x0C

#define ID_G_CIPHER_HIGH 0xA3
#define ID_G_CIPHER_MIDE 0x9F
#define ID_G_CIPHER_LOW 0xA0
#define ID_G_LIB_VERSION_H 0xA1
#define ID_G_LIB_VERSION_L 0xA2
#define ID_G_FIRMID 0xA6
#define ID_G_FOCALTECH_ID 0xA8
#define ID_G_PMODE 0xA5

typedef struct
{
    uint32_t CPIPHER;
    uint16_t LIB_VERSION;
    uint8_t FIRMWARE_VERSION;
    uint8_t VENDOR_ID;
} FT6336U_IC_INFO;

typedef struct
{
    uint8_t touch_num;
    uint16_t touch0_x;
    uint16_t touch0_y;
    uint16_t touch1_x;
    uint16_t touch1_y;
} FT6336U_TOUCH_POS;

void ft6336u_init();
void ft6336u_reset();
void ft6336u_read_ic_info(FT6336U_IC_INFO *info);
void ft6336u_read_touch_pos(FT6336U_TOUCH_POS *touch_pos);
uint8_t ft6336u_read_power_mode();

#endif // __FT6336U_DRIVER_H_
