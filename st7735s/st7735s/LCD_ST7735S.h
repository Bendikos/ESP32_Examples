#ifndef _LCD_ST7735S_H_
#define _LCD_ST7735S_H_

#define TAG "spi_example"

// SPI 引脚定义
#define PIN_NUM_MISO 19
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK 18
#define PIN_NUM_CS_1 5 // cs
#define LCD_PIN_RES 32 // reset
#define LCD_PIN_DC 33  // data or cmd
#define LCD_PIN_BLK 25 // backlight

#define EXAMPLE_MAX_CHAR_SIZE 64

#define WHITE 0xFFFF
#define BLACK 0x0000
#define BLUE 0x001F
#define BRED 0XF81F
#define GRED 0XFFE0
#define GBLUE 0X07FF
#define RED 0xF800
#define MAGENTA 0xF81F
#define GREEN 0x07E0
#define CYAN 0x7FFF
#define YELLOW 0xFFE0
#define BROWN 0XBC40     // 棕色
#define BRRED 0XFC07     // 棕红色
#define GRAY 0X8430      // 灰色
#define DARKBLUE 0X01CF  // 深蓝色
#define LIGHTBLUE 0X7D7C // 浅蓝色
#define GRAYBLUE 0X5458  // 灰蓝色

void lcdGpioInit(void);
void lcdInit(void);
void lcdSelectRegister(unsigned char data);
void lcdWriteDataU8(unsigned char data);
void lcdWriteDataU16(unsigned short data);

void lcdSetAddress(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2);
void lcdClear(unsigned short color);

void LCD_DrawPoint(unsigned short x, unsigned short y, unsigned short color);

void Lcd_Init(void);
void LCD_Clear(unsigned short color);

void LCD_DisplayOn(void);                                                                          // 开显示
void LCD_DisplayOff(void);                                                                         // 关显示
void LCD_Draw_Circle(unsigned short x0, unsigned short y0, unsigned char r, unsigned short color); // 画圆
void LCD_DrawFullCircle(unsigned short Xpos, unsigned short Ypos, unsigned short Radius, unsigned short Color);
void LCD_Fill(unsigned short sx, unsigned short sy, unsigned short ex, unsigned short ey, unsigned short color);                                                                                 // 填充区域
void LCD_DrawLine(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, unsigned short color);                                                                             // 画线
void LCD_DrawRectangle(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, unsigned short color);                                                                        // 画矩形
void LCD_ShowChar(unsigned short x, unsigned short y, unsigned char num, unsigned char size, unsigned char mode, unsigned short pen_color, unsigned short back_color);                           // 写ASCII字符
void LCD_ShowString(unsigned short x, unsigned short y, unsigned short width, unsigned short height, unsigned char size, unsigned char *p, unsigned short pen_color, unsigned short back_color); // 写ASCII字符串

void GUI_sprintf_hz1616(unsigned short x, unsigned short y, unsigned char c[3], unsigned short pen_color, unsigned short back_color);
void GUI_sprintf_hz16x(unsigned short x1, unsigned short y1, unsigned char *str, unsigned short dcolor, unsigned short bgcolor);

void GUI_sprintf_hz3232(unsigned short x, unsigned short y, unsigned char c[3], unsigned short pen_color, unsigned short back_color);
void GUI_sprintf_hz32x(unsigned short x1, unsigned short y1, unsigned char *str, unsigned short pen_color, unsigned short back_color);

#endif
