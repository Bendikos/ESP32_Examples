/*** 初始化webserver配置 ***/
void initWebServer()
{

  server.on("/", webRoot);                                       //处理主页请求
  server.onNotFound(webNotFound);                                //处理网页url请求和404
  server.on("/Wifi", HTTP_POST, webRootPost);                    //处理主页wifi表单

  server.on("/FileUpdata", HTTP_POST, replyOK, webFileUpload); // 处理文件上传页面respondOK
  server.on("/Read_fileSync", webRead_fileSync);               // ajax 请求当前文件系统剩余内存

  // **************** WEB高级文件管理器初始化 ********************************
  // Filesystem status 文件系统状态
  server.on("/status", HTTP_GET, handleStatus);
  // List directory 列表目录
  server.on("/list", HTTP_GET, handleFileList);
  // Load editor 加载编辑器
  server.on("/edit", HTTP_GET, handleGetEdit);
  // Create file 创建文件
  server.on("/edit",  HTTP_PUT, handleFileCreate);
  // Delete file 删除文件
  server.on("/edit",  HTTP_DELETE, handleFileDelete);
  // 上传文件
  //-在请求以所有解析参数结束后调用第一个回调
  //-第二个回调处理该位置的文件上载
  server.on("/edit",  HTTP_POST, replyOK, handleFileUpload);

  // ********************************************************************
  server.on("/Read_info", webRead_info);           // ajax 请求系统信息
  server.on("/Read_staNameIp", webRead_staNameIp); // ajax 请求STAwifi名称和ip地址
  server.on("/Read_scanWifi", webRead_scanWifi);   // ajax 扫描周边wifi

  server.on("/Put_clockTimeZone", HTTP_POST, webClockTimeZone); // ajax 处理时区表单
  server.on("/Read_clockTimeZone", webRead_clockTimeZone);      // ajax 请求当前的时区

  server.on("/clockLWJG", HTTP_POST, webClockLWJG); // ajax 处理时钟校准间隔表单
  server.on("/Read_clockLWJG", webRead_clockLWJG);  // ajax 请求当前的时钟校准间隔值

  server.on("/Put_ntpAdd", HTTP_POST, webPut_ntpAdd); // ajax ntp地址添加
  server.on("/Read_ntpAdd", webRead_ntpAdd);          // ajax 获取ntp地址

  server.on("/RESET", webRESET); // 处理重启请求

  // 配置http更新服务为为&server
  httpUpdater.setup(&server, "/update", "Jones", "3333");
  // 启动WebServer
  server.begin();
}

//回复状态码 200 给客户端
/*void respondOK()
  {
  server.send(200);
  }*/

void webNotFound() //设置处理404情况的函数'webUserRequest'
{
  //String path = server.uri();
  String uri;
  uri = WebServer::urlDecode(server.uri()); // 需要读取带有空格的路径
  if (handleFileRead(uri)) {
    return;
  }
  server.send(404, "text/html", sendHTML_main());
}

/*** 处理根目录get请求 ***/
void webRoot() //处理网站根目录"/"的访问请求
{
  String path;
  String contentType;
  String pathWithGz;
  path = F("/system/GCSBS.html");
  contentType = getContentType(path);  // 获取文件类型
  pathWithGz = path + ".gz";

  //Serial.print("pathWithGz:");Serial.println(pathWithGz);
  //Serial.print("contentType:");Serial.println(contentType);
  //Serial.print("path:");Serial.println(path);
  if (LittleFS.exists(pathWithGz)) // 是否有gz格式
  {
    File file = LittleFS.open(pathWithGz, "r"); // 则尝试打开该文件
    //Serial.print("file2:");Serial.println(file);
    server.streamFile(file, contentType); // 并且将该文件返回给浏览器
    file.close();                         // 并且关闭文件
  }
  else if (LittleFS.exists(path)) // 如果访问的文件可以在LittleFS中找到
  {
    File file = LittleFS.open(path, "r"); // 则尝试打开该文件
    //Serial.print("file1:");Serial.println(file);
    server.streamFile(file, contentType); // 并且将该文件返回给浏览器
    file.close();                         // 并且关闭文件
  }
  else server.send(200, "text/html", sendHTML_main());
}

size_t file_sy_zj;

