//****** 消失函数（重要，增加页面需要添加新的初始化函数）
void fade(uint8_t page)
{
    //当前的界面和转场动画界面一致时 不执行
    if(displayInterface == page) return;
    if(win.state != 0) return;
    while(displayInterface != page)
    {
        u8g2.sendBuffer();
        delay(30);
        switch (ui.fade)
        {
            case 1: for (uint16_t i = 0; i < buf_len; ++i)  if (i % 2 != 0) buf_ptr[i] = buf_ptr[i] & 0xAA; break;
            case 2: for (uint16_t i = 0; i < buf_len; ++i)  if (i % 2 != 0) buf_ptr[i] = buf_ptr[i] & 0x00; break;
            case 3: for (uint16_t i = 0; i < buf_len; ++i)  if (i % 2 == 0) buf_ptr[i] = buf_ptr[i] & 0x55; break;
            case 4: for (uint16_t i = 0; i < buf_len; ++i)  if (i % 2 == 0) buf_ptr[i] = buf_ptr[i] & 0x00; break;
            default:
                displayInterface = page; // 动画播放完毕跳转值相应的界面
                ui.fade = 0;

                //****** 各界面的初始化、新增页面需要加入对应的初始化函数 ******
                if (displayInterface == DISPLAY_MAIN)
                    displau_Main_Init(display_Main_xxwz, optMax_main);
                else if (displayInterface == DISPLAY_SET)
                    display_Set_Init(display_Set_xxwz, optMax_set);
                else if (displayInterface == DISPLAY_MOTO_SET)
                    display_MotoSet_Init(display_MotoSet_xxwz, optMax_motoSet);
                else if (displayInterface == DISPLAY_OLED_SET)
                    display_OledSet_Init(display_OledSet_xxwz, optMax_oledSet);
                else if (displayInterface == DISPLAY_PW)
                    display_PeiWang_Init(display_PeiWang_xxwz, optMax_peiWang);
                else if (displayInterface == DISPLAY_ABOUT)
                    display_About_Init(display_About_xxwz, optMax_about);

                break;
        }
        ui.fade++;
    }
}

//计算字符居中的X位置
uint16_t getCenterX(String z) 
{
  uint16_t zf_width = u8g2.getUTF8Width(z.c_str());  //获取字符的像素长度
  uint16_t x = (u8g2.getWidth() / 2) - (zf_width / 2);    //计算字符居中的X位置
  return x;
}
//计算字符居中的X位置
uint16_t getCenterX(const char* z) 
{
  uint16_t zf_width = u8g2.getUTF8Width(z);        //获取字符的像素长度
  uint16_t x = (u8g2.getWidth() / 2) - (zf_width / 2);  //计算字符居中的X位置
  return x;
}
//计算字符居中的X位置
uint16_t getCenterX(int32_t z) 
{
  return getCenterX(String(z));
}

//************ 坐标移动动画 ************
//初始全程平滑
void animation(float *a, float *a_trg, uint16_t n)
{
  if(*a == *a_trg) return;
  if (fabs(*a - *a_trg) < 0.15)
    *a = *a_trg;
  if (*a != *a_trg)
    *a += (*a_trg - *a) / (n / 10.0);
}
//缓慢结束
void animation1(float *a, float *a_trg, uint16_t n)
{
    if (*a == *a_trg) return;
        
    float cz = fabs(*a - *a_trg);

    if (cz <= 1) *a = *a_trg;
    else
    {
        if (cz < 10) n = n * cz * 0.1;
        if(n < 10) n = 10;
        *a += (*a_trg - *a) / (n * 0.1);  
    }
}
//缓慢进入
void animation2(float *a, float *a_trg, uint16_t n)
{ 
    if (*a == *a_trg) return;
       
    float cz = fabs(*a - *a_trg);
    if (cz <= 1) *a = *a_trg;
    else if (cz > 20) n = n * 3;
    else if (cz > 15) n = n * 2;
    else if (cz > 5) n = n * 1;
    if (*a != *a_trg) *a += (*a_trg - *a) / (n * 0.1);
}
//缓慢进入有BUG
void animation3(float *a, float *a_trg, uint16_t n)
{ 
    if (*a == *a_trg) return;
       
    float cz = fabs(*a - *a_trg);
    if (cz <= 1) *a = *a_trg;
    else n = n * cz * 0.05;
    if(n < 10) n = 10;
    if (*a != *a_trg) *a += (*a_trg - *a) / (n * 0.1);
}
//线性
void animation4(float *a, float *a_trg, uint16_t n)
{ 
    if (*a == *a_trg) return;
    
    float cz = fabs(*a - *a_trg);
    if (cz <= 1) *a = *a_trg;
    if (*a != *a_trg)
    {
        if (*a_trg >= 0) *a += n * 0.01;
        else            *a -= n * 0.01;
    }
    //Serial.print("*a_trg:");Serial.println(*a_trg);
    //Serial.print("cz:");Serial.println(cz);
}

