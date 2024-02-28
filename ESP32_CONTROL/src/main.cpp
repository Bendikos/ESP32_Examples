
//esp8266:scl-d1 sda-d2 up-d6 down-d5 enter-d7  beep-d4 led-d8 servo-rx tx sd2
//nano:scl-a5 sda-a4 up-2 down-3 enter-4

#include <U8g2lib.h>
#include <Wire.h>
#include <ESP32Servo.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 22, /* data=*/ 21);

Servo servo1;
Servo servo2;
Servo servo3;
//--------------------------nano
//#define key_up_pin 2
//#define key_down_pin 3
//#define key_enter_pin 4
//#define beep_pin 12
//#define led_pin 13
//-------------------------esp8266
#define key_up_pin 33
#define key_down_pin 25
#define key_enter_pin 26
#define beep_pin 5
#define led_pin 2
//---------------------------
#define key_up digitalRead(key_up_pin)
#define key_down digitalRead(key_down_pin)
#define key_enter digitalRead(key_enter_pin)
#define key_back digitalRead(key_back_pin)

#define BEEP(x) digitalWrite(15,x)


#define frame_line1 u8g2.drawRBox(0,1,127,16,3)     //以实色画第一行选择框  
#define frame_line2 u8g2.drawRBox(0,17,127,16,3)    //以实色画第二行选择框  
#define frame_line3 u8g2.drawRBox(0,33,127,16,3)    //以实色画第三行选择框  
#define frame_line4 u8g2.drawRBox(0,48,127,16,3)    //以实色画第四行选择框  

#define str_line1(x) u8g2.drawStr(14,14,x)    //在第一行写字
#define str_line2(x) u8g2.drawStr(14,30,x)    //在第二行写字
#define str_line3(x) u8g2.drawStr(14,46,x)    //在第三行写字
#define str_line4(x) u8g2.drawStr(14,61,x)    //在第四行写字

#define transparent_mode(x) u8g2.setFontMode(x)     //是否开启透明字体模式，0为默认值不开启透明字体，1开启
#define colour_mode(x) u8g2.setDrawColor(x);        //设置画笔颜色，0为背景透明色，1为实色

#define ON 1
#define OFF 0

#define longpresstime 60//长按时间60*10=600ms


enum
{
  _Main_UI=0,
  _SETTING_UI,
  _SERVO_UI,
  _SERVO_UI_CHILD1,
  _SERVO_UI_CHILD2,
  _SERVO_UI_CHILD3,
  _BEEP_Option,
  _Info_Option,
  _LED_Child,
  _SERVO_Child,
  _BEEP_Child,
  _Info_Child,
};

enum
{
  KEY_PREVIOUS=2,
  KEY_NEXT,
  KEY_ENTER, 
  KEY_BACK
};

int func_index=_Main_UI;//当前页面索引值
int last_index=_Main_UI;//上一个界面索引值
void (*current_operation_func)(int,int);//定义一个函数指针

 int cur_pos=1;
 int led_swi=1;
 float servo_angle1=90;
 float servo_angle2=90;
 float servo_angle3=90;
 int Bri_level=8;
 int BEEP_swi=ON;

typedef struct
{
 int Cur_Index;//当前索引项
  int previous;//上一页
  int next;//下一页
  int enter;//确认
  int back;//返回
  void (*current_operation)(int,int);// 当前索引执行的函数(界面)
}Main_Menu;

void Main_UI_Func(int page_index,int key_val);
void SETTING_UI_Func(int page_index,int key_val);
void SERVO_UI_Func(int page_index,int key_val);
void SERVO_CHILD_Func1(int page_index,int key_val);
void SERVO_CHILD_Func2(int page_index,int key_val);
void SERVO_CHILD_Func3(int page_index,int key_val);