// 处理上传文件函数
void webFileUpload()
{
  HTTPUpload& upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) //上传_文件_开始
  {
    file_sy_zj = LittleFS.totalBytes() - LittleFS.usedBytes(); //剩余空间，字节

    String uploadFileName = upload.filename; // 建立字符串变量用于存放上传文件名
    if (!uploadFileName.startsWith("/")) uploadFileName = "/" + uploadFileName;  // 确保路径始终以“/”开头
    Serial.println("文件名字: " + uploadFileName);
    Serial.println("文件系统剩余空间: " + String(file_sy_zj));
    //Serial.print("整个post请求的大小: ");  Serial.println(upload.contentLength);
    fsUploadFile = LittleFS.open(uploadFileName, "w+"); // 在LittleFS中建立文件用于写入用户上传的文件数据
    if (!fsUploadFile)
    {
      String xiaoxi = upload.filename + " 文件创建失败，可能名字太长";
      //server.send(500, "text/html", xiaoxi);
      promptWindowTransfer(xiaoxi, 1000); // 提示窗
      replyServerError(xiaoxi);
      server.handleClient();  //处理http请求
      delay(1000);
      reset_peiwang(); //重启至配网模式
    }
  }
  else if (upload.status == UPLOAD_FILE_WRITE) // 上传_文件_写入
  {
    //ESP.wdtFeed();
    yield();
    if (fsUploadFile)
    {
      //fsUploadFile.write(upload.buf, upload.currentSize); // 向LittleFS文件写入浏览器发来的文件数据
      size_t bytesWritten = fsUploadFile.write(upload.buf, upload.currentSize); //写入当前大小
      //Serial.println("upload.currentSize: " + String(upload.currentSize));
      //Serial.println("upload.totalSize: " + String(upload.totalSize)); //累积总大小
      if (upload.totalSize > file_sy_zj) //目前所上传的大小大于剩余空间
      {
        LittleFS.remove(upload.filename);
        String xiaoxi = upload.filename + " 上传失败，存储空间不足";
        //server.send(500, "text/html", xiaoxi);
        promptWindowTransfer(xiaoxi, 1000); // 提示窗
        delay(1000);
        //upload.status = UPLOAD_FILE_END;
        replyServerError(xiaoxi);
        server.handleClient();  //处理http请求
        reset_peiwang(); //重启至配网模式
      }
      if (bytesWritten != upload.currentSize)
      {
        LittleFS.remove(upload.filename);
        String xiaoxi = upload.filename + " 文件写入失败";
        //server.send(500, "text/html", xiaoxi);
        promptWindowTransfer(xiaoxi, 1000); // 提示窗
        delay(1000);
        //upload.status = UPLOAD_FILE_END;
        replyServerError(xiaoxi);
        server.handleClient();  //处理http请求
        reset_peiwang(); //重启至配网模式
      }
    }
  }
  else if (upload.status == UPLOAD_FILE_END) // 上传_文件_结束
  {
    if (fsUploadFile) // 如果文件成功建立
    {
      //uint32_t size1 = fsUploadFile.size(); //记录上传文件的大小
      fsUploadFile.close();// 将文件关闭
      //重新打开文件，并记录文件大小
      String uploadFileName;
      if (!upload.filename.startsWith("/")) uploadFileName = "/" + upload.filename;  // 确保路径始终以“/”开头
      fsUploadFile = LittleFS.open(uploadFileName, "r");
      uint32_t size1 = fsUploadFile.size(); //记录上传文件的大小
      //Serial.print("fsUploadFile:"); Serial.println(fsUploadFile);
      //Serial.print("文件名称："); Serial.println(uploadFileName);
      //Serial.print("文件大小："); Serial.println(size1);
      if (size1 == 0) //检查文件大小，等于0提示问题
      {
        LittleFS.remove(upload.filename);
        server.send(200, "text/plain", upload.filename + " 上传失败，空文件或存储空间不足");
        promptWindowTransfer(upload.filename + " 上传失败，空文件或存储空间不足", 1000); // 提示窗
      }
      else
      {
        server.send(200, "text/plain", upload.filename + " 上传成功");
        promptWindowTransfer(upload.filename + " 上传成功", 1000); // 提示窗
      }
    }
    else // 如果文件未能成功建立
    {
      Serial.println("文件上传失败");// 通过串口监视器输出报错信息
      server.send(500, "text/html", "上传失败，名称太长或有特殊符号");
      promptWindowTransfer(upload.filename + " 上传失败", 1000); // 提示窗
    }
  }
}
void webRead_fileSync() //文件系统剩余内存
{
  size_t file_sy_zj;
  file_sy_zj = LittleFS.totalBytes() - LittleFS.usedBytes(); //剩余空间，字节
  server.send(200, "text/plain", String(file_sy_zj));
}
/*** 处理根目录post请求 ***/
void webRootPost()
{
  String wifi_xiaoxi;
  bool err = 1;
  // wifi_xiaoxi = F("WIFI连接中，会断开热点");
  // char *strcpy(char *dest, const char *src) 把 src 所指向的字符串复制到 dest。
  // arg(name) —— 根据请求key获取请求参数的值
  // arg(index) —— 获取第几个请求参数的值
  /*Serial.print("WiFi表单-参数个数 "); Serial.println(server.args());
    Serial.print("WiFi表单-参数0名称 "); Serial.println(server.argName(0));
    Serial.print("WiFi表单-参数0数值 "); Serial.println(server.arg("plain"));*/
  if (server.hasArg("ssid")) // 查询是否存在某个参数
  {
    uint16_t length = server.arg("ssid").length();
    if (length == 0)
      wifi_xiaoxi = F("WIFI名称不能为空"); // 检查WIFI名称长度为0时退出
    else if (length > 32)
      wifi_xiaoxi = F("WIFI名称太长");
    else
    {
      eep_sta_ssid = server.arg("ssid"); // 保存参数
      err = 0;
    }
  }
  else
    wifi_xiaoxi = F("未找到参数'ssid'");

  // 查询是否存在某个参数
  if (server.hasArg("password"))
  {
    uint16_t length = server.arg("password").length();
    if (length > 64)
      wifi_xiaoxi += F("密码太长");
    else
    {
      eep_sta_password = server.arg("password"); // 保存参数
      err = 0;
    }
  }
  else
    wifi_xiaoxi = F("未找到参数'password'");

  if (err == 0)
  {
    wifi_xiaoxi = F("即将重启连接");
    eep.begin("eepUser", false);
    eep.putString("sta_ssid", eep_sta_ssid);         // wifi名称
    eep.putString("sta_password", eep_sta_password); // wifi密码
    eep.end();
    // 以重启的方式连接wifi
    //saveWifiToFlie(LittleFS, eep_sta_ssid, eep_sta_password); // 将wifi名称密码保存起来
    server.send(200, "text/plain", wifi_xiaoxi);
    server.handleClient(); // 处理http请求
    reset_peiwang();       // 重启至配网模式
  }
  else
  {
    server.send(200, "text/plain", wifi_xiaoxi);
    server.handleClient(); // 处理http请求
  }
}