//************ 弹窗动画 ************
//创建数据类型推导模版
enum NumTypeEnum {
    NUM_U8=0,
    NUM_U16,
    NUM_U32,
    NUM_8,
    NUM_16,
    NUM_32,
    NUM_F,
};
NumTypeEnum num_type; //数据类型
template <typename T>
//推导数据类型
void popoverTransfer(String name, T* num, int16_t num_min, int32_t num_max, int16_t step)
{
    if      (std::is_same<T, uint8_t>::value)  num_type = NUM_U8;
    else if (std::is_same<T, uint16_t>::value) num_type = NUM_U16;
    else if (std::is_same<T, uint32_t>::value) num_type = NUM_U32;
    else if (std::is_same<T, int8_t>::value)   num_type = NUM_8;
    else if (std::is_same<T, int16_t>::value)  num_type = NUM_16;  
    else if (std::is_same<T, int32_t>::value)  num_type = NUM_32;            
    else if (std::is_same<T, float>::value)    num_type = NUM_F;
    else
    {
        // 未知数据类型，可以在这里做一些错误处理或者默认值处理
        return;
    }
    popoverTransfer(name, num, num_type, num_min, num_max, step);
}
//弹窗参数传递
void popoverTransfer(String name, void* num, uint8_t num_type, int16_t num_min, int32_t num_max, int16_t step)
{
    switch (num_type) 
    {
        case NUM_U8:  win.num_u8  = static_cast<uint8_t*>(num);  break;
        case NUM_U16: win.num_u16 = static_cast<uint16_t*>(num); break;
        case NUM_U32: win.num_u32 = static_cast<uint32_t*>(num); break;        
        case NUM_8:   win.num_8   = static_cast<int8_t*>(num);   break;
        case NUM_16:  win.num_16  = static_cast<int16_t*>(num);  break; 
        case NUM_32:  win.num_32  = static_cast<int32_t*>(num);  break;
        case NUM_F:   win.num_f   = static_cast<float*>(num);    break; 
        default:
            // 未知数据类型，可以在这里做一些错误处理或者默认值处理
            break;
    }
    popoverInit(); 
    win.state = 1;         // 启动弹窗动画
    win.title = name;      // 标题
    win.num_min = num_min; // 最小值
    win.num_max = num_max; // 最大值
    win.num_step = step;   // 步进值
}
//弹窗函数初始化
void popoverInit()
{
    win.xNew = 11; win.yNew = 28; win.wNew = 107; win.hNew = 6;
    win.xTar = 11; win.yTar = 5;  win.wTar = 107; win.hTar = 45;

    win.jdtwk_xNew = 11+3; win.jdtwk_yNew = win.yTar+34; win.jdtwk_wNew = 3; win.jdtwk_hNew = 7;
    win.jdtwk_xTar = 11+3; win.jdtwk_yTar = win.yTar+34; win.jdtwk_wTar = 101; win.jdtwk_hTar = 7;

    win.jdt_xNew = win.jdtwk_xNew + 2; win.jdt_yNew = win.jdtwk_yNew + 2; win.jdt_wNew = 1; win.jdt_hNew = 3;
    win.jdt_xTar = win.jdtwk_xNew + 2; win.jdt_yTar = win.jdtwk_yTar + 2; win.jdt_wTar = 10; win.jdt_hTar = 3;
}
//弹窗函数
void popover()
{
    if (win.state == 1)  // 动画开始
    {
        // 选框的动画
        animation2(&win.yNew, &win.yTar, 30);
        animation2(&win.hNew, &win.hTar, 40);
        if (win.hNew > 39)
        {
            animation1(&win.jdtwk_wNew, &win.jdtwk_wTar, 60);
            animation1(&win.jdt_wNew, &win.jdt_wTar, 100);
        }
    }
    else if (win.state == -1) // 动画退出
    {
        animation(&win.yNew, &win.yTar, 120);
        win.jdtwk_yNew = win.yNew + 34;
        win.jdt_yNew = win.jdtwk_yNew + 2;
        //animation(&win.jdtwk_yNew, &win.jdtwk_yTar, 160);
        //animation(&win.jdt_yNew, &win.jdtwk_yTar, 160);
        if (win.yNew < -43) win.state = 0;
    }

    uint16_t num_y0 = win.yNew + 15; 
    uint16_t num_y1 = win.yNew + 30;
    
    u8g2.setDrawColor(1); // 0透显，1实显，2反色
    u8g2.drawBox(win.xNew, win.yNew, win.wNew, win.hNew); // 绘制外框背景
    u8g2.setDrawColor(2); // 0透显，1实显，2反色
    u8g2.drawBox(win.xNew, win.yNew, win.wNew, win.hNew); // 绘制外框背景
    u8g2.setDrawColor(1);
    u8g2.drawRFrame(win.xNew, win.yNew, win.wNew, win.hNew, 3); // 绘制外框描边

    // 绘制标题
    if (win.hNew > 15) 
        u8g2.drawUTF8(getCenterX(win.title), num_y0, win.title.c_str());
     // 绘制数值
    if (win.hNew > 30) 
    {  
        switch (num_type)
        {
            case 0: u8g2.setCursor(getCenterX(*win.num_u8),  num_y1);  u8g2.print(*win.num_u8);   break;
            case 1: u8g2.setCursor(getCenterX(*win.num_u16), num_y1);  u8g2.print(*win.num_u16);  break;
            case 2: u8g2.setCursor(getCenterX(*win.num_u32), num_y1);  u8g2.print(*win.num_u32);  break;
            case 3: u8g2.setCursor(getCenterX(*win.num_8),   num_y1);  u8g2.print(*win.num_8);    break;
            case 4: u8g2.setCursor(getCenterX(*win.num_16),  num_y1);  u8g2.print(*win.num_16);   break;
            case 5: u8g2.setCursor(getCenterX(*win.num_32),  num_y1);  u8g2.print(*win.num_32);   break;
            case 6: u8g2.setCursor(getCenterX(*win.num_f),   num_y1);  u8g2.print(*win.num_f);    break;
        }
    }
    // 绘制进度条
    if (win.hNew > 39)
    {
        switch (num_type) //计算进度条的长度
        {
            case 0: win.jdt_wTar = map(*win.num_u8,  win.num_min, win.num_max, 3, win.jdtwk_wNew - 4); break;
            case 1: win.jdt_wTar = map(*win.num_u16, win.num_min, win.num_max, 3, win.jdtwk_wNew - 4); break;
            case 2: win.jdt_wTar = map(*win.num_u32, win.num_min, win.num_max, 3, win.jdtwk_wNew - 4); break;
            case 3: win.jdt_wTar = map(*win.num_8,   win.num_min, win.num_max, 3, win.jdtwk_wNew - 4); break;
            case 4: win.jdt_wTar = map(*win.num_16,  win.num_min, win.num_max, 3, win.jdtwk_wNew - 4); break;
            case 5: win.jdt_wTar = map(*win.num_32,  win.num_min, win.num_max, 3, win.jdtwk_wNew - 4); break;
            case 6: win.jdt_wTar = map(*win.num_f,   win.num_min, win.num_max, 3, win.jdtwk_wNew - 4); break;
        }
        u8g2.drawRFrame(win.jdtwk_xNew, win.jdtwk_yNew, win.jdtwk_wNew, win.jdtwk_hNew, 1); // 绘制进度条外框
        u8g2.drawRBox(win.jdt_xNew, win.jdt_yNew, win.jdt_wNew, win.jdt_hNew, 1);           // 绘制进度条
    }
}

