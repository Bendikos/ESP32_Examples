#ifndef __PN532_I2C_H__
#define __PN532_I2C_H__

#include "driver/i2c_master.h"
#include <stdint.h>
#include "esp_attr.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "driver/gpio.h"

// External I2C Handles
extern i2c_master_bus_handle_t bus_handle;
extern i2c_master_dev_handle_t pn532_handle;

// GPIO Configuration
#define I2C_MASTER_SCL_IO 22      /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 21      /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_0  /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 100000 /*!< I2C master clock frequency */
#define PN532_IRQ_PIN 32
#define PN532_RESET_PIN 33

// PN532 Frame Format
#define PN532_PREAMBLE 0x00
#define PN532_STARTCODE1 0x00
#define PN532_STARTCODE2 0xFF
#define PN532_HOST_TO_PN532 0xD4
#define PN532_PN532_TO_HOST 0xD5

static const char *TAG = "PN532";
// NFC Settings
#define FIND_NFCCARD_MAXNUM 0x01
#define NFC_106K_PROTOCOL 0x00
#define ACK_LENGTH 7
#define RES_LENGTH 16
#define BUF_LENGTH 64

#define ready_to_recive()                                     \
    do                                                        \
    {                                                         \
        len = PN532_Package(buf, 3, pack);                    \
        if (PN532_Write_WaitAck(pack, len, 1000) == PN532_OK) \
        {                                                     \
            ESP_LOGI(TAG, "ready to receive data");           \
        }                                                     \
    } while (0)

// PN532 Commands
enum
{
    CMD_DIAGNOSE = 0x00,
    CMD_GET_FW_VERSION = 0x02,
    CMD_GET_GENERAL_STATUS = 0x04,
    CMD_READ_REGISTER = 0x06,
    CMD_WRITE_REGISTER = 0x08,
    CMD_READ_GPIO = 0x0C,
    CMD_WRITE_GPIO = 0x0E,
    CMD_SET_SERIAL_BAUD_RATE = 0x10,
    CMD_SET_PARAMETERS = 0x12,
    CMD_SAM_CONFIGURATION = 0x14,
    CMD_POWER_DOWN = 0x16,
    CMD_RF_CONFIGURATION = 0x32,
    CMD_RF_REGULATION_TEST = 0x58,
    CMD_IN_LIST_PASSIVE_TARGET = 0x4A,
    CMD_IN_ATR = 0x50,
    CMD_IN_PSL = 0x4E,
    CMD_IN_DATA_EXCHANGE = 0x40,
    CMD_IN_COMMUNICATE_THRU = 0x42,
    CMD_IN_DESELECT = 0x44,
    CMD_IN_RELEASE = 0x52,
    CMD_IN_SELECT = 0x54,
    CMD_IN_AUTO_POLL = 0x60
};

// I2C Address
#define PN532_I2C_ADDRESS ((uint8_t)0x24)

// Error Codes
typedef enum
{
    PN532_OK = 0,
    PN532_TIMEOUT = -1,
    PN532_DATA_ERROR = -2
} PN532_RES;

void dev_pn532_initialization(void);
int PN532_Package(uint8_t *in_buf, int in_len, uint8_t *out_buf);
int PN532_Parse(uint8_t *buf, int length, uint8_t *out_buf);
// PN532 Function Prototypes
void PN532_GPIO_Init(void);
void PN532_Reset(void);
PN532_RES PN532_Get_Version(uint8_t *version);
PN532_RES PN532_SAMConfig(void);
// PN532_RES PN532_ReadPassiveTargetID(uint8_t *card_id, uint8_t *id_length);
PN532_RES PN532_Write_WaitAck(uint8_t *pbuf, uint16_t len, uint32_t timeout);
PN532_RES PN532_Response(uint8_t *pbuf, uint16_t len, int timeout);
#endif // __PN532_I2C_H__
