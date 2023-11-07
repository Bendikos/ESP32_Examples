//由于是自己步进电机开窗开风扇项目，引入了电机库和时钟库，不需要自行删除电机和时钟部分
//数值滑动和消失动画参考至B站UP，框架则是自己重构，直白一点，但每个界面都需要另外创建
//https://www.bilibili.com/video/BV1wo4y1474K/?spm_id_from=333.788.video.desc.click

//****** 页面创建逻辑 ******
//创建页面需要3个函数、3个参数
//3个函数,需到FontBmp.h内添加声明
//页面初始化display_XXX_Init、页面显示display_XXX、页面的按键逻辑display_XXX_Key
//页面初始化函数放入fade()里面、页面显示函数放入loop()、页面的按键逻辑放入loop()
//3个参数
//页面的位置 #define DISPLAY_XXX 、页面有多少个选项 #define OptMax_XXX、页面选框位置记录 int8_t display_XXX_xxwz

//列表模版参考DisplaySet.h、DisplayOledSet.h、DisplayMotoSet.h、DisplayAbout.h即可
//创建列表页面只需要复制以上的模版，修改【选项setOpt】和拼装数值的【for循环】
//按键函数display_XXX_Key，一般无需修改选项的【选项切换】项，修改【选项确认】项做相应的功能即可

//****** 引用库 ******
#include <Arduino.h>
#include <WiFi.h> 
#include <WiFiClient.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ESPmDNS.h>
#include <WiFiClientSecure.h>
#include <LittleFS.h>
#include "SPI.h"
#include "FastAccelStepper.h"
#include <FS.h>
fs::FS fileSystem = LittleFS;  //配网模式-高级文件管理器使用的FS LittleFS
fs::FS fileSystem2 = LittleFS; //配网模式-高级文件管理器使用的FS LittleFS或SD 
const char* currentFsName = "LittleFS";

#include <U8g2lib.h> //OLED图形驱动库
#include <Wire.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA);

#if !(USING_DEFAULT_ARDUINO_LOOP_STACK_SIZE)
  uint16_t USER_CONFIG_ARDUINO_LOOP_STACK_SIZE = 16*1024;
#endif

struct FSInfo
{
  size_t totalBytes; // 整个文件系统的大小
  size_t usedBytes;  // 已用大小
  size_t blockSize;  // 块大小
  int8_t fatType;    // 卡类型 16=FAT16 32=FAT32 -1=识别失败 0=无卡
};FSInfo info;

#include <RX8010SJ.h>
#define RX8010_I2C_ADDR 0x32
const char * dayOfWeekStrings[] = {
	"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"
};
RX8010SJ::Adapter adapter = RX8010SJ::Adapter(RX8010_I2C_ADDR);
RX8010SJ::DateTime defaultDateTime = RX8010SJ::DateTime();
RX8010SJ::DateTime dateTime;
// 硬件时钟芯片是否正常
#define szxp_ok bitRead(RTC_clock_code, 0) == 1 && bitRead(RTC_clock_code, 1) == 1

//ntp时间设置
#include <NTPClient.h>
#include <WiFiUdp.h>
#define SECONDS_IN_DAY          86400 // 一天中的秒数
#define START_YEAR              1970  // 开始年份
#define TIME_ZONE               +8    // 默认时区
static uint8_t days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const char* NTPadd[5] = {"ntp.ntsc.ac.cn",
                         "ntp1.aliyun.com",
                         "s2k.time.edu.cn",
                         "time1.cloud.tencent.com",
                         "cn.ntp.org.cn"
                        };
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTPadd[0], TIME_ZONE * 3600, 60000); //udp，服务器地址，时间偏移量，更新间隔
struct ntpDate //ntp获取日期的结构体
{
  uint16_t year, month, day;
};

