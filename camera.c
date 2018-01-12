#include "camera.h"
#include "ov7670.h" 
#include "lcd.h"
#include "exti.h"

extern u8 ov_sta;  //��exit.c���涨��
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
      color=OV7670_DATA;    //������
      OV7670_RCK=1;   
      color<<=8;            
       OV7670_RCK=0;
      color|=OV7670_DATA;    //������      
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
    LCD_Scan_Dir(U2D_L2R);    //���ϵ���,������ 
    LCD_SetCursor(0x00,0x0000);  //���ù��λ�� 
    LCD_WriteRAM_Prepare();     //��ʼд��GRAM  
     OV7670_CS=0;   
     OV7670_RRST=0;        //��ʼ��λ��ָ�� 
    OV7670_RCK=0;
    OV7670_RCK=1;
    OV7670_RCK=0;
    OV7670_RRST=1;        //��λ��ָ����� 
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
    EXTI->PR=1<<15;         //���LINE8�ϵ��жϱ�־λ
    ov_sta=0;          //��ʼ��һ�βɼ�
    LCD_Scan_Dir(DFT_SCAN_DIR);  //�ָ�Ĭ��ɨ�跽��                 
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