void webRead_scanWifi() // 扫描周边wifi
{
  String web_message;
  int8_t n = 0;
  yield();
  n = WiFi.scanNetworks(); // 扫描wifi
  if (n > 0)
  {
    web_message = F("<label for='sel1'>周边网络:</label>");
    web_message += F("<select class='form-control' id='sel1' name='ssid'>");
    for (uint8_t i = 0; i < n; i++)
    {
      web_message += "<option>" + WiFi.SSID(i) + "</option>";
    }
    web_message += F("</select>");
  }
  else
  {
    web_message = F("<label for='sel1'>周边网络:</label>");
    web_message += F("<select class='form-control' id='sel1'>");
    web_message += F("<option >无扫描结果</option>");
    web_message += F("</select>");
  }
  WiFi.scanDelete(); // 删除扫描的结果
  yield();
  server.send(200, "text/plain", web_message);
}

void webRead_staNameIp() //ajax请求STA名字和ip
{
  String web_message = String();
  if (pw_state == 1)      web_message = F("WiFi未配置");
  else if (pw_state == 3)
  {
    web_message = WiFi.SSID(); web_message += F(" 连接失败");
  }
  else if (pw_state == 4) web_message = WiFi.SSID() + " " + WiFi.localIP().toString();
  server.send(200, "text/plain", web_message);
}

void webRead_info() //ajax请求系统信息
{
  String web_message = "";
  web_message = "程序版本：" + version + "<br>";
  web_message += "芯片ID：" + getEspChipId() + "<br>";
  web_message += F("芯片STA-MAC地址："); web_message += String(WiFi.macAddress().c_str()) + "<br>";
  web_message += F("芯片AP-MAC地址："); web_message += String(WiFi.softAPmacAddress().c_str()) + "<br>";
  web_message += F("可用堆大小："); web_message += String(ESP.getFreeHeap()) + "<br>";
  web_message += F("Sdk版本：");  web_message += String(ESP.getSdkVersion()) + "<br>";
  web_message += F("CPU运行频率："); web_message += String(ESP.getCpuFreqMHz()) + "MHZ" + "<br>";
  web_message += F("当前固件大小："); web_message += String(ESP.getSketchSize()) + "<br>";
  web_message += F("剩余可用固件空间："); web_message += String(ESP.getFreeSketchSpace()) + "<br>";
  //web_message += F("闪存芯片ID："); web_message += String(ESP.getFlashChipId()) + "<br>";
  web_message += F("闪存芯片大小："); web_message += String(ESP.getFlashChipSize()) + "<br>";
  web_message += F("闪存芯片运行频率："); web_message += String(ESP.getFlashChipSpeed()) + "<br><br>";
  web_message += F("<p style='font-size:0.8rem'><font color='red'>高级文件管理器涉及系统文件谨慎操作</font><p>");
  web_message += F("<p style='font-size:0.8rem'><a href='https://gitee.com/Lichengjiez'>甘草酸不酸</a><p>");
  server.send(200, "text/plain", web_message);
  promptWindowTransfer(F("请求系统信息"), 1000); // 提示窗
}

