#include "sys.h"
#include "usart.h"		
#include "delay.h"	
#include "led.h"
#include "usmart.h" 
#include "lcd.h" 
#include "ov7670.h" 
#include "exti.h" 
#include "timer.h" 
//ALIENTEK Mini STM32开发板扩展实验9
//摄像头实验
//技术支持：www.openedv.com
//广州市星翼电子科技有限公司  

extern u8 ov_sta;	//在exit.c里面定义
extern u8 ov_frame;	//在timer.c里面定义

//更新LCD显示
void camera_refresh(void)
{
	u32 j;
 	u16 color;	
  u16 r;
  u16 g;
  u16 b;	
	u16 gray;
	if(ov_sta==2)
	{
		LCD_Scan_Dir(U2D_L2R);		//从上到下,从左到右 
		LCD_SetCursor(0x00,0x0000);	//设置光标位置 
		// OV7670_Special_Effects(2);
		LCD_WriteRAM_Prepare();     //开始写入GRAM	
 		OV7670_CS=0;	 
 		OV7670_RRST=0;				//开始复位读指针 
		OV7670_RCK=0;
		OV7670_RCK=1;
		OV7670_RCK=0;
		OV7670_RRST=1;				//复位读指针结束 
		OV7670_RCK=1;  
		for(j=0;j<76800;j++)
		{
			GPIOB->CRL=0X88888888;		   
			OV7670_RCK=0; 
			color=OV7670_DATA;		//读数据
			OV7670_RCK=1; 	
			color<<=8;					  
 			OV7670_RCK=0;
			color|=OV7670_DATA;		//读数据		  
			OV7670_RCK=1; 
			GPIOB->CRL=0X33333333;
		  r=(color&0xf800)>>11;
			g=(color&0x07e0)>>6;
			b=color&0x0010;
			gray=(r>>2)+(g>>2)+(b>>2);
			gray=(gray>0x8)?0xffff:0x0;
			r=gray;
			g=gray;
			b=gray;
			color=(r<<11)|(g<<6)|b;
			LCD_WR_DATA(color);	 
		}  
 		OV7670_CS=1; 							 
		OV7670_RCK=0; 
		OV7670_RCK=1; 
		EXTI->PR=1<<15;     		//清除LINE8上的中断标志位
		ov_sta=0;					//开始下一次采集
 		ov_frame++; 
		LCD_Scan_Dir(DFT_SCAN_DIR);	//恢复默认扫描方向 	  				 	 
	} 
}	  
 int main(void)
 {	
	delay_init();	    	 //延时函数初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// 设置中断优先级分组2
	uart_init(9600);
	OV7670_Init();	
	LED_Init();		  		//初始化与LED连接的硬件接口
	LCD_Init();			   	//初始化LCD
	if(lcddev.id==0X6804||lcddev.id==0X5310||lcddev.id==0X5510||lcddev.id==0X1963) //强制设置屏幕分辨率为320*240.以支持3.5寸大屏
	{
		lcddev.width=240;
		lcddev.height=320; 
	}
	usmart_dev.init(72);	//初始化USMART	
	
 	POINT_COLOR=RED;//设置字体为红色 
  LCD_DrawRectangle(70,80,170,140);
  LCD_DrawRectangle(70,180,170,240);
	LCD_ShowString(100,100,100,16,16,"Album");
	LCD_ShowString(95,200,100,16,16,"Camera");
	while(OV7670_Init())//初始化OV7670
	{
		LCD_ShowString(60,150,200,200,16,"OV7670 Error!!");
		delay_ms(200);
	    LCD_Fill(60,150,239,166,WHITE);
		delay_ms(200);
	}
 	//LCD_ShowString(60,150,200,200,16,"OV7670 Init OK");
	TIM3_Int_Init(10000,7199);			//TIM3,10Khz计数频率,1秒钟中断									  
	EXTI15_Init();						//使能定时器捕获
	OV7670_Window_Set(10,174,240,320);	//设置窗口	  
  OV7670_CS=0;						 	 
 	while(0){
	delay_ms(1000);
	}
	while(1)
	{	
 		camera_refresh();	//更新显示	 
		
	}	   
}