#include <ArduinoJson.h>
#include <HTTPUpdateServer.h>
WebServer server(80);
HTTPUpdateServer httpUpdater;
File fsUploadFile;      // 建立文件对象用于闪存文件上传
//SdFile fsUploadFile;  // 建立文件对象用于闪存文件上传
static bool fsOK = 1;
String unsupportedFiles = "";
static const char TEXT_PLAIN[] PROGMEM = "text/plain";
static const char FS_INIT_ERROR[] PROGMEM = "FS INIT ERROR";
static const char FILE_NOT_FOUND[] PROGMEM = "FileNotFound";

//****** STA设置
//char sta_ssid[32] = {0};
//char sta_password[64] = {0};
IPAddress dns1(114, 114, 114, 114);
IPAddress dns2(114, 114, 115, 115);
//****** AP设置
const char *ap_ssid = "ESP32 E-Paper";
const char *ap_password = "333333333"; // 无密码则为开放式网络 9个3
IPAddress local_IP(192, 168, 3, 3);
IPAddress gateway(192, 168, 3, 3);
IPAddress subnet(255, 255, 255, 0);

//****** 可调参数 & 引脚配置 ******
const String version = "ESP32_MOTO_V001"; // 程序版本
#define DBG_OUTPUT_PORT Serial
// 限位开关
#define SIG1 18 // 最小位置开关
#define SIG2 23 // 最大位置开关
// 继电器
#define JDQ_IN1 16 // 继电器火线
#define JDQ_IN2 17 // 继电器零线
// 按键相关
#define re_lb_max 30 // 旋转编码器滤波次数
#define KEY_UP 33    // 返回键
#define KEY_DOWN 25  // 返回键
#define KEY_OK 26    // 确定键
// i2c相关
// #define I2C_SCL 19
// #define I2C_SDA 22
// I2C地址 0x44-sht30 0x18-es8311 0x32-rx8010

// 步进电机
#define enablePinStepper 12   // 连接步进电机驱动器的ENA
#define stepPinStepper 14     // 连接步进电机驱动器的STEP
#define dirPinStepper 27      // 连接到步进电机驱动器的DIR
#define STEPS_REVOLUTION 3200 // 1/16 细分 -> 3200 步 / 圈
FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper = NULL;
#define windowOpen 0             // 窗户打开
#define windowClose 1            // 窗户关闭
bool returnToOrigin_key = 0;     // 是否需要回到原点 0-不需要 1-需要
uint8_t returnToOrigin_loop = 0; // loop循环里的回原点操作

//****** 一些其他变量 ******
uint8_t wakeup_reason = 255;   // 唤醒原因
uint64_t oled_sleepp_time = 0; // 休眠对比时间
int8_t pw_state = 0;           // 启动ap模式状态  0-无 1-WiFi未配置 2-有配置 3-有配置但连接失败 4-连接成功
int8_t peiwangInitStete = 0;   // 配网初始化  0关闭 1-启动初始化 2-已启动过初始化
#define timingConfigPath  "/system/定时配置.txt"   // 定时文件的路径
uint8_t oledContrastOld = 0;  // 旧的亮度存储
//********** 显示
#define baise 1   // 白色
#define heise 0   // 黑色

//************ 全局UI参数 ************
uint8_t *buf_ptr; // 指向屏幕缓冲的指针
uint16_t buf_len; // 缓冲长度
struct GCUI //  UI结构体
{
  uint8_t loop = 1;   // 动画状态 0-暂停动画 1-动画数值初始化 2-播放动画计算数值
  uint8_t fade = 1;   // 消失动画
  int8_t xkwz = 0;    // 列表的 记录选框的位置 由于中文只有4行所以是0123
  int8_t lbwz = 0;    // 列表的 选项位置，用于记录列表上下移动
  //xkwz + lbwz = 实际的选项位置
};
GCUI ui;
//************************************

//************ 界面状态、选框位置 ************
uint8_t displayInterface = 0;  // 显示哪个界面
uint8_t fadeInterface = 0;     // 转场动画界面
#define DISPLAY_MAIN     0     // 主界面
#define DISPLAY_SET      1     // 设置界面
#define DISPLAY_MOTO_SET 2     // 电机设置界面
#define DISPLAY_OLED_SET 3     // OLED设置界面
#define DISPLAY_PW 4           // 配网界面
#define DISPLAY_ABOUT 5        // 关于界面