Main_Menu table[10]=
{
  //Cur_Index,previous,next,enter,back,(*current_operation)(u8,u8)
  //主界面
  {_Main_UI,_Main_UI,_Main_UI,_SETTING_UI,_Main_UI,Main_UI_Func},
  //主菜单
  {_SETTING_UI,_SETTING_UI,_SETTING_UI,_SETTING_UI,_Main_UI,SETTING_UI_Func},//设置子菜单
  {_SERVO_UI,_SERVO_UI,_SERVO_UI,_SERVO_UI_CHILD1,_SETTING_UI,SERVO_UI_Func},//舵机号菜单
  {_SERVO_UI_CHILD1,_SERVO_UI_CHILD1,_SERVO_UI_CHILD1,_SERVO_UI_CHILD1,_SERVO_UI,SERVO_CHILD_Func1},//舵机1设置
  {_SERVO_UI_CHILD2,_SERVO_UI_CHILD2,_SERVO_UI_CHILD2,_SERVO_UI_CHILD2,_SERVO_UI,SERVO_CHILD_Func2},//舵机2设置
  {_SERVO_UI_CHILD3,_SERVO_UI_CHILD3,_SERVO_UI_CHILD3,_SERVO_UI_CHILD3,_SERVO_UI,SERVO_CHILD_Func3},//舵机3设置
};


void servo_init(){
  servo1.attach(4);
  servo2.attach(13);
  servo3.attach(15);
}

void servo_move(){
   servo1.write(servo_angle1);
   servo2.write(servo_angle2);
   servo3.write(servo_angle3);
}

void scifi_UI(){  
  u8g2.drawTriangle(5, 0, 0, 0, 0, 5);
  u8g2.drawLine(7, 0, 0, 7);
  u8g2.drawLine(7, 0, 116, 0);
  u8g2.drawLine(3, 7, 7, 3);
  u8g2.drawLine(7, 3, 33, 3);
  u8g2.drawLine(68, 1, 116, 1);
  u8g2.drawLine(116, 0, 128, 12);
  u8g2.drawLine(69, 2, 117, 2);
  u8g2.drawLine(118, 3, 124, 9);
  u8g2.drawLine(117, 3, 123, 9);
  u8g2.drawLine(3, 7, 3, 55);
  u8g2.drawLine(3, 55, 12, 64);
  u8g2.drawLine(0, 7, 0, 18);
  u8g2.drawLine(2, 26, 2, 41);
  u8g2.drawLine(1, 27, 1, 40);
  u8g2.drawLine(0, 48, 0, 56);
  u8g2.drawLine(0, 56, 6, 62);
  u8g2.drawLine(9, 57, 13, 61);
  u8g2.drawLine(13, 61, 25, 61);
  u8g2.drawLine(12, 63, 40, 63);
  u8g2.drawLine(41, 62, 44, 59);
  u8g2.drawLine(44, 59, 70, 59);
  u8g2.drawLine(41, 63, 67, 63);
  u8g2.drawLine(67, 63, 71, 59);
  u8g2.drawLine(71, 63 , 75, 59);
  u8g2.drawLine(76, 59, 112, 59);
  u8g2.drawLine(113, 59, 128, 44);
  u8g2.drawLine(127, 44, 127, 12);
  u8g2.drawLine(100, 63, 115, 63);
  u8g2.drawLine(115, 63, 127, 51);
  u8g2.drawTriangle(119, 63, 127, 55, 128, 64);  
}


void Main_UI_Func(int page_index,int key_val)
{
scifi_UI();  
u8g2.drawStr(18,34,"test system");  
}

void menu1(){
u8g2.setFont(u8g2_font_unifont_t_symbols);  
u8g2.setFontPosBaseline();

transparent_mode(1);
colour_mode(1);
frame_line1;
colour_mode(0);
led_swi==1?str_line1("LED:ON"):str_line1("LED:OFF");  
colour_mode(1);
str_line2("SERVO");
BEEP_swi==1?str_line3("BEEP:ON"):str_line3("BEEP:OFF"); 
str_line4("Device Info"); 
}

void menu2(){
transparent_mode(1);
colour_mode(1);
frame_line2;
colour_mode(0);  
str_line2("SERVO"); 
colour_mode(1);
led_swi==1?str_line1("LED:ON"):str_line1("LED:OFF");  
BEEP_swi==1?str_line3("BEEP:ON"):str_line3("BEEP:OFF"); 
str_line4("Device Info");
}

