#include <TFT_eSPI.h>
#include "common.h"
#include "PreferencesUtil.h"

TFT_eSPI tft = TFT_eSPI(); 
TFT_eSprite clk = TFT_eSprite(&tft);
int backColor;
uint16_t backFillColor;
uint16_t penColor;

// 初始化tft
void tftInit(){
  tft.init();
  //tft.setSwapBytes(true);
  getBackColor();
  if(backColor == BACK_BLACK){
    backFillColor = 0x0000;
    penColor = 0xFFFF;
  }else{
    backFillColor = 0xFFFF;
    penColor = 0x0000;
  }
  tft.fillScreen(backFillColor);
}

// 按背景颜色刷新整个屏幕
void reflashTFT(){
  tft.fillScreen(backFillColor);
}

// 在屏幕中间显示文字
void drawText(String text){
  clk.setColorDepth(8);
  clk.setTextDatum(CC_DATUM);
  //clk.loadFont(clock_tips_28);
  clk.loadFont(HarmonyOS_Sans_SC_Medium16);
  clk.createSprite(128, 160); 
  clk.setTextColor(penColor);
  clk.fillSprite(backFillColor);
  clk.drawString(text, 128 / 2, 160 / 2);
  clk.pushSprite(0,0);
  clk.deleteSprite();
  clk.unloadFont(); 
}

// 在屏幕中间显示两行文字
void draw2LineText(String text1, String text2){
  clk.setColorDepth(8);
  clk.setTextDatum(CC_DATUM);
  // clk.loadFont(clock_tips_28);
  clk.loadFont(HarmonyOS_Sans_SC_Medium16);
  clk.createSprite(128, 160);
  clk.setTextColor(penColor);
  clk.fillSprite(backFillColor);
  clk.drawString(text1, 128 / 2, 160 / 2 - 10);
  clk.drawString(text2, 128 / 2, 160 / 2 + 10);
  clk.pushSprite(0,0);
  clk.deleteSprite();
  clk.unloadFont(); 
}