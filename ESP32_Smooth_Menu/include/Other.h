#define uS_TO_S_FACTOR 1000000ULL // 微秒到秒的转换系数
void esp_sleep(uint32_t minutes)  // 系统休眠
{
  Serial.println("休眠");
  //esp_sleep_config_gpio_isolate(); // 配置以隔离处于睡眠状态的所有GPIO引脚
  gpio_deep_sleep_hold_dis();      // 在深度睡眠期间禁用所有数字gpio pad hold功能。
  if (minutes != 0)
  {
     // esp_sleep_enable_timer_wakeup(minutes * uS_TO_S_FACTOR);
    esp_sleep_enable_timer_wakeup(minutes * 1000ULL);
    // esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
  }
  esp_deep_sleep_start();
}

uint8_t print_wakeup_reason()
{
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause(); // 获取休眠唤醒原因
  switch (wakeup_reason)
  {
  case ESP_SLEEP_WAKEUP_EXT0:
    Serial.println("使用RTC_IO由外部信号引起的唤醒");
    return 1;
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    Serial.println("使用RTC_CNTL由外部信号引起的唤醒");
    return 2;
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("计时器引起的唤醒");
    return 3;
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    Serial.println("触摸板引起的唤醒");
    return 4;
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    Serial.println("ULP程序导致的唤醒");
    return 5;
    break;
  default:
    Serial.printf("唤醒不是由深度睡眠引起的: %d\n", wakeup_reason);
    return 0;
    break;
  }
}

String getEspChipId()
{
  uint32_t chipId = 0;
  for (int i = 0; i < 17; i = i + 8)
  {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  return String(chipId);
}

// void i2c_init() // i2c初始化
// {
//   Wire.end();
//   Wire.begin(I2C_SDA, I2C_SCL);
// }

bool LittleFSBegin()
{
  // 文件系统初始化
  bool fsOK = LittleFS.begin(1);
  if (!fsOK)
  {
    u8g2.clearBuffer();
    u8g2.setCursor(0, 14);
    u8g2.print("LittleFS启动失败");
    u8g2.sendBuffer();
  }
  else Serial.println("LittleFS启动成功");
  return fsOK;
}