// 弹窗按键逻辑
void popover_Key()
{
    if (get_key(KEY_DOWN, 50, 1) == 1)
    {
        key_incrementalStep(); // 启动按键渐加步进值
        switch (num_type)
        {
            case 0: *win.num_u8  += win.num_step; if(*win.num_u8  > win.num_max)  *win.num_u8  = win.num_max; break;
            case 1: *win.num_u16 += win.num_step; if(*win.num_u16 > win.num_max)  *win.num_u16 = win.num_max; break;
            case 2: *win.num_u32 += win.num_step; if(*win.num_u32 > win.num_max)  *win.num_u32 = win.num_max; break;
            case 3: *win.num_8   += win.num_step; if(*win.num_8   > win.num_max)  *win.num_8   = win.num_max; break;
            case 4: *win.num_16  += win.num_step; if(*win.num_16  > win.num_max)  *win.num_16  = win.num_max; break;
            case 5: *win.num_32  += win.num_step; if(*win.num_32  > win.num_max)  *win.num_32  = win.num_max; break;
            case 6: *win.num_f   += win.num_step; if(*win.num_f   > win.num_max)  *win.num_f   = win.num_max; break;
        }
    }
    else if (get_key(KEY_UP, 50, 1) == 1)
    {
        key_incrementalStep();  // 启动按键渐加步进值
        switch (num_type)
        {
            case 0: *win.num_u8  -= win.num_step; if(*win.num_u8  > win.num_max || *win.num_u8  < win.num_min) *win.num_u8  = win.num_min; break;
            case 1: *win.num_u16 -= win.num_step; if(*win.num_u16 > win.num_max || *win.num_u16 < win.num_min) *win.num_u16 = win.num_min; break;
            case 2: *win.num_u32 -= win.num_step; if(*win.num_u32 > win.num_max || *win.num_u32 < win.num_min) *win.num_u32 = win.num_min; break;
            case 3: *win.num_8   -= win.num_step; if(*win.num_8   > win.num_max || *win.num_8   < win.num_min) *win.num_8   = win.num_min; break;
            case 4: *win.num_16  -= win.num_step; if(*win.num_16  > win.num_max || *win.num_16  < win.num_min) *win.num_16  = win.num_min; break;
            case 5: *win.num_32  -= win.num_step; if(*win.num_32  > win.num_max || *win.num_32  < win.num_min) *win.num_32  = win.num_min; break;
            case 6: *win.num_f   -= win.num_step; if(*win.num_f   > win.num_max || *win.num_f   < win.num_min) *win.num_f   = win.num_min; break;
        }
    }
    else if (get_key(KEY_OK, 500, 1) == 1)
    {
        key_IStepCount = 0;
        popoverQuit(-1); // 弹窗退出
        putAllEeeprom(); // 保存至eeprom
    }
}
// 弹窗退出
void popoverQuit(int8_t state)
{
    win.state = state; // 退出弹窗
    win.yTar = -64;
    win.jdt_yTar = -64;
    win.jdtwk_yTar = -64;

}
 // 按键渐加步进值
