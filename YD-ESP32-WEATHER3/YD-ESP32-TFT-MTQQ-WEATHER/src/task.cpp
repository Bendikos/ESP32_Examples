#include <TaskScheduler.h>
#include "tftUtil.h"
#include "net.h"
#include "common.h"

enum CurrentPage currentPage = SETTING;
String scrollText[5];        // 轮播天气的信息
int currentIndex = 0;        // 轮播索引
int currnetImgAnimIndex = 0; // 太空人动画索引
int tipsIndex = 0;           // 获取数据时，动态文字的索引
NowWeather nowWeather;       // 记录查询到的实况天气数据
FutureWeather futureWeather; // 记录查询到的七日天气数据
unsigned int timerCount = 0; // 计数器的值(ms)

void drawWeatherContent();
void drawFutureWeatherPage();
void doScrollTextData(String win, int tem_day, int tem_night, int air, String wea);
void drawWeaImage(String wea_img);
void drawFutureWeaImage(String wea_img, int x, int y);
void drawCityAir(String city, int air);
void drawTHProgressBar(int temperature, int humidity);
void drawDateWeek();
void drawTitle();
void drawNumsByCount(unsigned int count);
String week(int tm_wday);
String monthDay(int tm_mon, int tm_mday);

//////////////////////////////// 多任务区域/////////////////////////////
void tAnimCallback();
void tScrollTextCallback();
void tQueryWeatherCallback();
void tQueryFutureWeatherCallback();
void tCheckWiFiCallback();
void tCheckTimeCallback();
Scheduler runner;
Task tAnim(30, TASK_FOREVER, &tAnimCallback, &runner, true);                                          // 30毫秒播放一帧太空人动画
Task tScrollText(5000, TASK_FOREVER, &tScrollTextCallback, &runner, true);                            // 5秒轮播一条天气情况
Task tQueryWeather(60 * 60 * 1000, TASK_FOREVER, &tQueryWeatherCallback, &runner, false);             // 60分钟查询一次实况天气情况
Task tQueryFutureWeather(71 * 60 * 1000, TASK_FOREVER, &tQueryFutureWeatherCallback, &runner, false); // 71分钟查询一次一周天气情况
Task tCheckWiFi(5 * 60 * 1000, TASK_FOREVER, &tCheckWiFiCallback, &runner, true);                     // 5分钟检查一次网络状态
// Task tCheckTime(58 * 60 * 1000, TASK_FOREVER, &tCheckTimeCallback, &runner, true); // 58分钟进行一次NTP对时
//  启动runner
void startRunner()
{
  runner.startNow();
}
// 执行runner
void executeRunner()
{
  runner.execute();
}
// 切换到非实况天气页面时，失能太空人动画和轮播天气情况的线程
void disableAnimScrollText()
{
  tAnim.disable();
  tScrollText.disable();
}
// 切换回实况天气页面时，再开启太空人动画和轮播天气情况的线程
void enableAnimScrollText()
{
  tAnim.enable();
  tScrollText.enable();
}
////////////////////////////////////////////////////////////////////////

//////////////// 定时器相关区域//////////////////////////////////////////
// 定义定时器
hw_timer_t *timerQueryWeather = NULL;
hw_timer_t *timerShowTips = NULL;
void IRAM_ATTR onTimerQueryWeather()
{
  // 使能查询天气的多线程任务
  tQueryWeather.enable();
  tQueryFutureWeather.enable();
}
void IRAM_ATTR onTimerShowTips()
{
  // 获取数据时给用户提示
  if (tipsIndex == 0)
  {
    draw2LineText("同步天气数据", ".");
  }
  else if (tipsIndex == 1)
  {
    draw2LineText("同步天气数据", "..");
  }
  else if (tipsIndex == 2)
  {
    draw2LineText("同步天气数据", "...");
  }
  else if (tipsIndex == 3)
  {
    draw2LineText("同步天气数据", "....");
  }
  else if (tipsIndex == 4)
  {
    draw2LineText("同步天气数据", ".....");
  }
  else if (tipsIndex == 5)
  {
    draw2LineText("同步天气数据", "......");
  }
  tipsIndex++;
  if (tipsIndex == 6)
  {
    tipsIndex = 0;
  }
}
// 初始化定时器，让查询天气的多线程任务在一小时后再使能
void startTimerQueryWeather()
{
  timerQueryWeather = timerBegin(0, 80, true); // 1us计数一次
  timerAttachInterrupt(timerQueryWeather, &onTimerQueryWeather, true);
  timerAlarmWrite(timerQueryWeather, 3600000000, false); // 执行完一次后就取消这个定时器 (3600000000)3600秒，即60分钟后使能
  timerAlarmEnable(timerQueryWeather);
}
// 初始化定时器，在获取初始数据时，给用户提示
void startTimerShowTips()
{
  timerShowTips = timerBegin(1, 80, true); // 1us计数一次
  timerAttachInterrupt(timerShowTips, &onTimerShowTips, true);
  timerAlarmWrite(timerShowTips, 1000000, true); // 每秒执行一次
  timerAlarmEnable(timerShowTips);
}
///////////////////////////////////////////////////////////////////////

