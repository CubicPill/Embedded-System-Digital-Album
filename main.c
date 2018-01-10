#include "sys.h"
#include "usart.h"		
#include "delay.h"	
#include "led.h"
#include "usmart.h" 
#include "lcd.h" 
#include "ov7670.h" 
#include "exti.h" 
#include "timer.h" 
//ALIENTEK Mini STM32��������չʵ��9
//����ͷʵ��
//����֧�֣�www.openedv.com
//������������ӿƼ����޹�˾  

extern u8 ov_sta;	//��exit.c���涨��
extern u8 ov_frame;	//��timer.c���涨��

//����LCD��ʾ
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
		LCD_Scan_Dir(U2D_L2R);		//���ϵ���,������ 
		LCD_SetCursor(0x00,0x0000);	//���ù��λ�� 
		// OV7670_Special_Effects(2);
		LCD_WriteRAM_Prepare();     //��ʼд��GRAM	
 		OV7670_CS=0;	 
 		OV7670_RRST=0;				//��ʼ��λ��ָ�� 
		OV7670_RCK=0;
		OV7670_RCK=1;
		OV7670_RCK=0;
		OV7670_RRST=1;				//��λ��ָ����� 
		OV7670_RCK=1;  
		for(j=0;j<76800;j++)
		{
			GPIOB->CRL=0X88888888;		   
			OV7670_RCK=0; 
			color=OV7670_DATA;		//������
			OV7670_RCK=1; 	
			color<<=8;					  
 			OV7670_RCK=0;
			color|=OV7670_DATA;		//������		  
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
		EXTI->PR=1<<15;     		//���LINE8�ϵ��жϱ�־λ
		ov_sta=0;					//��ʼ��һ�βɼ�
 		ov_frame++; 
		LCD_Scan_Dir(DFT_SCAN_DIR);	//�ָ�Ĭ��ɨ�跽�� 	  				 	 
	} 
}	  
 int main(void)
 {	
	delay_init();	    	 //��ʱ������ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// �����ж����ȼ�����2
	uart_init(9600);
	OV7670_Init();	
	LED_Init();		  		//��ʼ����LED���ӵ�Ӳ���ӿ�
	LCD_Init();			   	//��ʼ��LCD
	if(lcddev.id==0X6804||lcddev.id==0X5310||lcddev.id==0X5510||lcddev.id==0X1963) //ǿ��������Ļ�ֱ���Ϊ320*240.��֧��3.5�����
	{
		lcddev.width=240;
		lcddev.height=320; 
	}
	usmart_dev.init(72);	//��ʼ��USMART	
	
 	POINT_COLOR=RED;//��������Ϊ��ɫ 
  LCD_DrawRectangle(70,80,170,140);
  LCD_DrawRectangle(70,180,170,240);
	LCD_ShowString(100,100,100,16,16,"Album");
	LCD_ShowString(95,200,100,16,16,"Camera");
	while(OV7670_Init())//��ʼ��OV7670
	{
		LCD_ShowString(60,150,200,200,16,"OV7670 Error!!");
		delay_ms(200);
	    LCD_Fill(60,150,239,166,WHITE);
		delay_ms(200);
	}
 	//LCD_ShowString(60,150,200,200,16,"OV7670 Init OK");
	TIM3_Int_Init(10000,7199);			//TIM3,10Khz����Ƶ��,1�����ж�									  
	EXTI15_Init();						//ʹ�ܶ�ʱ������
	OV7670_Window_Set(10,174,240,320);	//���ô���	  
  OV7670_CS=0;						 	 
 	while(0){
	delay_ms(1000);
	}
	while(1)
	{	
 		camera_refresh();	//������ʾ	 
		
	}	   
}

