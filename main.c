#include "sys.h"
#include "usart.h"		
#include "delay.h"	
#include "led.h"
#include "usmart.h" 
#include "lcd.h" 
#include "ov7670.h" 
#include "exti.h" 
#include "timer.h" 
#include "malloc.h"  
#include "mmc_sd.h" 
#include "ff.h"  
#include "exfuns.h"
#include "camera.h"
#include "touch.h"
#include "album.h"
#include "bmp.h"
#include "piclib.h"
#include "string.h"

__inline void render_main_menu(void);
__inline void render_side_bar(void);
int pixel_count = 76800;

//�õ�path·����,Ŀ���ļ����ܸ���
//path:·��		    
//����ֵ:����Ч�ļ���
u16 pic_get_tnum(u8 *path)
{	  
	u8 res;
	u16 rval=0;
 	DIR tdir;	 		//��ʱĿ¼
	FILINFO tfileinfo;	//��ʱ�ļ���Ϣ	
	u8 *fn;	 			 			   			     
    res=f_opendir(&tdir,(const TCHAR*)path); 	//��Ŀ¼
  	tfileinfo.lfsize=_MAX_LFN*2+1;				//���ļ�����󳤶�
	tfileinfo.lfname=mymalloc(tfileinfo.lfsize);//Ϊ���ļ������������ڴ�
	if(res==FR_OK&&tfileinfo.lfname!=NULL)
	{
		while(1)//��ѯ�ܵ���Ч�ļ���
		{
	        res=f_readdir(&tdir,&tfileinfo);       		//��ȡĿ¼�µ�һ���ļ�
	        if(res!=FR_OK||tfileinfo.fname[0]==0)break;	//������/��ĩβ��,�˳�		  
     		fn=(u8*)(*tfileinfo.lfname?tfileinfo.lfname:tfileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X50)//ȡ����λ,�����ǲ���ͼƬ�ļ�	
			{
				rval++;//��Ч�ļ�������1
			}	    
		}  
	} 
	return rval;
}