void webRead_clockTimeZone() //获取时区
{
  String web_message;
  String xiaoxi = "当前:" + String(eep_timeZone, 1);
  web_message = "<input type='text' class='form-control' placeholder='" + xiaoxi + "' name='ClockTimeZone' autocomplete='off'>";
  server.send(200, "text/plain", web_message);
}
void webClockTimeZone() //时区表单提交
{
  String web_message;
  //查询是否存在某个参数 ClockCompensate
  if (server.hasArg("ClockTimeZone")) //查询是否存在某个参数
  {
    if (server.arg("ClockTimeZone").length() != 0)  //检查名称长度为0时退出
    {
      float sum = atof(server.arg("ClockTimeZone").c_str()); //String 转 float
      if (eep_timeZone > 14 || eep_timeZone < -11) web_message = "没有" + String(eep_timeZone) + "区哦";
      else
      {
        eep_timeZone = sum;
        eep.begin("eepUser", false);
        eep.putFloat("timeZone", eep_timeZone); // 保存 时区
        eep.end();
        web_message = String(sum) + F("，保存成功");
      }
    }
    else web_message = F("未更改");
  }
  else web_message = F("ClockTimeZone 参数不存在");

  server.send(200, "text/plain", web_message);
  promptWindowTransfer("时区：" + String(eep_timeZone) + "保存成功", 1000); // 提示窗
}

void webRead_clockLWJG() //获取时钟校准间隔
{
  String web_message;
  String xiaoxi =  "当前:" + String(eep_clockLWJG) + "分钟";
  web_message = "<input type='text' class='form-control' placeholder='" + xiaoxi + "' name='clockLWJG' autocomplete='off'>";
  server.send(200, "text/plain", web_message);
}
void webClockLWJG() //时钟一言联网刷新间隔表单提交
{
  String web_message;
  //查询是否存在某个参数 ClockCompensate
  if (server.hasArg("clockLWJG")) //查询是否存在某个参数
  {
    if (server.arg("clockLWJG").length() != 0)  //检查名称长度为0时退出
    {
      uint32_t sum = atol(server.arg("clockLWJG").c_str()); //String 转 int
      if (sum < 15) web_message = F("最小为15");
      else if (sum > 255) web_message = F("最大为255");
      else
      {
        eep_clockLWJG = sum;
        eep.begin("eepUser", false);
        eep_clockLWJG = eep.getUInt("clockLWJG", 240);           // 时钟模式校准间隔 分钟
        eep.end();
        web_message = String(sum) + F("，保存成功");
      }
    }
    else web_message = F("未更改");
  }
  else web_message = F("clockLWJG 参数不存在");

  server.send(200, "text/plain", web_message);
  promptWindowTransfer("时钟校准间隔：" + String(eep_clockLWJG) + "保存成功", 1000); // 提示窗
}

void webRead_ntpAdd() //获取ntp地址
{
  String web_message;
  String xiaoxi =  eep_ntpAdd;
  web_message = F("<input type='text' class='form-control' placeholder='");
  web_message += xiaoxi;
  web_message += F("' name='ntpAdd' autocomplete='off'>");
  server.send(200, "text/plain", web_message);
}
void webPut_ntpAdd() //ntp地址 表单提交
{
  String web_message;
  //arg(name) —— 根据请求key获取请求参数的值
  //arg(index) —— 获取第几个请求参数的值
  /*Serial.print("SD卡频率-参数个数 "); Serial.println(server.args());
    Serial.print("SD卡频率-参数0名称 "); Serial.println(server.argName(0));
    Serial.print("SD卡频率-参数0数值 "); Serial.println(server.arg("plain"));*/
  //查询是否存在某个参数 ClockCompensate
  if (server.hasArg("ntpAdd")) //查询是否存在某个参数
  {
    if (server.arg("ntpAdd").length() == 0)  //检查名称长度为0时退出
    {
      eep_ntpAdd="";
      web_message = F("手动NTP已清除，使用内置NTP");
    }
    else if (server.arg("ntpAdd").length() > 32)
    {
      web_message = F("地址长度不能大于32");
    }
    else
    {
      eep_ntpAdd = server.arg("ntpAdd");
      web_message = "保存 " + String(eep_ntpAdd) + " 成功";
    }
    eep.begin("eepUser", false);
    eep.putString("ntpAdd", eep_ntpAdd); // 保存 手动NTP地址 
    eep.end();
  }
  else web_message = F("ntpAdd 参数不存在");

  server.send(200, "text/plain", web_message);

  promptWindowTransfer(web_message, 1000); // 提示窗
}

