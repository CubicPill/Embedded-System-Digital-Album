#include "camera.h"
#include "ov7670.h" 
#include "lcd.h"
#include "exti.h"

extern u8 ov_sta;  //在exit.c里面定义
u8 THRESHOLD=0x10;
const u8 MODE_NONE=0;
const u8 MODE_REVERT=1;
const u8 MODE_GRAY=2;
const u8 MODE_BIN=3;
void save_picture(){}
  
__inline u16 read_data(void){
  u16 color;
GPIOB->CRL=0X88888888;       
      OV7670_RCK=0; 
      color=OV7670_DATA;    //读数据
      OV7670_RCK=1;   
      color<<=8;            
       OV7670_RCK=0;
      color|=OV7670_DATA;    //读数据      
      OV7670_RCK=1; 
      GPIOB->CRL=0X33333333;
  return color;
}

void camera_refresh(u8 mode, int fill_pixels_count)
{
  u32 j;
   u16 color;  
  if(ov_sta==2)
  {
    LCD_Scan_Dir(U2D_L2R);    //从上到下,从左到右 
    LCD_SetCursor(0x00,0x0000);  //设置光标位置 
    LCD_WriteRAM_Prepare();     //开始写入GRAM  
     OV7670_CS=0;   
     OV7670_RRST=0;        //开始复位读指针 
    OV7670_RCK=0;
    OV7670_RCK=1;
    OV7670_RCK=0;
    OV7670_RRST=1;        //复位读指针结束 
    OV7670_RCK=1;  
    for(j=0;j<fill_pixels_count;j++)
    {
      color=read_data();
      switch (mode){
        case MODE_NONE:
          break;
        case MODE_REVERT:
          color=~color;
        break;
        case MODE_BIN:
          color=binarization(color);
        break;
        case MODE_GRAY:
          color=gray_scale(color);
        break;
      }
      LCD_WR_DATA(color);   
    }  
     OV7670_CS=1;                
    OV7670_RCK=0; 
    OV7670_RCK=1; 
    EXTI->PR=1<<15;         //清除LINE8上的中断标志位
    ov_sta=0;          //开始下一次采集
    LCD_Scan_Dir(DFT_SCAN_DIR);  //恢复默认扫描方向                 
  } 
}   

__inline u16 gray_scale(u16 color){
  u16 r,g,b,gray;
r=(color&0xf800)>>11;
      g=(color&0x07e0)>>6;
      b=color&0x0010;
      gray =(r*28+g*151+b*77)>>8;
  color=(gray<<11)|(gray<<6)|gray;
      return color;
      
}

__inline u16 binarization(u16 color){
  u16 r,g,b,gray;
r=(color&0xf800)>>11;
      g=(color&0x07e0)>>6;
      b=color&0x0010;
      gray =(r*28+g*151+b*77)>>8;
   color=(gray>THRESHOLD)?0xffff:0x0;
        return color;

}