/////////// 绘制整个页面区域////////////////////////////////////////////
// 绘制一周天气页面
void drawFutureWeatherPage()
{
  // reflashTFT(); // 清空屏幕
  // // 绘制标题
  // clk.createSprite(240, 60);
  // clk.loadFont(clock_other_title_35);
  // clk.setTextDatum(CC_DATUM);
  // clk.setTextColor(penColor);
  // clk.fillSprite(backFillColor);
  // clk.drawString("6日天气预报", 120, 30);
  // clk.drawFastHLine(0, 56, 240, penColor);
  // clk.drawFastHLine(0, 58, 240, penColor);
  // clk.pushSprite(0, 0);
  // clk.deleteSprite();
  // // 绘制天气图案
  // drawFutureWeaImage(futureWeather.day1wea_img, 15, 65);
  // drawFutureWeaImage(futureWeather.day2wea_img, 95, 65);
  // drawFutureWeaImage(futureWeather.day3wea_img, 175, 65);
  // drawFutureWeaImage(futureWeather.day4wea_img, 15, 195);
  // drawFutureWeaImage(futureWeather.day5wea_img, 95, 195);
  // drawFutureWeaImage(futureWeather.day6wea_img, 175, 195);
  // // 绘制上部分文字
  // clk.loadFont(clock_future_weather_20);
  // clk.setTextDatum(CC_DATUM);
  // clk.createSprite(240, 70);
  // clk.setTextColor(penColor);
  // clk.fillSprite(backFillColor);
  // clk.drawString(futureWeather.day1date.substring(5), 40, 10);
  // clk.drawString(futureWeather.day1wea, 40, 35);
  // clk.drawString(String(futureWeather.day1tem_night) + "-" + String(futureWeather.day1tem_day) + "℃", 40, 60);
  // clk.drawString(futureWeather.day2date.substring(5), 120, 10);
  // clk.drawString(futureWeather.day2wea, 120, 35);
  // clk.drawString(String(futureWeather.day2tem_night) + "-" + String(futureWeather.day2tem_day) + "℃", 120, 60);
  // clk.drawString(futureWeather.day3date.substring(5), 200, 10);
  // clk.drawString(futureWeather.day3wea, 200, 35);
  // clk.drawString(String(futureWeather.day3tem_night) + "-" + String(futureWeather.day3tem_day) + "℃", 200, 60);
  // clk.pushSprite(0, 115);
  // clk.deleteSprite();
  // // 绘制下部分文字
  // clk.createSprite(240, 70);
  // clk.setTextColor(penColor);
  // clk.fillSprite(backFillColor);
  // clk.drawString(futureWeather.day4date.substring(5), 40, 10);
  // clk.drawString(futureWeather.day4wea, 40, 35);
  // clk.drawString(String(futureWeather.day4tem_night) + "-" + String(futureWeather.day4tem_day) + "℃", 40, 60);
  // clk.drawString(futureWeather.day5date.substring(5), 120, 10);
  // clk.drawString(futureWeather.day5wea, 120, 35);
  // clk.drawString(String(futureWeather.day5tem_night) + "-" + String(futureWeather.day5tem_day) + "℃", 120, 60);
  // clk.drawString(futureWeather.day6date.substring(5), 200, 10);
  // clk.drawString(futureWeather.day6wea, 200, 35);
  // clk.drawString(String(futureWeather.day6tem_night) + "-" + String(futureWeather.day6tem_day) + "℃", 200, 60);
  // clk.pushSprite(0, 245);
  // clk.deleteSprite();
  // clk.unloadFont();
}
// 绘制实况天气页面
void drawWeatherPage()
{
  // 清空屏幕
  reflashTFT();

  // 绘制标题
  //  drawTitle();

  // 绘制时间、日期、星期
  drawDateWeek();

  // 绘制温湿度图标
  if (backColor == BACK_BLACK)
  {
    tft.pushImage(0, 160 - 20 * 2, 20, 20, temperature_black);
    tft.pushImage(0, 160 - 20, 20, 20, humidity_black);
  }
  else
  {
    tft.pushImage(0, 160 - 20 * 2, 20, 20, temperature);
    tft.pushImage(0, 160 - 20, 20, 20, humidity);
  }
  // 绘制天气相关内容
  drawWeatherContent();
}
// 绘制计时器页面
void drawTimerPage()
{
  // // 清空屏幕
  // reflashTFT();
  // // 绘制标题
  // clk.createSprite(240, 60);
  // clk.loadFont(clock_other_title_35);
  // clk.setTextDatum(CC_DATUM);
  // clk.setTextColor(penColor);
  // clk.fillSprite(backFillColor);
  // clk.drawString("计时器", 120, 30);
  // clk.drawFastHLine(0, 56, 240, penColor);
  // clk.drawFastHLine(0, 58, 240, penColor);
  // clk.pushSprite(0, 0);
  // clk.deleteSprite();
  // // 显示提示文字
  // clk.setColorDepth(8);
  // clk.setTextDatum(CC_DATUM);
  // clk.loadFont(clock_timer_24);
  // clk.createSprite(240, 60);
  // clk.setTextColor(penColor);
  // clk.fillSprite(backFillColor);
  // clk.drawString("单击开启/停止计时", 120, 45);
  // clk.drawString("长按3秒计数器归零", 120, 15);
  // clk.pushSprite(0, 250);
  // clk.deleteSprite();
  // clk.unloadFont();
  // // 根据计数器当前的数值进行绘制
  // drawNumsByCount(timerCount);
}
// 绘制出厂设置页面
void drawResetPage()
{
  // // 清空屏幕
  // reflashTFT();
  // // 绘制标题
  // clk.createSprite(240, 60);
  // clk.loadFont(clock_other_title_35);
  // clk.setTextDatum(CC_DATUM);
  // clk.setTextColor(penColor);
  // clk.fillSprite(backFillColor);
  // clk.drawString("还原设置", 120, 30);
  // clk.drawFastHLine(0, 56, 240, penColor);
  // clk.drawFastHLine(0, 58, 240, penColor);
  // clk.pushSprite(0, 0);
  // clk.deleteSprite();
  // // 绘制提示文字
  // clk.createSprite(240, 260);
  // clk.loadFont(clock_tips_28);
  // clk.setTextDatum(CC_DATUM);
  // clk.setTextColor(penColor);
  // clk.fillSprite(backFillColor);
  // clk.drawString("长按3秒钟", 120, 90);
  // clk.drawString("恢复出厂设置", 120, 140);
  // clk.pushSprite(0, 60);
  // clk.deleteSprite();
}
// 绘制主题设置页面
void drawThemePage()
{
  // // 清空屏幕
  // reflashTFT();
  // // 绘制标题
  // clk.createSprite(240, 60);
  // clk.loadFont(clock_other_title_35);
  // clk.setTextDatum(CC_DATUM);
  // clk.setTextColor(penColor);
  // clk.fillSprite(backFillColor);
  // clk.drawString("主题切换", 120, 30);
  // clk.drawFastHLine(0, 56, 240, penColor);
  // clk.drawFastHLine(0, 58, 240, penColor);
  // clk.pushSprite(0, 0);
  // clk.deleteSprite();
  // // 绘制提示文字
  // clk.createSprite(240, 260);
  // clk.loadFont(clock_tips_28);
  // clk.setTextDatum(CC_DATUM);
  // clk.setTextColor(penColor);
  // clk.fillSprite(backFillColor);
  // clk.drawString("长按3秒钟", 120, 90);
  // if (backColor == BACK_BLACK)
  // {
  //   clk.drawString("切换白色主题", 120, 140);
  // }
  // else
  // {
  //   clk.drawString("切换黑色主题", 120, 140);
  // }
  // clk.pushSprite(0, 60);
  // clk.deleteSprite();
}
/////////////////////////////////////////////////////////////////////