void webRESET() // 网页控制的系统重启
{
  String web_message = F("系统准备重启");
  promptWindowTransfer(web_message,1000);
  web_message = F("<meta charset = 'UTF-8'>系统重启，该页面已失效");
  server.send(200, "text/html", web_message);
  RTC_PW_rsert = 0; // 下次重启自动调跳转配网模式
  yield();
  esp_sleep(1000);
  // ESP.restart();
}
void reset_peiwang() // 重启至配网模式
{
  RTC_PW_rsert = 1;
  yield();
  esp_sleep(1000);
  // ESP.restart();
}

String sendHTML_main() //文件固件丢失提示界面，备用页面#include <u8g2_fonts.h>
{
  String webpage_html;
  webpage_html += F("<!DOCTYPE html>");
  webpage_html += F("<html lang='zh-CN'>");
  webpage_html += F("<head>");
  webpage_html += F("<title>mp3墨水屏</title>");
  webpage_html += F("<meta charset='utf-8'>");
  webpage_html += F("<meta name='viewport' content='width=device-width, initial-scale=1'>");
  webpage_html += F("</head>");
  webpage_html += F("<body>");
  webpage_html += F("<h3>糟糕！无法找到该页面，可能原因如下：</h3>");
  webpage_html += F("<p>1.文件系统未配置或网页丢失<br>2.文件系统不兼容，应为LittleFS<br>3.没有该页面<br>4.文件系统与程序固件不匹配<p><br>");
  webpage_html += F("<p>如需上传固件，请确保墨水屏和设备在同一WIFI网络下！</p>");
  webpage_html += F("<p>注意电量，以免在上传过程丢失固件！</p>");
  webpage_html += "<p>需要版本：" + version + "</p>";
  boolean ip = WiFi.localIP();
  if (ip)
  {
    webpage_html += "<p>http://" + WiFi.localIP().toString() + "/update</p>";
    webpage_html += "<a href=\"http://" + WiFi.localIP().toString() + "/update\">";
    webpage_html += F("<button type='button'>点击进入更新页面</button>");
    webpage_html += F("</a></body></html>");
  }
  else
  {
    webpage_html += "<p>http://" + WiFi.softAPIP().toString() + "/update</p>";
    webpage_html += "<a href=\"http://" + WiFi.softAPIP().toString() + "/update\">";
    webpage_html += F("<button type='button'>点击进入更新页面</button>");
    webpage_html += F("</a></body></html>");
  }
  return webpage_html;
}

// 获取文件类型
String getContentType(String filename)
{
  if (server.hasArg("download"))
    return "application/octet-stream";
  else if (filename.endsWith(".htm"))
    return "text/html";
  else if (filename.endsWith(".html"))
    return "text/html";
  else if (filename.endsWith(".css"))
    return "text/css";
  else if (filename.endsWith(".js"))
    return "application/javascript";
  else if (filename.endsWith(".png"))
    return "image/png";
  else if (filename.endsWith(".gif"))
    return "image/gif";
  else if (filename.endsWith(".jpg"))
    return "image/jpeg";
  else if (filename.endsWith(".ico"))
    return "image/x-icon";
  else if (filename.endsWith(".xml"))
    return "text/xml";
  else if (filename.endsWith(".pdf"))
    return "application/x-pdf";
  else if (filename.endsWith(".zip"))
    return "application/x-zip";
  else if (filename.endsWith(".gz")) 
    return "application/x-gzip";
  else if (filename.endsWith(".ttf"))
    return "application/font";
  return "text/plain";
}

#define systemNameNum  11 //系统文件的数量
//判断该文件是否是系统文件
boolean getSystemFile(String fileName) //文件名
{
  String systemName[systemNameNum] = {
    "/system/GCSBS.html.gz",
    "/system/ace.js.gz",
    "/system/bootstrap.css.gz",
    "/system/bootstrapBundle.js.gz",
    "/system/favicon.ico.gz",
    "/system/GCSBS.html.gz",
    "/system/iconfont.ttf",
    "/system/jquery.js.gz",
    "/system/manager.htm.gz",
    "/system/toast.css.gz",
    "/system/toast.js.gz",
  };

  for (uint8_t i = 0; i < systemNameNum; i++)
  {
    if (fileName == systemName[i]) return 1;
  }

  return 0;
}


