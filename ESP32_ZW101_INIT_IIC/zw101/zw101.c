#include "zw101.h"

#define MY_ZW101_PACK_DATA_HEADER (0xEF01)

#define MY_ZW101_PACK_FLAG_CMD (0x01)
#define MY_ZW101_PACK_FLAG_DATA (0x02)
#define MY_ZW101_PACK_FLAG_END (0x08)
#define MY_ZW101_PACK_FLAG_BACK (0x07)

// static const char *TAG = "zw101";

/**
 * 计算校验和
 */
static uint16_t my_zw101_checksum(uint8_t pack_flag, uint16_t pack_len, uint8_t *pack_data)
{
    uint16_t sum = 0x00;
    sum += pack_flag;
    sum += pack_len;
    for (uint16_t i = 0; i < pack_len - 2; i++)
    {
        sum += pack_data[i];
    }
    return sum;
}

/**
 * 生成包结构
 */
static uint8_t my_zw101_pack(my_zw101_node *node, uint8_t pack_flag, uint16_t pack_len)
{
    node->pack_buffer_len = 0;
    if (pack_len + 9 >= MY_ZW101_PACK_BUFF_LEN)
        return 1;
    if (pack_len <= 2)
        return 1;

    // 后移数据
    for (int16_t i = pack_len - 2; i > 0; i--)
    {
        node->pack_buffer[i - 1 + 9] = node->pack_buffer[i - 1];
    }
    // 包头
    node->pack_buffer[node->pack_buffer_len++] = MY_ZW101_PACK_DATA_HEADER >> 8;
    node->pack_buffer[node->pack_buffer_len++] = (uint8_t)MY_ZW101_PACK_DATA_HEADER;
    // 设备地址
    node->pack_buffer[node->pack_buffer_len++] = (node->address) >> 24;
    node->pack_buffer[node->pack_buffer_len++] = (node->address) >> 16;
    node->pack_buffer[node->pack_buffer_len++] = (node->address) >> 8;
    node->pack_buffer[node->pack_buffer_len++] = (node->address);
    // 包标识
    node->pack_buffer[node->pack_buffer_len++] = pack_flag;
    // 包长度
    node->pack_buffer[node->pack_buffer_len++] = pack_len >> 8;
    node->pack_buffer[node->pack_buffer_len++] = pack_len;
    // 计算校验和
    uint16_t sum = my_zw101_checksum(pack_flag, pack_len, &node->pack_buffer[node->pack_buffer_len]);
    // 数据长度
    node->pack_buffer_len += pack_len - 2;
    // 校验和
    node->pack_buffer[node->pack_buffer_len++] = sum >> 8;
    node->pack_buffer[node->pack_buffer_len++] = sum;
    // 发送
    node->div_p(node);

    return 0;
}

/**
 * 握手
 */
uint8_t my_zw101_get_echo(my_zw101_node *node)
{
    node->pack_buffer[0] = 0x53;
    return my_zw101_pack(node, MY_ZW101_PACK_FLAG_CMD, 0x0003);
}

/**
 * 校验传感器是否正常工作
 */
uint8_t my_zw101_ps_check_sensor(my_zw101_node *node)
{
    node->pack_buffer[0] = 0x36;
    return my_zw101_pack(node, MY_ZW101_PACK_FLAG_CMD, 0x0003);
}

/**
 * 休眠命令
 */
uint8_t my_zw101_ps_sleep(my_zw101_node *node)
{
    node->pack_buffer[0] = 0x33;
    return my_zw101_pack(node, MY_ZW101_PACK_FLAG_CMD, 0x0003);
}

/**
 * 自动验证指纹
 * @param level 分数等级 0 或者 1
 * @param id 为0xFFFF时为1:N搜索
 * @param data
 */
uint8_t my_zw101_ps_auto_identify(my_zw101_node *node, uint8_t level, uint16_t id, uint16_t data)
{
    node->pack_buffer[0] = 0x32;
    node->pack_buffer[1] = level;
    node->pack_buffer[2] = id >> 8;
    node->pack_buffer[3] = id;
    node->pack_buffer[4] = data >> 8;
    node->pack_buffer[5] = data;
    return my_zw101_pack(node, MY_ZW101_PACK_FLAG_CMD, 0x0008);
}