/////////// 多任务协程回调区域 ////////////////////////////////////////
// NTP对时
void tCheckTimeCallback()
{
  getNTPTime();
}
// 检查Wifi状态,如果失败，重新连接
void tCheckWiFiCallback()
{
  Serial.println("开始检查网络状态");
  checkWiFiStatus();
}
// 绘制太空人任务
void tAnimCallback()
{
  int x = 89, y = 120, w = 40;
  if (backColor == BACK_BLACK)
  {
    switch (currnetImgAnimIndex)
    {
    case 0:
      tft.pushImage(x, y, w, w, yuhangyuan0_black);
      break;
    case 1:
      tft.pushImage(x, y, w, w, yuhangyuan1_black);
      break;
    case 2:
      tft.pushImage(x, y, w, w, yuhangyuan2_black);
      break;
    case 3:
      tft.pushImage(x, y, w, w, yuhangyuan3_black);
      break;
    case 4:
      tft.pushImage(x, y, w, w, yuhangyuan4_black);
      break;
    case 5:
      tft.pushImage(x, y, w, w, yuhangyuan5_black);
      break;
    case 6:
      tft.pushImage(x, y, w, w, yuhangyuan6_black);
      break;
    case 7:
      tft.pushImage(x, y, w, w, yuhangyuan7_black);
      break;
    case 8:
      tft.pushImage(x, y, w, w, yuhangyuan8_black);
      break;
    case 9:
      tft.pushImage(x, y, w, w, yuhangyuan9_black);
      break;
    default:
      currnetImgAnimIndex = 9;
      break;
    }
  }
  else
  {
    switch (currnetImgAnimIndex)
    {
    case 0:
      tft.pushImage(x, y, w, w, yuhangyuan0);
      break;
    case 1:
      tft.pushImage(x, y, w, w, yuhangyuan1);
      break;
    case 2:
      tft.pushImage(x, y, w, w, yuhangyuan2);
      break;
    case 3:
      tft.pushImage(x, y, w, w, yuhangyuan3);
      break;
    case 4:
      tft.pushImage(x, y, w, w, yuhangyuan4);
      break;
    case 5:
      tft.pushImage(x, y, w, w, yuhangyuan5);
      break;
    case 6:
      tft.pushImage(x, y, w, w, yuhangyuan6);
      break;
    case 7:
      tft.pushImage(x, y, w, w, yuhangyuan7);
      break;
    case 8:
      tft.pushImage(x, y, w, w, yuhangyuan8);
      break;
    case 9:
      tft.pushImage(x, y, w, w, yuhangyuan9);
      break;
    default:
      currnetImgAnimIndex = 9;
      break;
    }
  }
  currnetImgAnimIndex += 1;
  if (currnetImgAnimIndex == 10)
  {
    currnetImgAnimIndex = 0;
  }
}

