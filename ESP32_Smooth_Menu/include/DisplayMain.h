//xxwz-选框位置 ， optMax-选项最大的数量
//主界面位置初始化
void displau_Main_Init(int8_t xxwz, uint8_t optMax)
{
     ui.loop = 1; // 开启动画
    // 字符的初始位置
    for (uint8_t i = 0; i < optMax; i++)
    {
        strNew_x[i] = 128;
        strNew_y[i] = 0;
    }
    // 选框的初始位置
    String name = F("回原点");
    uint8_t seek = xxwz + 2; // 偏移量，前两项为显示项所以+2
    xkwzNew.x = 50 - 1;
    xkwzNew.y = 37 - 13;
    xkwzNew.w = u8g2.getUTF8Width(name.c_str()) + 2;
    xkwzNew.h = 15;
    // 设置目标位置
    float strTar_x1[optMax] = {0, 0, 1, 85, 1, 35, 99};
    float strTar_y1[optMax] = {14, 30, 47, 47, 62, 62, 62};
    for (uint8_t i = 0; i < optMax; i++)
    {
        strTar_x[i] = strTar_x1[i];
        strTar_y[i] = strTar_y1[i];
    }
}
//主界面显示
void display_Main(int8_t xxwz,uint8_t optMax) 
{
    //u8g2.clearBuffer();
    String staName[optMax] = {F("当前位置:"), F("终点位置:"),
                              F("回原点"), F("去终点"),
                              F("前进"), F("后退"), F("设置")};

    staName[0] += String(stepper->getCurrentPosition());
    staName[1] += String(eep_endPoint);
    for (uint8_t i = 0; i < optMax; i++)
    {
        u8g2.setCursor(strNew_x[i], strNew_y[i]);
        u8g2.print(staName[i]);
    }
    // 画反色选框
    u8g2.setDrawColor(2); // 0透显，1实显，2反色
    u8g2.drawRBox(xkwzNew.x, xkwzNew.y, xkwzNew.w, xkwzNew.h, 0.5);
    u8g2.setDrawColor(1);

    // 开启动画初始化
    if (ui.loop == 1)
    {
        ui.loop = 2; //启动动画
        // 设置选框的目标值
        uint8_t seek = xxwz + 2; //偏移量，前两项为显示项所以+2
        xkwzTar.x = strTar_x[seek] - 1;
        xkwzTar.y = strTar_y[seek] - 13;
        xkwzTar.w = u8g2.getUTF8Width(staName[seek].c_str()) + 2;
        xkwzTar.h = 15;
    }
    // 播放动画
    if (ui.loop == 2)
    {
        // 计算新值
        for (uint8_t i = 0; i < optMax; i++)
        {
            //名称的动画
            animation1(&strNew_x[i], &strTar_x[i], 80);
            animation1(&strNew_y[i], &strTar_y[i], 80);
        }
        //选框的动画
        animation1(&xkwzNew.x, &xkwzTar.x, 80);
        animation1(&xkwzNew.y, &xkwzTar.y, 80);
        animation1(&xkwzNew.w, &xkwzTar.w, 80);
        animation1(&xkwzNew.h, &xkwzTar.h, 80);
    }

    //u8g2.sendBuffer();
    //fade1(fadeInterface);
}
//主界面的按键逻辑
void display_Main_Key(int8_t *xxwz, uint8_t optMax)
{ 
    if(win.state) return; // 弹窗时不执行
    //****** 选项切换
    if (get_key(KEY_UP, 150, 1) == 1)
    {
        ui.loop = 1; //开启动画
        *xxwz-=1;
        if (*xxwz < 0) *xxwz = 4;  
        Serial.print("*xxwz:"); Serial.println(*xxwz);
    }
    else if (get_key(KEY_DOWN, 150, 1) == 1)
    {
        ui.loop = 1;  //开启动画
        *xxwz+=1;
        if (*xxwz > 4) *xxwz = 0;
        Serial.print("*xxwz:"); Serial.println(*xxwz);
    }

    //****** 选项按OK
    if (*xxwz == 0 && get_key(KEY_OK) == 1) // 回到原点,长按
    {
        // 未执行原点程序 且 电机是停止状态
        if (returnToOrigin_key == 0 && stepper->isRunning() == 0)
        {
            if (digitalRead(SIG1) == 0) return; // 已经是原点
                
            returnToOrigin_key = 1;
            //防止失效，再次设置原点开关为输入上拉，撞击时拉低
            pinMode(SIG1, INPUT_PULLUP);
            delay(5); //等待稳定
            // 设置中断号、响应函数、触发方式 FALLING下降沿 RISING上升沿 CHANGE电平变化
            attachInterrupt(SIG1, return_to_origin, FALLING);
            stepper->move(-100000); // 移动
            //stepper->runBackward();
        }
        else if (returnToOrigin_key == 1) // 执行原点程序时
        {
            // 将处理步进命令队列，但不会添加其他命令。这意味着步进器可以在大约20ms内停止。
            stepper->stopMove(); // 以正常减速停止正在运行的步进器。
            returnToOrigin_key = 0;
        }
    }
    else if (*xxwz == 1 && get_key(KEY_OK) == 1) // 去终点
    {
        if (!stepper->isRunning() && digitalRead(SIG2) == 1)
            stepper->moveTo(eep_endPoint); // 移动到指定位置
        else if (stepper->isRunning())
            stepper->stopMove(); // 以正常减速停止正在运行的步进器。
    }
    else if (*xxwz == 2 && get_key(KEY_OK, 0, 1)) // 前进
    {
       if (digitalRead(SIG2) == 1) stepper->move(100); // 移动
    }
    else if (*xxwz == 3 && get_key(KEY_OK, 0, 1)) // 后退
    {
        if (digitalRead(SIG1) == 1)
        {
            // 设置中断号、响应函数、触发方式 FALLING下降沿 RISING上升沿 CHANGE电平变化
            attachInterrupt(SIG1, return_to_origin, FALLING);
            stepper->move(-100); // 移动
        }
    }
    else if (*xxwz == 4 && get_key(KEY_OK, 200, 1) == 1) // 进入设置界面
    {
        DisplaySW(DISPLAY_SET); //使用转场动画切换至设置界面
    }
}