void menu3(){
transparent_mode(1);
colour_mode(1);
frame_line3;
colour_mode(0);
BEEP_swi==1?str_line3("BEEP:ON"):str_line3("BEEP:OFF");    
colour_mode(1);
led_swi==1?str_line1("LED:ON"):str_line1("LED:OFF");  
str_line2("SERVO"); 
str_line4("Device Info");
}

void menu4(){
transparent_mode(1);
colour_mode(1);
frame_line4;
colour_mode(0);
str_line4("Device Info");  
colour_mode(1);
led_swi==1?str_line1("LED:ON"):str_line1("LED:OFF");  
str_line2("SERVO");   
BEEP_swi==1?str_line3("BEEP:ON"):str_line3("BEEP:OFF"); 
}

void menu5(){
transparent_mode(1);
colour_mode(1);
frame_line1;
colour_mode(0);
str_line1("SERVO1");  
colour_mode(1);
str_line2("SERVO2");
str_line3("SERVO3"); 
}

void menu6(){
transparent_mode(1);
colour_mode(1);
frame_line2;
colour_mode(0);
str_line2("SERVO2");  
colour_mode(1);
str_line1("SERVO1");
str_line3("SERVO3"); 
}

void menu7(){
transparent_mode(1);
colour_mode(1);
frame_line3;
colour_mode(0);
str_line3("SERVO3");  
colour_mode(1);
str_line1("SERVO1");
str_line2("SERVO2"); 
}

void SETTING_UI_Func(int page_index,int key_val)
{
  if(last_index==_SETTING_UI)//判断是否是第一次进入此界面,如果等于，说明不是第一次进入该页面
  {
    switch(key_val)
    {
      case KEY_PREVIOUS: cur_pos==1?cur_pos=4:cur_pos--;
          break;
      case KEY_ENTER://确定(设置)按键
      {
        switch(cur_pos)
        {
          case 1:led_swi=!led_swi;
              break;
          case 2:
            func_index=_SERVO_UI;
              break;
          case 3:BEEP_swi=!BEEP_swi;
              break;
          case 4://BEEP_swi=!BEEP_swi;
              break;

          default:break;
        }
      }
          break;
      case KEY_NEXT:cur_pos==4?cur_pos=1:cur_pos++;
          break;
      default:break;
    }
  
  }
  else cur_pos=1;//第一次进入此界面,此瞬间不做操作，防止“使得进入该页面的按击”对该页面造成影响
  
  switch(cur_pos)
    {
      case 1:   
     menu1();
      break;
      
      case 2:
     menu2();
      break;  
      
      
      case 3:
     menu3();
      break;

      case 4:
     menu4();
      break;      
    }
  }

void SERVO_UI_Func(int page_index,int key_val){
  if(last_index==_SERVO_UI)//判断是否是第一次进入此界面,如果等于，说明不是第一次进入该页面
  {
    switch(key_val)
    {
      case KEY_PREVIOUS: cur_pos==1?cur_pos=3:cur_pos--;
          break;
      case KEY_ENTER://确定(设置)按键
      {
        switch(cur_pos)
        {
          case 1:func_index=_SERVO_UI_CHILD1;
              break;
          case 2:func_index=_SERVO_UI_CHILD2;            
              break;
          case 3:func_index=_SERVO_UI_CHILD3;
              break;
//          case 4: ;
//              break;

          default:break;
        }
      }
          break;
      case KEY_NEXT:cur_pos==3?cur_pos=1:cur_pos++;
          break;
      default:break;
    }
  
  }
  else cur_pos=1;//第一次进入此界面,此瞬间不做操作，防止“使得进入该页面的按击”对该页面造成影响
  
  switch(cur_pos)
    {
      case 1:   
     menu5();
      break;
      
      case 2:
     menu6();
      break;  
      
      
      case 3:
     menu7();
      break;

//      case 4:
//     menu4();
//      break;      
    }
}

void SERVO_CHILD_Func1(int page_index,int key_val)
{
scifi_UI();  
  if(servo_angle1>180){servo_angle1=0;}
  if(servo_angle1<0){servo_angle1=180;}
    u8g2.setCursor(55, 49);
    u8g2.print(servo_angle1); 
    u8g2.drawFrame(14,18,100,15);//圆角矩形
    u8g2.drawBox(14,20,int(servo_angle1/1.8),11);//圆角填充框矩形框   
}

