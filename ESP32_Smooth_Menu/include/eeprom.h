void eeprom_init()
{
  eep.begin("eepUser", false);

  bool eep_init = eep.getBool("eep_init", 0); // 获取初始化状态 1-已初始化 0-未初始化

  if (eep_init == 0)
  {
    Serial.println("eepromPut");
    String xreep = F("正在写入EEPROM");
    Serial.println(xreep);
    eep.clear(); // 清除空间

    // 写入数据 注意：命名空间名称限制为15个字符。
    eep.putString("sta_ssid", eep_sta_ssid);         // wifi名称
    eep.putString("sta_password", eep_sta_password); // wifi密码
    eep.putFloat("timeZone", eep_timeZone);          // 时区
    eep.putString("ntpAdd", eep_ntpAdd);             // 自定义ntp地址
    eep.putUInt("clockLWJG", eep_clockLWJG);         // 时钟联网校准间隔，0为不联网校准 ，单位小时

    eep.putUInt("speed", eep_speed);            // 电机速度
    eep.putUInt("acc", eep_acc);                // 电机加速度
    eep.putULong("startPoint", eep_startPoint); // 起点位置
    eep.putULong("endPoint", eep_endPoint);     // 终点位置
    eep.putBool("motoDir", eep_motoDir);        // 电机方向

    eep.putUInt("oledContrast", eep_oledContrast);   // Oled亮度
    eep.putUInt("oledScreenOff", eep_oledScreenOff); // 息屏时间,分钟

    eep_init = 1; // 初始化成功
    eep.putBool("eep_init", eep_init);
    ESP.restart();
  }
  else
  {
    Serial.println("eepromGet");

    eep_sta_ssid = eep.getString("sta_ssid", "");         // wifi名称
    eep_sta_password = eep.getString("sta_password", ""); // wifi密码
    eep_timeZone = eep.getFloat("timeZone", 8.0);         // 时区
    eep_ntpAdd = eep.getString("ntpAdd", "");             // 自定义ntp地址
    eep_clockLWJG = eep.getUInt("clockLWJG", 0);          // 时钟联网校准间隔，0为不联网校准 ，单位小时

    eep_speed = eep.getUInt("speed", 5000);            // 电机速度
    eep_acc = eep.getUInt("acc", 5000);                // 电机加速度
    eep_startPoint = eep.getULong("startPoint", 1000); // 起点位置
    eep_endPoint = eep.getULong("endPoint", 90000);    // 终点位置
    eep_motoDir = eep.getBool("motoDir", 0);           // 电机方向

    eep_oledContrast = eep.getUInt("oledContrast", 50);  // Oled亮度
    eep_oledScreenOff = eep.getUInt("oledScreenOff", 3); // 息屏时间,分钟
    eep.end();
  }
}

// 保存电机参数和oled参数，数值一样不会重复刷写，不必担心寿命
void putAllEeeprom()
{
  // 保存eeprom
  eep.begin("eepUser", false);

  eep.putUInt("speed", eep_speed);            // 电机速度
  eep.putUInt("acc", eep_acc);                // 电机加速度
  eep.putULong("startPoint", eep_startPoint); // 起点位置
  eep.putULong("endPoint", eep_endPoint);     // 终点位置
  eep.putBool("motoDir", eep_motoDir);        // 电机方向

  eep.putUInt("oledContrast", eep_oledContrast);   // Oled亮度
  eep.putUInt("oledScreenOff", eep_oledScreenOff); // 息屏时间,分钟

  eep.end();
  // 设置参数到电机
  stepper->setDirectionPin(dirPinStepper, eep_motoDir); // 设置方向引脚,本项目0为远离原点
  stepper->setSpeedInHz(eep_speed);                     // 设置速度输入单位 steps/s
  stepper->setAcceleration(eep_acc);                    // 设置加速度 100 steps/s²
  u8g2.setContrast(eep_oledContrast);                   // 设置亮度
}