//****** EDIT 文件管理器 SETUP******
void replyOK() {
  server.send(200, FPSTR(TEXT_PLAIN), "");
}
void replyOKWithMsg(String msg) {
  server.send(200, FPSTR(TEXT_PLAIN), msg);
}
void replyNotFound(String msg) {
  server.send(404, FPSTR(TEXT_PLAIN), msg);
}
void replyBadRequest(String msg) {
  //DBG_OUTPUT_PORT.println(msg);
  server.send(400, FPSTR(TEXT_PLAIN), msg + "\r\n");
}
void replyServerError(String msg) {
  //DBG_OUTPUT_PORT.println(msg);
  server.send(500, FPSTR(TEXT_PLAIN), msg + "\r\n");
}
////////////////////////////////
// 请求处理程序

bool existsEdit(String path)
{
  bool yes = false;
  File file = fileSystem.open(path, "r");
  if (!file) //文件不存在
    yes = false;
  else if (!file.isDirectory())  //文件不是文件夹
    yes = true;
  file.close();
  return yes;
}
// 检查文件是否存在
bool exists(fs::FS &fs, String path)
{
  bool yes = 0;
  File file = fs.open(path, "r");
  if (file == 0) // 文件不存在 返回0
    yes = 0;
  else if (!file.isDirectory()) // 不是文件夹 返回1
    yes = 1;
  file.close();
  return yes;
}
/*
   返回FS类型、状态和大小信息
*/
void handleStatus()
{
  boolean fsOK2 = serverFlieSDInit();

  //DBG_OUTPUT_PORT.println("handleStatus");

  String json;

  json = "{\"type\":\"";
  json += currentFsName;
  json += "\", \"isOk\":";
  if (fsOK2)
  {
    if (currentFsName == "LittleFS")
    {
      json += F("\"true\", \"totalBytes\":\"");
      json += LittleFS.totalBytes(); //总字节数
      json += F("\", \"usedBytes\":\"");
      json += LittleFS.usedBytes(); //已用字节
      json += "\"";
    }
  }
  else
  {
    json += "\"false\"";
  }
  json += F(",\"unsupportedFiles\":\"");
  json += unsupportedFiles;
  json += "\"}";

  server.send(200, "application/json", json);
}


/*
  返回“dir”查询字符串参数指定的目录中的文件列表。
*/
void handleFileList()
{
  boolean fsOK2 = serverFlieSDInit();

  if (!fsOK2) // 文件系统错误
  {
    return replyServerError(FPSTR(FS_INIT_ERROR));
  }

  if (!server.hasArg("dir"))
  {
    return replyBadRequest(F("DIR ARG丢失")); // DIR ARG丢失
  }

  String path = server.arg("dir");
  //DBG_OUTPUT_PORT.println("handleFileList: " + path);
  //DBG_OUTPUT_PORT.print("currentFsName:");DBG_OUTPUT_PORT.println(currentFsName);

  File root = fileSystem2.open(path);
  path = String();

  String output = "[";
  if (root.isDirectory())
  {
    File file = root.openNextFile();
    while (file)
    {
      if (output != "[")
      {
        output += ',';
      }
      output += "{\"type\":\"";
      output += (file.isDirectory()) ? "dir" : "file";
    
      output += "\",\"size\":\"";
      output += file.size();

      output += "\",\"name\":\"";
      //output += String(file.path()).substring(1); substring(1)表示去掉前面一个字符
      output += file.name();
      output += "\"}";
      file = root.openNextFile();
    }
  }
  output += "]";
  server.send(200, "text/json", output);
}