// 轮播ScrollText任务
void tScrollTextCallback()
{
  if (scrollText[currentIndex])
  {
    clk.setColorDepth(8);
    clk.loadFont(HarmonyOS_Sans_SC_Medium16);
    for (int pos = 0; pos < 42; pos++)
    {
      clk.createSprite(85, 20);
      clk.setTextColor(penColor);
      clk.fillSprite(backFillColor);
      clk.setTextDatum(CC_DATUM);
      clk.drawString(scrollText[currentIndex], pos, 12);
      clk.pushSprite(0, 20);
    }

    clk.deleteSprite();
    clk.unloadFont();
    if (currentIndex >= 3)
    {
      currentIndex = 0;
    }
    else
    {
      currentIndex += 1;
    }
    // Serial.print("当前轮播第");Serial.print(currentIndex);Serial.println("条信息");
  }
}

// 查询天气
void tQueryWeatherCallback()
{
  getNowWeather();
  if (queryNowWeatherSuccess)
  { // 查询天气成功，再继续下面的操作
    Serial.println("更新天气信息");
    // 如果是在实况天气页面，则绘制天气相关内容
    if (currentPage == WEATHER)
    {
      drawWeatherContent();
    }
  }
}

// 查询未来天气
void tQueryFutureWeatherCallback()
{
  getFutureWeather();
  if (queryFutureWeatherSuccess)
  { // 查询一周天气成功，再继续下面的操作
    Serial.println("更新一周天气信息");
    // 如果是在一周天气页面，则绘制一周天气相关内容
    if (currentPage == FUTUREWEATHER)
    {
      drawFutureWeatherPage();
    }
  }
}
/////////////////////////////////////////////////////////////////////

