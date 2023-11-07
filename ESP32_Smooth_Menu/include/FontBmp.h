// 声明外部字体常量 // #include <clib/u8g2.h>
extern const uint8_t chinese_gb2312[254343] U8G2_FONT_SECTION("chinese_gb2312");

// eeprom.h
void eeprom_init();
void putAllEeeprom();

// Other.h
void esp_sleep(uint32_t minutes); // 系统休眠
uint8_t print_wakeup_reason();    // 获取休眠唤醒原因
String getEspChipId();            // 获取espID
void i2c_init();                  // i2c初始化

// STA_AT_OTA.h
bool connectToWifi();    // 初始化STA配置
void initApSTA();        // 初始化AP&STA配置
void initApSTANoBegin(); // 初始化AP&STA配置无需Begin
void initAp();           // 初始化AP配置
boolean scanNetworks(String currentSSID);

// CallHttps.h
String callHttps(String url);
String callHttp(String url);
// NTPApi.h
bool ntpConnect();                                        // ntp连接程序 0-失败 1-成功
bool ntpToClockChip();                                    // 将NTP数据写入时钟芯片内
void getNTPDate(uint32_t timeStamp, struct ntpDate *jgt); // 获取NTP的日期
bool isLeapYear(unsigned int year);                       // 判断是否闰年
String weekDigitalToChinese(int digital);                 // 星期数字1234567转中文

// WEBServer1.h
void initWebServer();
void webNotFound();      // 设置处理404情况的函数'webUserRequest'
void webRoot();          // 处理网站根目录"/"的访问请求
void webFileUpload();    // 上传至文件系统
void webRead_fileSync(); // 文件系统剩余内存
void webRootPost();      // wifi连接
void webRead_scanWifi(); // 扫描周边wifi
void webRead_staNameIp();
void webRead_info();
void webClockTimeZone();
void webRead_clockTimeZone();
void webClockLWJG();
void webRead_clockLWJG();
void webPut_ntpAdd();
void webRead_ntpAdd();
void webRESET();
void reset_peiwang();
String sendHTML_main();
String getContentType(String filename);
boolean getSystemFile(String fileName);

void replyOK();
void replyOKWithMsg(String msg);
void replyNotFound(String msg);
void replyBadRequest(String msg);
void replyServerError(String msg);
bool existsEdit(String path);
bool exists(fs::FS &fs, String path);

void handleStatus();
void handleFileList();
bool handleFileRead(String path);
String lastExistingParent(String path);
void handleFileCreate();
void handleFileDelete();
void handleFileUpload();
void handleGetEdit();
boolean serverFlieSDInit(); // 网页文件管理器文件挂载

// key.h

// 返回值：0-未按下 1-短按 2-长按
// timeMax: 按键时间滤波
// mod: 0-长短按模式 1-只有短按模式
int8_t get_key(uint8_t key_pin, uint16_t timeMax = 0, bool mod = 0);
bool key_lb(uint8_t key, uint16_t lb_max);

// Clock8010.h
bool clockChipReadCheck(); // 读取时钟数据是否正常
bool clockChipPowerDown(); // 检查是否掉电了
bool clockChipExist();     // 检查时钟芯片是否存在

// SetupClockCalibration.h
void setupClockCalibration(); // 时钟校准程序

// MotoReturnToOrigin.h
void return_to_origin();      // 回到原点的程序-中断
void return_to_origin_loop(); // 回到原点的程序-loop中用

// GanCaoUI.h
uint16_t getCenterX(String z);
uint16_t getCenterX(const char *z);
uint16_t getCenterX(int32_t z);

void animation(float *a, float *a_trg, uint16_t n);
void animation1(float *a, float *a_trg, uint16_t n);
void animation2(float *a, float *a_trg, uint16_t n);
void animation3(float *a, float *a_trg, uint16_t n);
void animation4(float *a, float *a_trg, uint16_t n);

void fade(uint8_t page); // 消失函数

void popoverInit();               // 弹窗参数初始化
void popover();                   // 弹窗函数
void popover_Key();               // 弹窗按键逻辑
void popoverQuit(int8_t state);   // 弹窗退出
void key_incrementalStep();       // 按键渐加步进值
void key_incrementalStep_clear(); // 按键渐加步进值清理

void promptWindowTransfer(String name, uint16_t time = 0); //  提示窗参数传递 time自动清理的滞后时间
void promptWindowInit();                                   //  提示窗参数初始化
void promptWindow();                                       // 提示窗
void promptWindow_Key();                                   // 提示窗按键逻辑
void promptWindow_clear();                                 // 提示窗清理

void DisplaySW(uint8_t interface); // 显示界面切换，自动计算层架
void oledSleep();                  // oled休眠函数
void oledContrastChange();         // oled亮度更改

// DisplayMain.h
void displau_Main_Init(int8_t xxwz, uint8_t optMax); // 主界面位置初始化
void display_Main(int8_t xxwz, uint8_t optMax);      // 主界面显示
void display_Main_Key(int8_t *xxwz, uint8_t optMax); // 主界面的按键逻辑

// DisplaySet.h
void display_Set_Init(int8_t lbwz, uint8_t optMax);
void display_Set(int8_t lbwz, uint8_t optMax);
void display_Set_Key(int8_t *lbwz, uint8_t optMax);

// DisplayMotoSet.h
void display_MotoSet_Init(int8_t lbwz, uint8_t optMax);
void display_MotoSet(int8_t lbwz, uint8_t optMax);
void display_MotoSet_Key(int8_t *lbwz, uint8_t optMax);

// DisplayOledSet.h
void display_OledSet_Init(int8_t lbwz, uint8_t optMax);
void display_OledSet(int8_t lbwz, uint8_t optMax);
void display_OledSet_Key(int8_t *lbwz, uint8_t optMax);

// DisplayPeiWang.h
void display_PeiWang_Init(int8_t lbwz, uint8_t optMax);
void display_PeiWang(int8_t lbwz, uint8_t optMax);
void display_PeiWang_Key(int8_t *lbwz, uint8_t optMax);

// DisplayAbout.h
void display_About_Init(int8_t lbpy, uint8_t optMax);
void display_About(int8_t lbpy, uint8_t optMax);
void display_Set_Key(int8_t *lbpy, uint8_t optMax);
void peiwang_start(); // 启动配网模式
String toStringIp(IPAddress ip);