/*
   从文件系统读取给定的文件，并将其流回到客户端
*/
bool handleFileRead(String path)
{
  //DBG_OUTPUT_PORT.println();
  //DBG_OUTPUT_PORT.println(String("handleFileRead: ") + path);
  //DBG_OUTPUT_PORT.println(String("fsOK: ") + fsOK);
  if (!fsOK)
  {
    replyServerError(FPSTR(FS_INIT_ERROR));
    return true;
  }

  if (path.endsWith("/"))
  {
    path += "system/manager.htm";
  }

  //DBG_OUTPUT_PORT.print("handleFileRead:"); DBG_OUTPUT_PORT.println(path);

  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";

  //检测是不是系统文件,系统文件使用LittleFS ，不是系统文件看当前有无挂载SD卡fileSystem2

  fs::FS FSEdit = LittleFS;

  /*if (getSystemFile(pathWithGz) || getSystemFile(path)) // 是系统文件
  {
    //DBG_OUTPUT_PORT.println("是系统文件");
    FSEdit = LittleFS;
  }
  else // 不是是系统文件
  {
    //DBG_OUTPUT_PORT.println("不是系统文件");
    FSEdit = fileSystem2;
  } */

  if (FSEdit.exists(pathWithGz)) // 是否有gz格式
  {
    File file = FSEdit.open(pathWithGz, "r"); // 则尝试打开该文件
    // Serial.print("file2:");Serial.println(file);
    server.streamFile(file, contentType); // 并且将该文件返回给浏览器
    file.close();                         // 并且关闭文件
    return true;
  }
  else if (FSEdit.exists(path)) // 如果访问的文件可以在LittleFS中找到
  {
    File file = FSEdit.open(path, "r"); // 则尝试打开该文件
    // Serial.print("file1:");Serial.println(file);
    server.streamFile(file, contentType); // 并且将该文件返回给浏览器
    file.close();                         // 并且关闭文件
    return true;
  }

  return false;
}

/*
  由于某些FS（例如LittleFS）在删除最后一个子文件夹时会删除父文件夹，
  返回仍然存在的最近父级的路径
*/
String lastExistingParent(String path)
{
  boolean fsOK2 = serverFlieSDInit();
  while (!path.isEmpty() && !fileSystem2.exists(path))
  {
    if (path.lastIndexOf('/') > 0)
    {
      path = path.substring(0, path.lastIndexOf('/'));
    }
    else
    {
      path = String(); // 无斜杠=>顶部文件夹不存在
    }
  }
  // DBG_OUTPUT_PORT.println(String("Last existing parent: ") + path);
  return path;
}

/*
   处理新文件的创建/重命名
   Operation      | req.responseText
   ---------------+--------------------------------------------------------------
   Create file    | parent of created file
   Create folder  | parent of created folder
   Rename file    | parent of source file
   Move file      | parent of source file, or remaining ancestor
   Rename folder  | parent of source folder
   Move folder    | parent of source folder, or remaining ancestor
*/
void handleFileCreate()
{
  boolean fsOK2 = serverFlieSDInit();

  if (!fsOK2)
  {
    return replyServerError(FPSTR(FS_INIT_ERROR));
  }

  String path = server.arg("path");
  if (path.isEmpty())
  {
    return replyBadRequest(F("缺少路径参数"));
  }

  if (path == "/")
  {
    return replyBadRequest(F("路径名无效"));
  }
  if (fileSystem2.exists(path))
  {
    return replyBadRequest(F("路径下有同名文件"));
  }

  String src = server.arg("src");
  if (src.isEmpty())
  {
    // 未指定源：创建
    DBG_OUTPUT_PORT.println(String("handleFileCreate: ") + path);
    if (path.endsWith("/"))
    {
      // 创建文件夹
      path.remove(path.length() - 1);
      if (!fileSystem2.mkdir(path))
      {
        return replyServerError(F("文件夹创建失败"));
      }
    }
    else
    {
      // 创建一个文件
      File file = fileSystem2.open(path, "w");
      if (file)
      {
        //file.write(0);
        file.close();
      }
      else
      {
        return replyServerError(F("文件创建失败"));
      }
    }
    if (path.lastIndexOf('/') > -1)
    {
      path = path.substring(0, path.lastIndexOf('/'));
    }
    replyOKWithMsg(path);
  }
  else
  {
    // 指定的源：重命名
    if (src == "/")
    {
      return replyBadRequest("SRC无效");
    }
    if (!fileSystem2.exists(src))
    {
      return replyBadRequest(F("找不到SRC文件"));
    }

    // DBG_OUTPUT_PORT.println(String("handleFileCreate: ") + path + " from " + src);

    if (path.endsWith("/"))
    {
      path.remove(path.length() - 1);
    }
    if (src.endsWith("/"))
    {
      src.remove(src.length() - 1);
    }
    if (!fileSystem2.rename(src, path))
    {
      return replyServerError(F("重命名失败"));
    }
    replyOKWithMsg(lastExistingParent(src));
  }
}

