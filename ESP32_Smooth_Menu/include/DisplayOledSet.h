//lbpy-列表偏移量 ， optMax-列表最大的数量
//设置菜单坐标初始化
void display_OledSet_Init(int8_t lbpy, uint8_t optMax)
{
    ui.loop = 1; // 开启动画
    ui.xkwz = 0; // 位置归零
    // 字符的初始位置
    for (uint8_t i = 0; i < optMax; i++)
    {
        strNew_x[i] = 1;
        strNew_y[i] = 13;
        strTar_x[i] = 1;
        strTar_y[i] = 13 + (i - lbpy) * 16;
    }
    // 选框的初始位置
    String name = F("[ OLED设置 ]"); // 默认选框的名称
    uint8_t seek = lbpy + 0;         // 偏移量
    xkwzTar.x = 0;
    xkwzTar.y = 13 - 13;
    xkwzTar.w = u8g2.getUTF8Width(name.c_str()) + 2; // 计算像素长度
    xkwzTar.h = 15;
    // 设置进度条初始位置
    if (jduNew_y >= 126)     jduNew_y = 0;
    else if (jduNew_y == 0)  jduNew_y = 126;
}
//设置菜单 
void display_OledSet(int8_t lbpy,uint8_t optMax)
{
   // 创建名称、选项
    String setOpt[optMax] = {F("[ OLED设置 ]"),
                             F("亮度："),
                             F("息屏时间："),
                             F("纵然一夜风吹去，只在芦花浅水边")};

    // 拼装数值和显示
    for (uint8_t i = 0; i < optMax; i++)
    {
        //常规数值拼装
        if (i == 1)      setOpt[i] += String(eep_oledContrast);
        else if (i == 2) setOpt[i] += String(eep_oledScreenOff) + F("分钟");
        // 显示
        u8g2.setCursor(strNew_x[i], strNew_y[i]);
        u8g2.print(setOpt[i]); 
    }

    // 清除进度条内的字符
    u8g2.setDrawColor(0);
    u8g2.drawBox(u8g2.getWidth() - 6, 0, 6, 127);

    // 画反色选框
    u8g2.setDrawColor(2); // 0透显，1实显，2反色
    u8g2.drawRBox(xkwzNew.x, xkwzNew.y, xkwzNew.w, xkwzNew.h, 0.5);
    u8g2.setDrawColor(1);

    // 绘制进度条
    int8_t pos = ui.xkwz + lbpy; // 选项实际位置
    jduTar_y = map(pos, 0, optMax - 1, 1, 63);
    u8g2.drawHLine(u8g2.getWidth() - 5, 0, 5);
    u8g2.drawHLine(u8g2.getWidth() - 5, u8g2.getHeight() - 1, 5);
    u8g2.drawVLine(u8g2.getWidth() - ceil((float)5 / 2), 0, u8g2.getHeight());
    u8g2.drawBox(u8g2.getWidth() - 5, 0, 5, jduNew_y);

    // 动画处理
    if (ui.loop == 1) //动画参数设置
    {
        ui.loop = 2;     // 启动动画
        if (ui.xkwz > 3) // 选框到了列表底部
        {  
            ui.xkwz = 3;
            if (strTar_y[optMax - 1] != 13 + 3 * 16)
            {
                for (uint8_t i = 0; i < optMax; i++)
                {
                    // 将列表Y轴往上移动
                    strTar_y[i] -= 16;
                }
            }
        }
        else if (ui.xkwz < 0) // 选框到了列表顶部
        {
            ui.xkwz = 0;
            if (strTar_y[0] != 13 + 0 * 16)
            {
                for (uint8_t i = 0; i < optMax; i++)
                {
                    // 将列表Y轴往下移动
                    strTar_y[i] += 16;
                }
            }
        }

        // 设置选框的目标值
        uint8_t pos = ui.xkwz + lbpy; // 选项实际位置
        xkwzTar.x = strTar_x[pos] - 1;
        xkwzTar.y = strTar_y[pos] - 13;
        xkwzTar.w = u8g2.getUTF8Width(setOpt[pos].c_str()) + 2;
        xkwzTar.h = 15;

        // 长字符移动用
        w_moveX = xkwzTar.w;
        if (xkwzTar.x < 0)   xkwzTar.x = 0;
        if (xkwzTar.w > 122) xkwzTar.w = 122;
    }

    // 播放动画
    if (ui.loop == 2)
    {
        for (uint8_t i = 0; i < optMax; i++)
        {
            // 名称的动画
            animation4(&strNew_x[i], &strTar_x[i], 100);
            animation1(&strNew_y[i], &strTar_y[i], 80);
        }
        // 选框的动画
        animation1(&xkwzNew.x, &xkwzTar.x, 80);
        animation1(&xkwzNew.y, &xkwzTar.y, 80);
        animation1(&xkwzNew.w, &xkwzTar.w, 80);
        animation1(&xkwzNew.h, &xkwzTar.h, 80);
        // 进度条的动画
        animation1(&jduNew_y, &jduTar_y, 80);
        // 字符太长开启滚动
        if (xkwzTar.w >= 122 && strTar_x[pos] == strNew_x[pos])
        {
            uint8_t pos = ui.xkwz + lbpy; // ui.xkwz + lbpy = 实际位置
            if (moveX_time == 0)
            {
                strTar_x[pos] = 1;
                if (strTar_x[pos] == strNew_x[pos]) moveX_time = millis();
            }
            else if (millis() - moveX_time > 1200) // 滞后1.6秒滚动
            {
                strTar_x[pos] = 80 - w_moveX;
                if (strTar_x[pos] == strNew_x[pos]) moveX_time = 0;
            }
        }
    }
}
//设置菜单按键逻辑
void display_OledSet_Key(int8_t *lbpy, uint8_t optMax)
{
    if(win.state) return; // 弹窗时不执行
    // ************ 选项切换  ************
    if (get_key(KEY_UP, 100, 1) == 1)
    {
        ui.loop = 1; //开启动画
        ui.xkwz--;
        //到达列表顶部
        if (ui.xkwz < 0) *lbpy -= 1; // 列表偏移量减1
        if (*lbpy < 0)   *lbpy = 0;  // 不允许负值
        Serial.print("ui.xkwz:"); Serial.println(ui.xkwz);
        Serial.print("*lbpy:"); Serial.println(*lbpy);
    }
    else if (get_key(KEY_DOWN, 100, 1) == 1)
    {
        ui.loop = 1;  //开启动画
        if(ui.xkwz < optMax - 1) ui.xkwz++;
        if (ui.xkwz > 3) // 到达列表底部
        {
            *lbpy += 1; // 列表偏移量加1
            // 限制最大值，因为一页显示4行，所以减4
            if (*lbpy > optMax - 4)  *lbpy = optMax - 4;
        }
        Serial.print("ui.xkwz:"); Serial.println(ui.xkwz);
        Serial.print("*lbpy:"); Serial.println(*lbpy);
    }

    //************ 选项确认（*lbpy + ui.xkwz 等于 实际位置） ************
    if (*lbpy + ui.xkwz <= 0 && get_key(KEY_OK, 100, 1) == 1)
    {
        DisplaySW(DISPLAY_SET); // 切换至设置界面
    }
    else if (*lbpy + ui.xkwz == 1 && get_key(KEY_OK, 500, 1) == 1)
    {
        // 亮度 弹窗
        popoverTransfer(F("亮度"), &eep_oledContrast, 5, 255, 1);
    }
    else if (*lbpy + ui.xkwz == 2 && get_key(KEY_OK, 500, 1) == 1)
    {
        //息屏时间 弹窗
        popoverTransfer(F("息屏时间"), &eep_oledScreenOff, 0, 30, 1);
    }
}