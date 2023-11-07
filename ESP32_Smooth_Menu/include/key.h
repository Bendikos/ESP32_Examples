#define key_lb_max 30    // 按键滤波次数
#define ca_time_max 1000 // 长按识别时间

// 返回值：0-未按下 1-短按 2-长按
// timeMax: 按键时间滤波
// mod: 0-长短按模式 1-只有短按模式
uint64_t key_time=0;
int8_t get_key(uint8_t key_pin, uint16_t timeMax, bool mod)
{
  if (timeMax != 0)
  {
    if (millis() - key_time < timeMax) return -1;
  }

  uint32_t ca_time = 0;          // 长按的对比时间
  if (digitalRead(key_pin) == 0) // 按键按下为低电平
  {
    // 唤醒屏幕
    if (oled_sleepp_time == 0) u8g2.setPowerSave(0); //打开屏幕
    oled_sleepp_time = millis();                     // 休眠时间
    moveX_time = 0;                                  // 长字符延迟移动归零
    
    // 按键滤波
    uint8_t key_lb_count = 0; // 按键滤波计数
    boolean key_lb_state = 0; // 按键滤波状态 0-未稳定 1-稳定
    while (digitalRead(key_pin) == 0 && key_lb_state == 0)
    {
      key_lb_count++;
      if (key_lb_count >= key_lb_max) key_lb_state = 1;
    }

    if (mod == 1 && key_lb_state)
    {
      key_time = millis();
      return 1;
    }

    else if (key_lb_state) // 按键稳定
    {
      ca_time = millis();
      while (digitalRead(key_pin) == 0) // 长短按判断
      {
        // 持续一定时间判断长按
        if (millis() - ca_time > ca_time_max) return 2;
        delay(1);
      }
      // 持续时间未到规定判断短按
      return 1;
    }
    else return 0;//按键未稳定
      
  }
  else return 0;
}

//旋转编码器滤波 参数：key按键 ，lv_max滤波次数
//连续定值算法，连续lv_max次一样的值才判断按键有效
bool key_lb(uint8_t key, uint16_t lb_max)
{
  uint8_t count = 0;
  bool state; //当前值
  bool state_old = digitalRead(key);//旧值
  while (count < lb_max) //连续多少次
  {
    state = digitalRead(key);
    if (digitalRead(key) == state_old) //值一样，累积
      count++;
    else //值不一样，重新开始
    {
      state_old = state;
      count = 0;
    }
  }
  return state;
}