/*
   处理文件删除请求
   Operation      | req.responseText
   ---------------+--------------------------------------------------------------
   Delete file    | parent of deleted file, or remaining ancestor
   Delete folder  | parent of deleted folder, or remaining ancestor
*/
void handleFileDelete()
{
  boolean fsOK2 = serverFlieSDInit();
  if (!fsOK2) 
  {
    return replyServerError(FPSTR(FS_INIT_ERROR));
  }
  if (server.args() == 0)
  {
    return server.send(500, "text/plain", "ARGS 不对");
  }
  String path = server.arg(0);
  if (path == "/")
  {
    return server.send(500, "text/plain", "PATH 不对");
  }
  /*if (!exists(fileSystem2,path))
  {
    return server.send(404, "text/plain", "文件不存在");
  }*/

  File root = fileSystem2.open(path, "r");
  if(!root) return server.send(404, "text/plain", "文件不存在");
  bool isDir = root.isDirectory();
  root.close();

  //DBG_OUTPUT_PORT.println(String("删除: ") + path);

  if (!isDir) // 如果是普通文件，请将其删除
  {
    // 在内部空间判断是不是系统文件，不让其删除
    if (currentFsName == "SD") fileSystem2.remove(path);
    else
    {
      if (getSystemFile(path) == 0) fileSystem2.remove(path);
      else replyServerError(F("系统文件不能删除"));
    }
  }
  else // 如果是文件夹，检测文件夹内无内容才能删除
  {
    File dir = fileSystem2.open(path);
    isDir = dir.isDirectory(); 
    if(isDir && dir.openNextFile()) replyServerError(F("文件夹内有文件时，是无法删除文件夹哒"));
    else fileSystem2.rmdir(path);
  }

  replyOKWithMsg(lastExistingParent(path));
}

/*
   处理文件上载请求
*/
boolean serverFlieSDInitUploadState = 1;
boolean fsOK3;
void handleFileUpload()
{
  if (serverFlieSDInitUploadState) 
  {
    serverFlieSDInitUploadState = 0;
    fsOK3 = serverFlieSDInit();
  }
  if (!fsOK3) {
    return replyServerError(FPSTR(FS_INIT_ERROR));
  }

  if (server.uri() != "/edit") {
    return;
  }

  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) //上传_文件_开始
  {
    String filename = upload.filename;
    // 确保路径始终以“/”开头
    if (!filename.startsWith("/"))
    {
      filename = "/" + filename;
    }
    //DBG_OUTPUT_PORT.println(String("handleFileUpload Name: ") + filename);
    //DBG_OUTPUT_PORT.println(String("currentFsName: ") + currentFsName);
    //DBG_OUTPUT_PORT.println(String("handleFileUpload totalSize: ") + upload.totalSize);
    fsUploadFile = fileSystem2.open(filename, "w+");
    if (!fsUploadFile)
    {
      //DBG_OUTPUT_PORT.println("创建失败");
      serverFlieSDInitUploadState = 1;
      return replyServerError(F("创建失败"));
    }
    //DBG_OUTPUT_PORT.println(String("上传开始, 名称: ") + filename);
    yield();
    promptWindowTransfer(String("上传中，") + filename, 2000); // 提示窗
  }
  else if (upload.status == UPLOAD_FILE_WRITE) //上传_文件_写入
  {
    if (fsUploadFile)
    {
      size_t bytesWritten = fsUploadFile.write(upload.buf, upload.currentSize); //写入当前大小
      if (bytesWritten != upload.currentSize)
      {
        serverFlieSDInitUploadState = 1;
        yield();
        promptWindowTransfer(String("上传失败，") + upload.filename, 2000); // 提示窗
        return replyServerError(F("写入失败"));
      }
    }
    //DBG_OUTPUT_PORT.println(String("Upload: WRITE, Bytes: ") + upload.currentSize);
  }
  else if (upload.status == UPLOAD_FILE_END) //上传_文件_结束
  {
    if (fsUploadFile)
    {
      serverFlieSDInitUploadState = 1;
      fsUploadFile.close();
    }
    yield();
    promptWindowTransfer(String("上传结束，") + upload.filename, 2000); // 提示窗
   // DBG_OUTPUT_PORT.println(String("上传结束，大小: ") + upload.totalSize);
  }
}

/*
  特定处理程序返回索引。/edit文件夹中的htm（或gzip版本）。
  如果文件不存在，但设置了“包含\回退\索引\ HTM”标志，则回退到版本
  嵌入在程序代码中。
  否则，将使用包含调试信息的404页面失败
*/
void handleGetEdit()
{
  if (handleFileRead(F("/system/manager.htm"))) return;
  replyNotFound(FPSTR(FILE_NOT_FOUND));
}

boolean serverFlieSDInit() // 网页文件管理器文件挂载
{
  //Serial.print("eep_sdState: ");Serial.println(eep_sdState);
  //Serial.print("sdInitOk: ");Serial.println(sdInitOk);
   if(currentFsName == "LittleFS") 
  {
    return 1;
  }
  return 0;
}
//****** EDIT 文件管理器 END ******