//********* 时钟校准程序 *********
void setupClockCalibration()
{
    u8g2.clearBuffer();

    //****** 检查时钟芯片是否存在
    if (!clockChipExist())
    {
        u8g2.setCursor(0, 14);
        u8g2.print("时钟芯片不存在");
        u8g2.sendBuffer();
        goto tuichu;
    }
    u8g2.setCursor(0, 14);
    u8g2.print("时钟芯片存在");
    u8g2.sendBuffer();

    //****** 连接WIFI
    u8g2.setCursor(0, 28);
    u8g2.print("连接:" + eep_sta_ssid);
    u8g2.sendBuffer();
    if (!connectToWifi())
    {
        // wifi连接失败
        u8g2.setCursor(100, 28);
        u8g2.print("失败");
        u8g2.sendBuffer();
        goto tuichu;
    }
    u8g2.setCursor(100, 28);
    u8g2.print("成功");
    u8g2.sendBuffer();

    //****** 连接NTP服务器
    u8g2.setCursor(0, 42);
    u8g2.print("NTP服务器连接中");
    u8g2.sendBuffer();
    if (!ntpConnect()) // 连接NTP服务器
    {
        u8g2.setCursor(0, 42);
        u8g2.print("NTP服务器连接失败");
        u8g2.sendBuffer();
        goto tuichu;
    }
    u8g2.setCursor(0, 42);
    u8g2.print("NTP服务器连接成功");
    u8g2.sendBuffer();

    //****** 数据写入时钟芯片
    if (!ntpToClockChip())
    {
        u8g2.setCursor(0, 58);
        u8g2.print("写入时钟芯片失败");
        u8g2.sendBuffer();
        goto tuichu; 
    }
    u8g2.setCursor(0, 58);
    u8g2.print("写入时钟芯片成功");
    u8g2.sendBuffer();

tuichu:
    delay(2000);
    return;
}