int main(void) {
    DIR picdir;	 		//ͼƬĿ¼
    FILINFO picfileinfo;//�ļ���Ϣ
    u8 *fn;   			//���ļ���
	u8 *pname;			//��·�����ļ���
	u16 totpicnum; 		//ͼƬ�ļ�����
    u16 curindex;		//ͼƬ��ǰ����
    u8 pause=0;			//��ͣ���
	u16 *picindextbl;	//ͼƬ������
    u8 t;
    u16 temp;

    u16 mode;
	u8 res;
    u8 option = 0;
    delay_init(); //��ʱ������ʼ��
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // �����ж����ȼ�����2
    uart_init(9600);
    OV7670_Init();
    LED_Init(); //��ʼ����LED���ӵ�Ӳ���ӿ�
    LCD_Init(); //��ʼ��LCD

    tp_dev.init();

    usmart_dev.init(72); //��ʼ��USMART	
    POINT_COLOR = BLACK;
    while (OV7670_Init()) //��ʼ��OV7670
    {
        LCD_ShowString(60, 150, 200, 200, 16, "OV7670 Error!!");
        delay_ms(200);
        LCD_Fill(60, 150, 239, 166, WHITE);
        delay_ms(200);
    }
    while (SD_Initialize()) //���SD��
    {
        LCD_ShowString(60, 150, 200, 16, 16, "SD Card Error!");
        delay_ms(200);
        LCD_Fill(60, 150, 240, 150 + 16, WHITE); //�����ʾ			  
        delay_ms(200);
    }
    mem_init();		//��ʼ���ڲ��ڴ��
    exfuns_init();
    f_mount(fs, "0:", 1); //����SD��
    res=f_mkdir("0:/DCIM");
		if(res!=FR_EXIST&&res!=FR_OK) 	//�����˴���
	{		    
		LCD_Clear(WHITE);     
        LCD_ShowString(60, 150, 200, 16, 16, "Folder Error!");
		delay_ms(2000);
        return 0;		
	}
	
    EXTI15_Init(); //ʹ�ܶ�ʱ������
    OV7670_Window_Set(10, 174, 240, 320); //���ô���	  
    OV7670_CS = 0;

    INIT:
    BACK_COLOR=WHITE;
		render_main_menu();
    pixel_count = 76800;
    mode = 0;

    option = 0;

    while (!option) {
        tp_dev.scan(0);
        if (tp_dev.sta & TP_PRES_DOWN) {
            BACK_COLOR = LIGHTBLUE;
            if (tp_dev.x[0] <= 170 && tp_dev.x[0] >= 70 && tp_dev.y[0] <= 140 && tp_dev.y[0] >= 80) //������������
            {
                LCD_Fill(70, 80, 170, 140, LIGHTBLUE);
                LCD_ShowString(100, 100, 100, 16, 16, "Album");
                option = 1;

            } else if (tp_dev.x[0] <= 170 && tp_dev.x[0] >= 70 && tp_dev.y[0] <= 240 && tp_dev.y[0] >= 180) {
                option = 2;
                LCD_Fill(70, 180, 170, 240, LIGHTBLUE);
                LCD_ShowString(95, 200, 100, 16, 16, "Camera");
            }
        }
    }
    if (option == 2) {
        // camera
        delay_ms(500);
        while (1) {
            tp_dev.scan(0);
            if (tp_dev.sta & TP_PRES_DOWN) //������������
            {

                if (tp_dev.y[0] <= 100) { // go back
                    goto INIT;
                } else if (tp_dev.y[0] > 100 && tp_dev.y[0] < 220) {
                    //display filter selection
                    render_side_bar();

                    pixel_count = 57600;

                    // enter filter selection

                    while (1) {
                        tp_dev.scan(0);
                        if (tp_dev.sta & TP_PRES_DOWN) {
                            if (tp_dev.x[0] >= 180) {
                                if (tp_dev.y[0] <= 80) {
                                    LCD_Fill(180, 0, 240, 80, LIGHTBLUE);
                                    LCD_ShowChar(205, 30, 'N', 24, 1);
                                    mode = 0;
                                    break;
                                } else if (tp_dev.y[0] > 80 && tp_dev.y[0] <= 160) {
                                    LCD_Fill(180, 80, 240, 160, LIGHTBLUE);
                                    LCD_ShowChar(205, 110, 'R', 24, 1);
                                    mode = 1;
                                    break;
                                } else if (tp_dev.y[0] > 160 && tp_dev.y[0] <= 240) {
                                    LCD_Fill(180, 160, 240, 240, LIGHTBLUE);
                                    LCD_ShowChar(205, 190, 'G', 24, 1);
                                    mode = 2;
                                    break;
                                } else if (tp_dev.y[0] > 240) {
                                    LCD_Fill(180, 240, 240, 320, LIGHTBLUE);
                                    LCD_ShowChar(205, 270, 'B', 24, 1);
                                    mode = 3;
                                    break;
                                }

                            }
                        }
                        camera_refresh(mode, pixel_count);
                    }

                    pixel_count = 76800;

                    delay_ms(100);
                } else {
                    //take photo
									save_picture();
                }

            }
            camera_refresh(mode, pixel_count); //������ʾ	 
        }

    } else if (option == 1) {
        delay_ms(500);
		BACK_COLOR=WHITE;

        // album
        /*LCD_Clear(WHITE);
        LCD_ShowString(60, 150, 200, 16, 16, "Not implemented!");
        delay_ms(1000);
        goto INIT;*/
        while(f_opendir(&picdir,"0:/DCIM"))//��ͼƬ�ļ���
        {	    
           LCD_ShowString(60,170,240,16,16,"ͼƬ�ļ��д���!");
           delay_ms(200);				  
           LCD_Fill(60,170,240,186,WHITE);//�����ʾ	     
           delay_ms(200);				  
        }
        totpicnum=pic_get_tnum("0:/DCIM"); //�õ�����Ч�ļ���
        while(totpicnum==NULL)//ͼƬ�ļ�Ϊ0		
        {	    
           LCD_ShowString(60,170,240,16,16,"no picture file!");
           delay_ms(200);				  
           LCD_Fill(60,170,240,186,WHITE);//�����ʾ	     
           delay_ms(200);				  
        }
        picfileinfo.lfsize=_MAX_LFN*2+1;						//���ļ�����󳤶�
        picfileinfo.lfname=mymalloc(picfileinfo.lfsize);	//Ϊ���ļ������������ڴ�
        pname=mymalloc(picfileinfo.lfsize);				//Ϊ��·�����ļ��������ڴ�
        picindextbl=mymalloc(2*totpicnum);				//����2*totpicnum���ֽڵ��ڴ�,���ڴ��ͼƬ����
        while(picfileinfo.lfname==NULL||pname==NULL||picindextbl==NULL)//�ڴ�������
        {	    
           LCD_ShowString(60,170,240,16,16,"memory allocation failure!");
           delay_ms(200);				  
           LCD_Fill(60,170,240,186,WHITE);//�����ʾ	     
           delay_ms(200);				  
        }
        //��¼����
        res=f_opendir(&picdir,"0:/DCIM"); //��Ŀ¼
        if(res==FR_OK)
        {
            curindex=0;//��ǰ����Ϊ0
            while(1)//ȫ����ѯһ��
            {
                temp=picdir.index;								//��¼��ǰindex
                res=f_readdir(&picdir,&picfileinfo);       		//��ȡĿ¼�µ�һ���ļ�
                if(res!=FR_OK||picfileinfo.fname[0]==0)break;	//������/��ĩβ��,�˳�		  
                fn=(u8*)(*picfileinfo.lfname?picfileinfo.lfname:picfileinfo.fname);			 
                res=f_typetell(fn);	
                if((res&0XF0)==0X50)//ȡ����λ,�����ǲ���ͼƬ�ļ�	
                {
                    picindextbl[curindex]=temp;//��¼����
                    curindex++;
                }	    
            } 
        }
        delay_ms(1500);
        piclib_init();										//��ʼ����ͼ	   	   
        curindex=0;											//��0��ʼ��ʾ
        res=f_opendir(&picdir,(const TCHAR*)"0:/DCIM"); 	//��Ŀ¼
        while(res==FR_OK)//�򿪳ɹ�
        {	
            dir_sdi(&picdir,picindextbl[curindex]);			//�ı䵱ǰĿ¼����	   
            res=f_readdir(&picdir,&picfileinfo);       		//��ȡĿ¼�µ�һ���ļ�
            if(res!=FR_OK||picfileinfo.fname[0]==0)break;	//������/��ĩβ��,�˳�
            fn=(u8*)(*picfileinfo.lfname?picfileinfo.lfname:picfileinfo.fname);			 
            strcpy((char*)pname,"0:/DCIM/");				//����·��(Ŀ¼)
            strcat((char*)pname,(const char*)fn);  			//���ļ������ں���
            LCD_Clear(BLACK);
            ai_load_picfile(pname,0,0,lcddev.width,lcddev.height,1);//��ʾͼƬ    
            LCD_ShowString(2,2,240,16,16,pname); 				//��ʾͼƬ����
            t=0;			
            while(1) 
            {
				if(t>250){//��һ��ͼƬ
					curindex++;		   	
					if(curindex>=totpicnum)curindex=0;//��ĩβ��ʱ��,�Զ���ͷ��ʼ
					break;
				}
				if((t%20)==0)LED0=!LED0;//LED0��˸,��ʾ������������.
				tp_dev.scan(0);
				if(tp_dev.sta & TP_PRES_DOWN) {
					if(tp_dev.y[0] <= 50 && tp_dev.x[0] <= 50) // go back
					{ 
						goto INIT;
					}else if(tp_dev.x[0] <= 50 && tp_dev.y[0] >= lcddev.height-50) //�������½ǣ�ɾ������
					{
						f_unlink(pname);
						LCD_ShowString(60, 150, 200, 16, 16, "Deleted!");
						delay_ms(500);
						break;
					}else if(tp_dev.y[0] < 120) //�����ϲ��ֵ�1/3����һ��
					{
						if(curindex)curindex--;
						else curindex=totpicnum-1;
						break;
					}else if(tp_dev.y[0] > 120 && tp_dev.y[0]<240) //�����м��1/3����ͣ
					{
						pause=!pause;
						LED1=!pause;	//��ͣ��ʱ��LED1��.
					}else if(tp_dev.y[0]>240)  //���������1/3����һ��
					{
						curindex++;		   	
						if(curindex>=totpicnum)curindex=0;//��ĩβ��ʱ��,�Զ���ͷ��ʼ
						break;
					}
				}
				if(pause==0)t++;
				delay_ms(10);
            }					    
            res=0;  
        }
        myfree(picfileinfo.lfname);	//�ͷ��ڴ�			    
        myfree(pname);				//�ͷ��ڴ�			    
        myfree(picindextbl);		//�ͷ��ڴ�
    }
}

__inline void render_main_menu(void) {
    LCD_Clear(WHITE);
    POINT_COLOR = BLACK;
    LCD_DrawRectangle(70, 80, 170, 140);
    LCD_DrawRectangle(70, 180, 170, 240);
    LCD_ShowString(100, 100, 100, 16, 16, "Album");
    LCD_ShowString(95, 200, 100, 16, 16, "Camera");

}

__inline void render_side_bar(void) {
    POINT_COLOR = BLACK;

    LCD_Fill(180, 0, 240, 320, WHITE);
    LCD_ShowChar(205, 30, 'N', 24, 1);
    LCD_DrawLine(180, 80, 240, 80);

    LCD_ShowChar(205, 110, 'R', 24, 1);
    LCD_DrawLine(180, 160, 240, 160);

    LCD_ShowChar(205, 190, 'G', 24, 1);
    LCD_DrawLine(180, 240, 240, 240);

    LCD_ShowChar(205, 270, 'B', 24, 1);
}