#define optMax_main 7             // 主菜单选项的数量(包括标题)
int8_t display_Main_xxwz = 0;    // 主界面选框位置 

#define optMax_set 6              // 设置选项的数量(包括标题)
int8_t display_Set_xxwz = 0;     // 设置界面选项位置

#define optMax_motoSet 8          // 电机设置选项的数量(包括标题)
int8_t display_MotoSet_xxwz = 0; // 电机设置界面选项位置
 
#define optMax_oledSet 4          // oled设置选项的数量(包括标题)
int8_t display_OledSet_xxwz = 0; // oled设置界面选项位置

#define optMax_peiWang 9         // 配网选项的数量(包括标题)
int8_t display_PeiWang_xxwz = 0; // 配网界面选项位置

#define optMax_about 9           // 关于选项的数量(包括标题)
int8_t display_About_xxwz = 0;   // 关于界面选项位置
//************************************

//************ 弹窗界面参数 ************
struct POP
{
  // 1 弹窗开始   -1 弹窗退出
  // 2 提示窗开始 -2 提示窗退出
  int8_t state = 0; // 动画状态

  // 选框位置偏移量缓存，默认10级，不会真有人嵌套10级吧
  uint8_t xkwz_Cache[10] = {0};
  uint8_t level = 0; //层级

  // 弹窗参数
  String title;                 // 标题
  int8_t num_types = -1;        // 数值类型
  int16_t num_min = 0;          // 最小
  int32_t num_max = 100;        // 最大
  int16_t num_step = 1;         // 步进值
  int16_t num_step_old = 1;     // 初始步进值记录
  // 外框参数
  float xTar, yTar, wTar, hTar;
  float xNew, yNew, wNew, hNew;
  // 进度条外框参数
  float jdtwk_xNew, jdtwk_yNew, jdtwk_wNew, jdtwk_hNew;
  float jdtwk_xTar, jdtwk_yTar, jdtwk_wTar, jdtwk_hTar;
  // 进度条框参数
  float jdt_xNew, jdt_yNew, jdt_wNew, jdt_hNew;
  float jdt_xTar, jdt_yTar, jdt_wTar, jdt_hTar;

  // 弹窗要传递的数值类型
  uint8_t *num_u8 = NULL;       
  uint16_t *num_u16 = NULL;     
  uint32_t *num_u32 = NULL;    
  int8_t *num_8 = NULL;        
  int16_t *num_16 = NULL;       
  int32_t *num_32 = NULL;      
  float *num_f;                 
};
POP win;
uint64_t time_key_IStep = 0;          // 按键渐加计数的时间 表示按下按键的时间，用来对比松开按键多少时间将key_IStepCount置0
uint32_t key_IStepCount = 0;          // 按键渐加计数的数值 表示连续按了多少下
uint64_t dbtime_promptWindow_clear = 0; // 提示窗清理对比时间
uint16_t promptWindow_clear_time = 0; // 自动清理滞后时间
//************************************

//************ 列表菜单的参数 ************
// 字符位置、目标值和新值 ，若有任一页的选项数量大于20则需要加大数组数量
float strTar_x[20]; 
float strTar_y[20];
float strNew_x[20];
float strNew_y[20];
//  选框位置结构体
struct xkwz 
{
  float x = 0; // X坐标
  float y = 0; // Y坐标
  float w = 0; // 宽度
  float h = 0; // 高度
};
xkwz xkwzTar; // 目标位置
xkwz xkwzNew; // 新的位置
float jduTar_y = 0; // 进度条Y目标值
float jduNew_y = 0; // 进度条Y新值

// 长字符移动用
float w_moveX;      // 长字符移动用
uint64_t moveX_time = 0; // 长字符移动用
//************************************

//****** EEPROM地址和定义 ******
#include <Preferences.h>
Preferences eep;
String eep_sta_ssid = "";     // wifi名称
String eep_sta_password = ""; // wifi密码
float eep_timeZone = 8.0;     // 时区
String eep_ntpAdd = "";       // 自定义ntp地址
uint16_t eep_clockLWJG = 0;   // 时钟联网校准间隔，0为不联网校准 ，单位小时