/**
 * 自动注册模板
 * @param id
 * @param entries 录入次数
 * @param param bit0：采图背光灯控制位，0-LED 长亮，1-LED 获取图像成功后灭; \n
 *              bit1：保留; \n
 *              bit2：注册过程中，要求模组返回关键步骤，0-要求返回，1-不要求返回; \n
 *              bit3：是否允许覆盖ID 号，0-不允许，1-允许; \n
 *              bit4：允许指纹重复注册控制位，0-允许，1-不允许; \n
 *              bit5：注册时，多次指纹采集过程中，是否要求手指离开才能进入下一次指纹图像采集， 0-要求离开；1-不要求离开; \n
 *              bit6~bit15：预留; \n
 * @return null
 */
uint8_t my_zw101_ps_auto_enroll(my_zw101_node *node, uint16_t id, uint8_t entries, uint16_t param)
{
    node->pack_buffer[0] = 0x31;
    node->pack_buffer[1] = id >> 8;
    node->pack_buffer[2] = id;
    node->pack_buffer[3] = entries;
    node->pack_buffer[4] = param >> 8;
    node->pack_buffer[5] = param;
    return my_zw101_pack(node, MY_ZW101_PACK_FLAG_CMD, 0x0008);
}

// #include "esp_log.h"
// static const char *TAG = "test answer";
/**
 * 应答包整体校验
 * @result uint8_t 0为成功
 */
uint8_t my_zw101_answer_check(my_zw101_node *node, uint16_t *head, uint16_t *pack_data_len)
{
    uint16_t status = 0;
    uint16_t len = 0;
    uint16_t sum = 0;
    for (uint16_t i = 0; i < node->pack_buffer_len; i++)
    {
        // ESP_LOGI(TAG, "start index = [%d], data = [0x%02x], status = [%d]", i, node->pack_buffer[i], status);
        switch (status)
        {
        case 0: // 标头1
            if (node->pack_buffer[i] == (uint8_t)(MY_ZW101_PACK_DATA_HEADER >> 8))
                status = 1;
            else
                status = 0;
            break;
        case 1: // 标头2
            if (node->pack_buffer[i] == (uint8_t)MY_ZW101_PACK_DATA_HEADER)
                status = 2;
            else
                status = 0;
            break;
        case 2: // 地址1
            if (node->pack_buffer[i] == (uint8_t)(node->address >> 24))
                status = 3;
            else
                status = 0;
            break;
        case 3: // 地址2
            if (node->pack_buffer[i] == (uint8_t)(node->address >> 16))
                status = 4;
            else
                status = 0;
            break;
        case 4: // 地址3
            if (node->pack_buffer[i] == (uint8_t)(node->address >> 8))
                status = 5;
            else
                status = 0;
            break;
        case 5: // 地址4
            if (node->pack_buffer[i] == (uint8_t)node->address)
                status = 6;
            else
                status = 0;
            break;
        case 6: // 包标识
            if (node->pack_buffer[i] == MY_ZW101_PACK_FLAG_BACK)
            {
                status = 7;
            }
            else
            {
                status = 0;
            }
            break;
        case 7: // 长度1
            status = 8;
            len <<= 8;
            len = node->pack_buffer[i];
            break;
        case 8: // 长度2
            status = 9;
            len <<= 8;
            len |= node->pack_buffer[i];
            *head = i + 1;
            *pack_data_len = len - 2;
            if (len < 3)
            {
                status = 0;
            }
            break;
        case 9: // 数据处理
            sum = my_zw101_checksum(node->pack_buffer[i - 3], len, &node->pack_buffer[i]);
            if (sum != (((uint16_t)node->pack_buffer[i + len - 2] << 8) | node->pack_buffer[i + len - 1]))
                return 3; // 校验值错误
            return 0;     // 成功
            break;

        default:
            return 2; // 越界
            break;
        }
        // ESP_LOGI(TAG, "end index = [%d], data = [0x%02x], status = [%d]", i, node->pack_buffer[i], status);
        // ESP_LOGI(TAG, "-------------------------------------------------------");
    }
    return 1; // 未找到
}