// 根据计数器当前的数值进行绘制
void drawNumsByCount(unsigned int count)
{
  // clk.setColorDepth(8);
  // clk.setTextDatum(CC_DATUM);
  // clk.createSprite(240, 180);
  // clk.setTextColor(penColor);
  // clk.fillSprite(backFillColor);
  // // 根据 count 计数器的毫秒值，计算对应的小时、分、秒、毫秒
  // int hour = count / 1000 / 60 / 60;
  // int minute = (count / 1000 / 60) % 60;
  // int second = (count / 1000) % 60;
  // int millisecond = count % 1000;
  // clk.loadFont(clock_timer_hour_52);
  // clk.setTextColor(penColor);
  // clk.drawString(String(hour) + " H", 120, 50);
  // clk.unloadFont();
  // clk.loadFont(clock_num_big_64);
  // clk.drawString(String(minute / 10), 20, 120);
  // clk.drawString(String(minute % 10), 55, 120);
  // clk.drawString(":", 80, 120);
  // clk.setTextColor(0xFC60);
  // clk.drawString(String(second / 10), 110, 120);
  // clk.drawString(String(second % 10), 145, 120);
  // clk.unloadFont();
  // clk.loadFont(clock_num_50);
  // clk.setTextColor(TFT_RED);
  // clk.drawString(String((millisecond / 10) / 10), 187, 123);
  // clk.drawString(String((millisecond / 10) % 10), 215, 123);
  // clk.unloadFont();
  // // Serial.print(String(millisecond));Serial.println("毫秒");
  // // Serial.print(String(second));Serial.println("秒");
  // // Serial.print(String(minute));Serial.println("分");
  // // Serial.print(String(hour));Serial.println("时");
  // clk.pushSprite(0, 70);
  // clk.deleteSprite();
}

// 绘制天气相关内容
void drawWeatherContent()
{
  // 将显示温湿度文字的地方先置为底色，防止温度刷新时，屏幕有残影
  // clk.createSprite(57, 80);
  // clk.fillSprite(backFillColor);
  // clk.pushSprite(113, 230);
  // clk.deleteSprite();

  // 绘制温湿度条
  drawTHProgressBar(nowWeather.tem,nowWeather.humidity);

  // 绘制城市、空气质量
  drawCityAir(nowWeather.city, nowWeather.air);

  // 绘制天气图案和文字
  drawWeaImage(nowWeather.wea_img);

  // 处理轮播字条数据(风向win,最高温度tem_day,最低温度tem_night,空气指数air,天气实况wea)
  doScrollTextData(nowWeather.win, nowWeather.tem_day, nowWeather.tem_night, nowWeather.air, nowWeather.wea);
}