void key_incrementalStep()
{
    time_key_IStep = millis();
    ui.loop = 1;
    if (key_IStepCount < 40 + 1) //40表示步进值增量4次 ，必须+1，需要停止
    {
        key_IStepCount++;
        if (key_IStepCount == 1)
            win.num_step_old = win.num_step; //保存旧值，以便在清理的时候归还
    }

    if (key_IStepCount % 10 == 0) //每10次增量一次
        win.num_step = win.num_step * 2; //增量的量，可自己定义
}
//按键渐加步进值清理
void key_incrementalStep_clear()  
{
    if(key_IStepCount == 0) return;
    if (millis() - time_key_IStep > 100) //松开按键100ms清理
    {
        key_IStepCount = 0;
        win.num_step = win.num_step_old;
    }
}

//************ 提示窗动画属于弹窗的延伸 ************
//  提示窗参数传递
void promptWindowTransfer(String name, uint16_t time)
{
    promptWindow_clear_time = time;
    win.state = 2;      // 启动提示窗动画
    win.title = name;   // 标题
    promptWindowInit(); 
}
//  提示窗参数初始化 
void promptWindowInit()
{
    win.xNew = 11; win.yNew = 28; win.wNew = 107; win.hNew = 6;
    win.xTar = 11; win.yTar = 5;  win.wTar = 107; win.hTar = 21;

    win.jdtwk_yNew = win.yTar + 25;
    win.jdtwk_yTar = win.yTar + 32;

    if (u8g2.getUTF8Width(win.title.c_str()) >= 107)
    {
        win.hTar = 36;
    }
}
// 提示窗
void promptWindow()
{
    if (promptWindow_clear_time != 0 && dbtime_promptWindow_clear == 0) // 启动自动清理
    {
        dbtime_promptWindow_clear = millis();
    }
    if (win.state == 2) // 动画开始
    {
        // 外框的动画
        animation2(&win.yNew, &win.yTar, 30);
        animation2(&win.hNew, &win.hTar, 40);
        if (win.hNew > 25) animation2(&win.jdtwk_yNew, &win.jdtwk_yTar, 30);
    }
    else if (win.state == -2) // 动画退出
    {
        animation(&win.yNew, &win.yTar, 120);
        if (win.yNew < -43) win.state = 0;
    }

    u8g2.setDrawColor(1);                                 // 0透显，1实显，2反色
    u8g2.drawBox(win.xNew, win.yNew, win.wNew, win.hNew); // 绘制外框背景
    u8g2.setDrawColor(2);                                 // 0透显，1实显，2反色
    u8g2.drawBox(win.xNew, win.yNew, win.wNew, win.hNew); // 绘制外框背景
    u8g2.setDrawColor(1);
    u8g2.drawRFrame(win.xNew, win.yNew, win.wNew, win.hNew, 3);

    uint16_t num_y0 = win.yNew + 16;
    uint16_t num_y1 = win.yNew + 32;

    if (win.hTar <= 21) // 一行字
    {
        if (win.hNew > 16)
            u8g2.drawUTF8(getCenterX(win.title), num_y0, win.title.c_str());
    }
    else // 二行字进行拆分
    {
        String str0 = "", str1 = "";
        uint16_t mid = 0;
        uint16_t length = win.title.length();
        uint16_t width = u8g2.getUTF8Width(win.title.c_str()) - 2;
        for (int i = 0; i < length; i++)
        {
            if ((win.title[i] & 0xf0) == 0xe0) // 3位utf-8
            {
                    str0 += win.title[i];
                    str0 += win.title[i + 1];
                    str0 += win.title[i + 2];
                    i += 2;
            }
            else if ((win.title[i] & 0xe0) == 0xc0) // 2位utf-8
            {
                    str0 += win.title[i];
                    str0 += win.title[i + 1];
                    i += 1;
            }
            else
            {
                    str0 += win.title[i];
            }
            if (u8g2.getUTF8Width(str0.c_str()) >= width / 2)
            {
                    mid = i;
                    break;
            }
        }
        str1 = win.title.substring(mid + 1);
        if (win.hNew > 16) //绘制第1段
            u8g2.drawUTF8(getCenterX(str0), num_y0, str0.c_str());
        if (win.hNew >= 36)  //绘制第2段
            u8g2.drawUTF8(getCenterX(str1), num_y1, str1.c_str());
        else if (win.hNew > 25) //退出用
            u8g2.drawUTF8(getCenterX(str1), win.jdtwk_yNew, str1.c_str());
    }
}
// 提示窗按键逻辑 
void promptWindow_Key()
{
    if (get_key(KEY_OK, 500, 1) == 1)
    {
        popoverQuit(-2); // 提示窗退出
    }
}