uint16_t eep_speed = 5000;      // 电机速度
uint16_t eep_acc = 5000;        // 电机加速度
uint32_t eep_startPoint = 1000; // 起点
uint32_t eep_endPoint = 90000;  // 终点
bool eep_motoDir = 0;

uint16_t eep_oledContrast = 50;  // Oled亮度
uint16_t eep_oledScreenOff = 3;  // 息屏时间,分钟

RTC_DATA_ATTR uint32_t RTC_PW_rsert = 0;       // 重启进入配网 1-需要 0-不需要

// 声明外部文件
#include "gb2312.c"
#include "FontBmp.h"
#include "GanCaoUI.h"
#include "Other.h"
#include "STA_AT_OTA.h" 
#include "CallHttps.h"
#include "key.h"
#include "Clock8010.h"
#include "NTPApi.h"
#include "DisplayMain.h"
#include "DisplaySet.h" 
#include "DisplayMotoSet.h" 
#include "DisplayOledSet.h" 
#include "DisplayAbout.h"
#include "SetupClockCalibration.h" 
#include "eeprom.h"
#include "MotoReturnToOrigin.h"
#include "WEBServer1.h" 
#include "DisplayPeiWang.h"

void setup()
{
  WiFi.mode(WIFI_OFF); //  关闭WIFI

  // 设置按键为上拉模式
  pinMode(KEY_UP, INPUT_PULLUP);
  pinMode(KEY_DOWN, INPUT_PULLUP);
  pinMode(KEY_OK, INPUT_PULLUP);
  // 限位开关设置上拉
  pinMode(SIG1, INPUT_PULLUP);
  pinMode(SIG2, INPUT_PULLUP);
  // 继电器设置输出
  pinMode(JDQ_IN1, OUTPUT);
  pinMode(JDQ_IN2, OUTPUT);
  digitalWrite(JDQ_IN1, 0);
  digitalWrite(JDQ_IN2, 0);

  Serial.begin(115200);

  wakeup_reason = print_wakeup_reason(); //打印启动原因

  eeprom_init(); //eeprom初始化

  // OLED初始化
  u8g2.begin();
  u8g2.setFont(chinese_gb2312);       // 自制的中文字库，内存小的单片机不能用，esp8266和esp32可用
  u8g2.setContrast(eep_oledContrast); // 设置亮度
  u8g2.enableUTF8Print();             // 开启UTF8字符功能
  u8g2.setBusClock(5000000);          // 设置I2C速度 ，一遍的单片机跑不到5Mhz，需降至400Khz
  // u8g2.setFontMode(1);
  // u8g2.setDrawColor(2);

  // 消失转场用
  buf_ptr = u8g2.getBufferPtr();                                        // 获取屏幕缓存指针
  buf_len = 8 * u8g2.getBufferTileHeight() * u8g2.getBufferTileWidth(); // 获取屏幕缓存长度

  setupClockCalibration(); // 检查时钟是否需要校准

  // 电机初始化
  engine.init();
  stepper = engine.stepperConnectToPin(stepPinStepper);
  if (stepper)
  {
    stepper->setDirectionPin(dirPinStepper, eep_motoDir); // 设置方向引脚,本项目0为远离原点
    stepper->setEnablePin(enablePinStepper);              // 设置使能引脚
    stepper->setAutoEnable(true);                         // 设置自动启用

    // 如果自动启用/禁用需要延迟，只需添加（一个或两个）：
    // stepper->setDelayToEnable(50);    // 设置延迟启用
    // stepper->setDelayToDisable(1000); // 设置延迟禁用

    stepper->setSpeedInHz(eep_speed);  // 设置速度输入单位 steps/s
    stepper->setAcceleration(eep_acc); // 设置加速度 100 steps/s²

    // 使用enableOutputs/disableOutput可以启用和禁用步进器
    // 对于设置了autoEnable的运行电机，disableOutputs（）将返回false
    // stepper->enableOutputs();
    // stepper->disableOutputs();
    
    // 限位开关设置上拉
    pinMode(SIG1, INPUT_PULLUP);
    pinMode(SIG2, INPUT_PULLUP);
  }

  while (!stepper)
  {
    u8g2.clearBuffer();
    u8g2.setCursor(0, 14);
    u8g2.print("电机启动失败");
    u8g2.sendBuffer();
    delay(5000);
  }

  //主界面位置初始化
   displau_Main_Init(display_Main_xxwz, optMax_main);
   //弹窗初始化
   popoverInit();

   fsOK = LittleFSBegin(); // 启动文件系统

   // oled休眠用
   oled_sleepp_time = millis();
}