// 处理轮播字条数据(风向win,最高温度tem_day,最低温度tem_night,空气指数air,天气实况wea)
void doScrollTextData(String win, int tem_day, int tem_night, int air, String wea)
{
  scrollText[0] = "";
  scrollText[1] = "";
  scrollText[2] = "";
  scrollText[3] = "";
  scrollText[4] = "";
  scrollText[0] = wea;
  scrollText[1] = win;
  scrollText[2] += "最低";
  scrollText[2] += tem_night;
  scrollText[2] += "℃";
  scrollText[3] += "最高";
  scrollText[3] += tem_day;
  scrollText[3] += "℃";
  // scrollText[4] += "空气";
  // scrollText[4] += air;
}
// 绘制天气图案
void drawWeaImage(String wea_img)
{
  String s = "";
  if (backColor == BACK_BLACK)
  {
    if (wea_img.equals("xue"))
    {
      s = "雪";
      tft.pushImage(89, 0, 40, 40, xue_black);
    }
    else if (wea_img.equals("lei"))
    {
      s = "雷";
      tft.pushImage(89, 0, 40, 40, lei_black);
    }
    else if (wea_img.equals("shachen"))
    {
      s = "沙尘";
      tft.pushImage(89, 0, 40, 40, shachen_black);
    }
    else if (wea_img.equals("wu"))
    {
      s = "雾";
      tft.pushImage(89, 0, 40, 40, wu_black);
    }
    else if (wea_img.equals("bingbao"))
    {
      s = "冰雹";
      tft.pushImage(89, 0, 40, 40, bingbao_black);
    }
    else if (wea_img.equals("yun"))
    {
      s = "多云";
      tft.pushImage(89, 0, 40, 40, yun_black);
    }
    else if (wea_img.equals("yu"))
    {
      s = "雨";
      tft.pushImage(89, 0, 40, 40, yu_black);
    }
    else if (wea_img.equals("yin"))
    {
      s = "阴";
      tft.pushImage(89, 0, 40, 40, yin_black);
    }
    else if (wea_img.equals("qing"))
    {
      s = "晴";
      tft.pushImage(89, 0, 40, 40, qing_black);
    }
  }
  else
  {
    if (wea_img.equals("xue"))
    {
      s = "雪";
      tft.pushImage(89, 0, 40, 40, xue);
    }
    else if (wea_img.equals("lei"))
    {
      s = "雷";
      tft.pushImage(89, 0, 40, 40, lei);
    }
    else if (wea_img.equals("shachen"))
    {
      s = "沙尘";
      tft.pushImage(89, 0, 40, 40, shachen);
    }
    else if (wea_img.equals("wu"))
    {
      s = "雾";
      tft.pushImage(89, 0, 40, 40, wu);
    }
    else if (wea_img.equals("bingbao"))
    {
      s = "冰雹";
      tft.pushImage(89, 0, 40, 40, bingbao);
    }
    else if (wea_img.equals("yun"))
    {
      s = "多云";
      tft.pushImage(89, 0, 40, 40, yun);
    }
    else if (wea_img.equals("yu"))
    {
      s = "雨";
      tft.pushImage(89, 0, 40, 40, yu);
    }
    else if (wea_img.equals("yin"))
    {
      s = "阴";
      tft.pushImage(89, 0, 40, 40, yin);
    }
    else if (wea_img.equals("qing"))
    {
      s = "晴";
      tft.pushImage(89, 0, 40, 40, qing);
    }
  }
}