void promptWindow_clear() // 提示窗清理
{
    if (dbtime_promptWindow_clear == 0) return;

    if (millis() - dbtime_promptWindow_clear > promptWindow_clear_time)
    {
        popoverQuit(-2); // 提示窗退出
        dbtime_promptWindow_clear = 0;  // 对比时间清零
        promptWindow_clear_time = 0;    // 滞后时间清零
    }
}

// 显示界面切换，自动计算层级
void DisplaySW(uint8_t interface)
{
    fadeInterface = interface; // 使用转场

    if (fadeInterface > displayInterface)      // 进入下一层,输入的界面大于现在的界面
        win.xkwz_Cache[win.level++] = ui.xkwz; // 存储层级

    else                                       // 进入上一层,输入的界面小于现在的界面
        ui.xkwz = win.xkwz_Cache[--win.level]; // 归还层级
}

//oled休眠函数
void oledSleep()
{
    if (eep_oledScreenOff == 0 || oled_sleepp_time == 0)
        return;
    if (millis() - oled_sleepp_time > eep_oledScreenOff * 60 * 1000)
    {
        u8g2.setPowerSave(1);
        oled_sleepp_time = 0;
    }
}

void oledContrastChange() //oled亮度更改
{
    if (oledContrastOld != eep_oledContrast)
    {
        u8g2.setContrast(eep_oledContrast); // 设置亮度
        oledContrastOld = eep_oledContrast;
    }
}