void SERVO_CHILD_Func2(int page_index,int key_val)
{
scifi_UI();  
  if(servo_angle2>180){servo_angle2=0;}
  if(servo_angle2<0){servo_angle2=180;}
    u8g2.setCursor(55, 49);
    u8g2.print(servo_angle2); 
    u8g2.drawFrame(14,18,100,15);//圆角矩形
    u8g2.drawBox(14,20,int(servo_angle2/1.8),11);//圆角填充框矩形框   
}

void SERVO_CHILD_Func3(int page_index,int key_val)
{
scifi_UI();  
  if(servo_angle3>180){servo_angle3=0;}
  if(servo_angle3<0){servo_angle3=180;}
    u8g2.setCursor(55, 49);
    u8g2.print(servo_angle3); 
    u8g2.drawFrame(14,18,100,15);//圆角矩形
    u8g2.drawBox(14,20,int(servo_angle3/1.8),11);//圆角填充框矩形框   
}

void led_init(){
pinMode(led_pin,OUTPUT); 
}

void led_write(int i){
  digitalWrite(led_pin,i);
}

void key_init(){
  pinMode(key_up_pin,INPUT_PULLUP);
  pinMode(key_down_pin,INPUT_PULLUP);
  pinMode(key_enter_pin,INPUT_PULLUP);

}


int get_keyvalue(){//支持连按
 int c;
 static int flag=0;//长按标志
 if((!key_up||!key_down||!key_enter))
 {
  c=0;
    delay(20);//按键消抖
    if(key_up==0)return 2;
    if(key_down==0)return 3;
    if(key_enter==0){
      while(key_enter==0&&c<longpresstime){
        c++;
        delay(10);
      }
      if(c>=longpresstime){
        flag=1;//长按后标志置1
        while(key_enter==0) {return 5;}                
        }
       else if(flag==0){return 4;}  //防止长按后会return一个4，所以使用flag判断       
      }
  }
  flag=0;//按完一次按键后flag归0
  return 0;
}

void beep_init() {
  pinMode(beep_pin, OUTPUT);
}

 
void setup() {
  u8g2.begin();
  u8g2.setFont(u8g2_font_koleeko_tf);
  //Serial.begin(9600);
  key_init();
  beep_init();
  led_init();
  servo_init();
}
 
void loop() {

int key_val=get_keyvalue();
//Serial.println(key_val);
  if(key_val!=0)//只有按键按下才刷屏
  {    
    last_index=func_index;//按下按钮及更新上一界面索引值，此时由于索引值没有变，实际不更新，等程序循环一次回来后才更新，这样可以防止按下按键更新屏幕后按键瞬间对新屏幕产生影响
    switch(key_val)
    {
      case KEY_PREVIOUS:if(func_index==0||func_index==1){func_index=table[func_index].previous;}//更新索引值
                        else if(func_index==3){servo_angle1-=5;}
                        else if(func_index==4){servo_angle2-=5;}
                        else if(func_index==5){servo_angle3-=5;}
                        
          break;
      case KEY_ENTER:if(func_index==0||func_index==1){func_index=table[func_index].enter;}//在前两个页面更新索引值，在舵机号页面enter由子函数SERVO_UI_Func控制，不受数组控制                    
          break;
      case KEY_NEXT:if(func_index==0||func_index==1){func_index=table[func_index].next;}//更新索引值
                        else if(func_index==3){servo_angle1+=5;}
                        else if(func_index==4){servo_angle2+=5;}
                        else if(func_index==5){servo_angle3+=5;}
          break;
      case KEY_BACK:func_index=table[func_index].back;//更新索引值
          break;
      default:break;
    }
    if(BEEP_swi==ON)
    {
      BEEP(1);
      delay(50);
      BEEP(0);
    }
  }
  current_operation_func=table[func_index].current_operation;
  u8g2.clearBuffer();
  (*current_operation_func)(func_index,key_val);//执行当前索引对应的函数
  u8g2.sendBuffer(); 
  led_write(led_swi); 
  servo_move();
  }
  
 
 