// 绘制一周天气图案
void drawFutureWeaImage(String wea_img, int x, int y)
{
  if (backColor == BACK_BLACK)
  {
    if (wea_img.equals("xue"))
    {
      tft.pushImage(x, y, 50, 50, xue_black);
    }
    else if (wea_img.equals("lei"))
    {
      tft.pushImage(x, y, 50, 50, lei_black);
    }
    else if (wea_img.equals("shachen"))
    {
      tft.pushImage(x, y, 50, 50, shachen_black);
    }
    else if (wea_img.equals("wu"))
    {
      tft.pushImage(x, y, 50, 50, wu_black);
    }
    else if (wea_img.equals("bingbao"))
    {
      tft.pushImage(x, y, 50, 50, bingbao_black);
    }
    else if (wea_img.equals("yun"))
    {
      tft.pushImage(x, y, 50, 50, yun_black);
    }
    else if (wea_img.equals("yu"))
    {
      tft.pushImage(x, y, 50, 50, yu_black);
    }
    else if (wea_img.equals("yin"))
    {
      tft.pushImage(x, y, 50, 50, yin_black);
    }
    else if (wea_img.equals("qing"))
    {
      tft.pushImage(x, y, 50, 50, qing_black);
    }
  }
  else
  {
    if (wea_img.equals("xue"))
    {
      tft.pushImage(x, y, 50, 50, xue);
    }
    else if (wea_img.equals("lei"))
    {
      tft.pushImage(x, y, 50, 50, lei);
    }
    else if (wea_img.equals("shachen"))
    {
      tft.pushImage(x, y, 50, 50, shachen);
    }
    else if (wea_img.equals("wu"))
    {
      tft.pushImage(x, y, 50, 50, wu);
    }
    else if (wea_img.equals("bingbao"))
    {
      tft.pushImage(x, y, 50, 50, bingbao);
    }
    else if (wea_img.equals("yun"))
    {
      tft.pushImage(x, y, 50, 50, yun);
    }
    else if (wea_img.equals("yu"))
    {
      tft.pushImage(x, y, 50, 50, yu);
    }
    else if (wea_img.equals("yin"))
    {
      tft.pushImage(x, y, 50, 50, yin);
    }
    else if (wea_img.equals("qing"))
    {
      tft.pushImage(x, y, 50, 50, qing);
    }
  }
}
// 绘制城市、空气质量
void drawCityAir(String city, int air)
{
  clk.setColorDepth(8);
  clk.setTextDatum(CC_DATUM);

  // 城市
  clk.loadFont(HarmonyOS_Sans_SC_Medium16);
  clk.createSprite(60, 20);
  clk.fillSprite(backFillColor);
  clk.setTextColor(0xFC60);
  clk.drawString(city, 30, 12);
  clk.pushSprite(0, 0);
  clk.deleteSprite();
  clk.unloadFont();

  // 空气质量， ≤50优（绿色），≤100良（蓝），≤150中（橙），≤200差（红）
  clk.loadFont(HarmonyOS_Sans_SC_Medium16);
  String level;
  uint16_t color;
  if (air <= 50)
  {
    color = 0x0E27;
    level = "优";
  }
  else if (air <= 100)
  {
    color = 0x2C3E;
    level = "良";
  }
  else if (air <= 150)
  {
    color = 0xFC60;
    level = "中";
  }
  else
  {
    color = TFT_RED;
    level = "差";
  }
  clk.createSprite(24, 20);
  clk.fillSprite(backFillColor);
  clk.fillRoundRect(0, 1, 84 - 60, 18, 3, color); // 进度条填充(x方向,y方向,长度,高度,圆角,颜色)
  clk.setTextColor(0xFFFF);
  clk.drawString(level, 12, 12);
  clk.pushSprite(60, 0);
  clk.deleteSprite();
  clk.unloadFont();
}

// 绘制温湿度进度条和温湿度数据
void drawTHProgressBar(int temperature, int humidity)
{
  int temperatureLen, humidityLen;
  int totalLen = 24;
  uint16_t temperatureColor;

  // 计算湿度要显示的长度
  humidityLen = (humidity / 100.0) * totalLen;

  // 计算温度要显示的长度以及要显示的颜色
  if (temperature >= 40)
  { // 超过40度，红色高温预警
    temperatureLen = totalLen;
    temperatureColor = TFT_RED;
  }
  else if (temperature >= 35)
  { // 超过37度，橙色高温预警
    temperatureLen = totalLen * 0.8;
    temperatureColor = 0xFC60;
  }
  else if (temperature > 0 && temperature < 35)
  {
    temperatureLen = totalLen *0.6;
    temperatureColor = 0x2C3E;
  }
  else if (temperature <= 0)
  {
    temperatureLen = totalLen *0.2;
    if (temperatureLen < 0)
    {
      temperatureLen = 0;
    }
    temperatureColor = TFT_DARKGREY;
  }
  // Serial.print("temperatureLen:");Serial.println(temperatureLen);
  // Serial.print("humidityLen:");Serial.println(humidityLen);

  // 显示温湿度数据
  clk.createSprite(40, 20);
  clk.loadFont(HarmonyOS_Sans_SC_Medium16);
  clk.setTextColor(penColor, backFillColor);
  clk.setTextDatum(MR_DATUM);
  clk.drawNumber(temperature, 24, 12);
  clk.drawString("℃", 40, 12);
  clk.pushSprite(45, 160 - 20 * 2);
  clk.deleteSprite();

  char strTemp[32] = {0};
  sprintf(strTemp, "%d%%", humidity);
  clk.createSprite(40, 20);
  clk.loadFont(HarmonyOS_Sans_SC_Medium16);
  clk.setTextColor(penColor, backFillColor);
  clk.setTextDatum(CC_DATUM);
  clk.drawString(strTemp, 44 / 2 + 2, 12);
  clk.pushSprite(45, 160 - 20);
  clk.deleteSprite();
  tft.unloadFont();

  // 绘制温度条
  clk.setColorDepth(8);
  clk.createSprite(26, 20); // 创建布局大小 宽x高
  clk.fillSprite(backFillColor);
  clk.fillRect(1, 11, temperatureLen, 4, temperatureColor);    // 进度条填充(x方向,y方向,长度,高度,圆角,颜色)
  clk.drawRoundRect(0, 10, 26, 6, 2, penColor); // 画进度条外边(x方向,y方向,长度,高度,圆角,颜色)
  clk.pushSprite(20, 160 - 20 * 2);                                         // 布局坐标
  clk.deleteSprite();

  // 绘制湿度条
  clk.setColorDepth(8);
  clk.createSprite(26, 20); // 创建布局大小 宽x高
  clk.fillSprite(backFillColor);
  clk.fillRect(1, 11, humidityLen, 4, 0x0E27);    // 进度条填充(x方向,y方向,长度,高度,圆角,颜色)
  clk.drawRoundRect(0, 10, 26, 6, 2, penColor); // 画进度条外边(x方向,y方向,长度,高度,圆角,颜色)
  clk.pushSprite(20, 160 - 20);                            // 布局坐标
  clk.deleteSprite();
}