void loop()
{
  return_to_origin_loop();

  u8g2.clearBuffer();

  /************ 增加页面时需要加入到IF ************/
  if (displayInterface == DISPLAY_MAIN) // 主界面
  {
    display_Main(display_Main_xxwz, optMax_main);
    display_Main_Key(&display_Main_xxwz, optMax_main);
  }
  else if (displayInterface == DISPLAY_SET) // 设置选择界面
  {
    display_Set(display_Set_xxwz, optMax_set);
    display_Set_Key(&display_Set_xxwz, optMax_set);
  }
   else if (displayInterface == DISPLAY_MOTO_SET) // 电机设置界面
  {
    display_MotoSet(display_MotoSet_xxwz, optMax_motoSet);
    display_MotoSet_Key(&display_MotoSet_xxwz, optMax_motoSet);
  }
   else if (displayInterface == DISPLAY_OLED_SET) // OLED设置界面
  {
    display_OledSet(display_OledSet_xxwz, optMax_oledSet);
    display_OledSet_Key(&display_OledSet_xxwz, optMax_oledSet);
  }
  else if (displayInterface == DISPLAY_PW) // 配网界面
  {
    display_PeiWang(display_PeiWang_xxwz, optMax_peiWang);
    display_PeiWang_Key(&display_PeiWang_xxwz, optMax_peiWang);
  }
  else if (displayInterface == DISPLAY_ABOUT) // 关于界面
  {
    display_About(display_About_xxwz, optMax_about);
    display_About_Key(&display_About_xxwz, optMax_about);
  }

  /************ 弹窗、提示窗无需改动 ************/
  // 弹窗界面，需放在最下面，以覆盖之前的界面
  if (win.state)
  {
    if (win.state == 1 || win.state == -1) // 数值弹窗
    {
      popover();                   // 数值弹窗
      popover_Key();               // 数值弹窗按键
      key_incrementalStep_clear(); // 按键渐加步进值清理
      oledContrastChange();        // 监测OLED亮度是否更改
    }
    else if (win.state == 2 || win.state == -2) // 提示窗口
    {
      promptWindow();     // 提示窗
      promptWindow_Key(); // 提示窗按键退出
    }
    promptWindow_clear(); // 提示窗清理
  }

  /************ 增加页面时需在fade1函数内添加页面初始化界面 ************/
  fade(fadeInterface); // 消失动画

  u8g2.sendBuffer();

  peiwang_start();

  oledSleep();
}




// r 以只读方式操作文件，读位置在文件的开始位置，文件不存在返回空对象；
// r+ 以可读可写方式打开文件，读写位置在文件的开始位置，文件不存在返回空对象；
// w 截取文件长度到0或者创建新文件，只能写操作，写位置在文件的开始位置；
// w+ 截取文件长度到0或者创建新文件，可读可写操作，写位置在文件的开始位置；
// a 在文件末尾追加内容或者文件不存在就创建新文件，追加位置在当前文件的末尾，只能写操作；
// a+ 在文件末尾追加内容或者文件不存在就创建新文件，追加位置在当前文件的末尾，可读写操作；
// 如果模式值是 SeekSet，则从文件开头移动指定的偏移量。
// 如果模式值是 SeekCur，则从目前的文件位置移动指定的偏移量。
// 如果模式值是 SeekEnd，则从文件结尾处移动指定的偏移量。