#include <driver/i2c_master.h>
#include "pn532_i2c.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define ACK_LENGTH 7
#define RES_LENGTH 16
#define BUF_LENGTH 64

static const uint8_t ACK_PACKET[ACK_LENGTH] = {0x01, 0x00, 0x00, 0xff, 0x00, 0xff, 0x00};

/**
 * @brief  Generate PN532 data package.
 */
int PN532_Package(uint8_t *in_buf, int in_len, uint8_t *out_buf)
{
    uint8_t checksum = PN532_HOST_TO_PN532;
    out_buf[0] = PN532_PREAMBLE;
    out_buf[1] = PN532_STARTCODE1;
    out_buf[2] = PN532_STARTCODE2;
    out_buf[3] = in_len + 1;
    out_buf[4] = 0x100 - out_buf[3];
    out_buf[5] = PN532_HOST_TO_PN532;

    for (int i = 0; i < in_len; i++)
    {
        out_buf[i + 6] = in_buf[i];
        checksum += in_buf[i];
    }
    out_buf[in_len + 6] = 0x100 - checksum;
    out_buf[in_len + 7] = 0x00;

    return in_len + 8;
}

/**
 * @brief  Parse PN532 data.
 */
int PN532_Parse(uint8_t *buf, int length, uint8_t *out_buf)
{
    if (length < 7 || buf[0] != 0x00 || buf[1] != 0x00 || buf[2] != 0xff ||
        buf[3] != (0x100 - buf[4]) || buf[3] > length - 7 || buf[5] != PN532_PN532_TO_HOST ||
        buf[buf[3] + 6] != 0x00)
    {
        return -1;
    }

    for (int i = 0; i < buf[3]; i++)
    {
        out_buf[i] = buf[6 + i];
    }
    return buf[3];
}

/**
 * @brief  Initialize PN532 GPIOs.
 */
void PN532_GPIO_Init(void)
{
    gpio_config_t irq_gpio_config = {
        .pin_bit_mask = (1ULL << PN532_IRQ_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&irq_gpio_config);

    gpio_config_t reset_gpio_config = {
        .pin_bit_mask = (1ULL << PN532_RESET_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&reset_gpio_config);
}

/**
 * @brief  Reset PN532.
 */
void PN532_Reset(void)
{
    gpio_set_level(PN532_RESET_PIN, 1);
    gpio_set_level(PN532_RESET_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(40));
    gpio_set_level(PN532_RESET_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
}

/**
 * @brief  Wait until I2C bus is ready.
 */
PN532_RES PN532_WaitReady(int timeout)
{
    while (gpio_get_level(PN532_IRQ_PIN))
    {
        vTaskDelay(pdMS_TO_TICKS(1));
        if (timeout == -1)
            continue;
        if (--timeout == 0)
            return PN532_TIMEOUT;
    }
    return PN532_OK;
}

/**
 * @brief  Write to PN532 and wait for ACK.
 */
PN532_RES PN532_Write_WaitAck(uint8_t *pbuf, uint16_t len, uint32_t timeout)
{
    uint8_t res[ACK_LENGTH];
    ESP_ERROR_CHECK(i2c_master_transmit(pn532_handle, pbuf, len, 100));

    if (PN532_WaitReady(timeout) != PN532_OK)
        return PN532_TIMEOUT;

    ESP_ERROR_CHECK(i2c_master_receive(pn532_handle, res, ACK_LENGTH, 100));

    return memcmp(ACK_PACKET, res, ACK_LENGTH) == 0 ? PN532_OK : PN532_DATA_ERROR;
}

/**
 * @brief  Get response from PN532.
 */
PN532_RES PN532_Response(uint8_t *pbuf, uint16_t len, int timeout)
{
    if (PN532_WaitReady(timeout) != PN532_OK)
        return PN532_TIMEOUT;
    ESP_ERROR_CHECK(i2c_master_receive(pn532_handle, pbuf, len, 100));
    return PN532_OK;
}

/**
 * @brief  Get PN532 version.
 */
PN532_RES PN532_Get_Version(uint8_t *ver)
{
    uint8_t pbuf[] = {CMD_GET_FW_VERSION};
    uint8_t pack[RES_LENGTH];
    uint8_t res[RES_LENGTH];

    int ret = PN532_Package(pbuf, 1, pack);
    if (PN532_Write_WaitAck(pack, ret, 200) != PN532_OK)
        return PN532_TIMEOUT;
    if (PN532_Response(res, RES_LENGTH, 100) != PN532_OK)
        return PN532_TIMEOUT;

    ver[0] = res[9];
    ver[1] = res[10];
    return PN532_OK;
}

/**
 * @brief  Configure PN532 SAM.
 */
PN532_RES PN532_SAMConfig(void)
{
    uint8_t pbuf[] = {CMD_SAM_CONFIGURATION, FIND_NFCCARD_MAXNUM, NFC_106K_PROTOCOL};
    uint8_t pack[RES_LENGTH];
    uint8_t res[RES_LENGTH];

    int ret = PN532_Package(pbuf, 3, pack);
    if (PN532_Write_WaitAck(pack, ret, 200) != PN532_OK)
        return PN532_TIMEOUT;
    if (PN532_Response(res, RES_LENGTH, 100) != PN532_OK)
        return PN532_TIMEOUT;

    return (res[7] == 0x15) ? PN532_OK : PN532_DATA_ERROR;
}
/**
 * @brief  Read passive target ID.
 */
PN532_RES PN532_ReadPassiveTargetID(uint8_t *card_id, uint8_t *id_len)
{
    uint8_t buf[] = {CMD_IN_LIST_PASSIVE_TARGET, FIND_NFCCARD_MAXNUM, NFC_106K_PROTOCOL};
    uint8_t pack[BUF_LENGTH];
    uint8_t res[BUF_LENGTH];
    uint8_t pdata[RES_LENGTH];

    int ret = PN532_Package(buf, 3, pack);
    if (PN532_Write_WaitAck(pack, ret, 1000) != PN532_OK)
        return PN532_TIMEOUT;
    if (PN532_Response(res, 23, -1) != PN532_OK)
        return PN532_TIMEOUT;

    if (res[0] == 0x01)
    {
        ret = PN532_Parse(&res[1], 22, pdata);
        if (ret > 0)
        {
            *id_len = pdata[6];
            memcpy(card_id, &pdata[7], *id_len);
            return PN532_OK;
        }
    }
    return PN532_TIMEOUT;
}