// 绘制时间、日期、星期
void drawDateWeek()
{
  // 获取RTC时间
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("获取RTC时间失败");
    return;
  }

  // 往串口打印当前 小时：分：秒
  // char timeString[12];
  // sprintf(timeString, "%02d:%02d:%02d", timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec);
  // Serial.println(timeString);

  clk.setColorDepth(8);

  // 时、分
  clk.createSprite(55, 60);
  clk.setTextColor(penColor);
  clk.fillSprite(backFillColor);
  clk.loadFont(clock_num_50);
  clk.setTextDatum(CC_DATUM); // 设置文本基准位置（默认为左上角）

  char temp[3] = {0};
  sprintf(temp, "%02d", timeinfo.tm_hour);
  clk.drawString(temp, 55 / 2, 60 / 2 + 5);
  clk.pushSprite(0, 40);
  clk.deleteSprite();

  clk.createSprite(55, 60);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(0xFC60);
  sprintf(temp, "%02d", timeinfo.tm_min);
  clk.drawString(temp, 55 / 2, 60 / 2 + 5);
  clk.pushSprite(55, 40);
  clk.unloadFont();
  clk.deleteSprite();

  // 秒
  clk.createSprite(25, 20);
  clk.fillSprite(backFillColor);
  clk.loadFont(HarmonyOS_Sans_SC_Bold16);
  clk.setTextDatum(TL_DATUM);
  clk.setTextColor(TFT_RED);
  sprintf(temp, "%02d", timeinfo.tm_sec);
  clk.drawString(temp, 0, 0);
  clk.unloadFont();
  clk.pushSprite(109, 78);
  clk.deleteSprite();

  // 底部日期、星期
  clk.loadFont(HarmonyOS_Sans_SC_Medium16);

  // 月日 星期
  clk.createSprite(128, 20);
  clk.setTextColor(penColor);
  clk.fillSprite(backFillColor);
  clk.setTextDatum(CC_DATUM);
  char dayweek[32] = {0};
  sprintf(dayweek, "%s  %s", (char*)monthDay(timeinfo.tm_mon, timeinfo.tm_mday).c_str(), (char*)week(timeinfo.tm_wday).c_str());
  clk.drawString(dayweek, 64, 12);
  clk.pushSprite(0, 160 - 20 * 3);
  clk.deleteSprite();

  clk.unloadFont();
}

// 处理星期
String week(int tm_wday)
{
  String wk[7] = {"日", "一", "二", "三", "四", "五", "六"};
  String s = "星期" + wk[tm_wday];
  return s;
}
// 处理月日
String monthDay(int tm_mon, int tm_mday)
{
  String s = "";
  s = s + (tm_mon + 1);
  s = s + "月" + tm_mday + "日";
  return s;
}
