#include "sys.h"
#include "usart.h"		
#include "delay.h"	
#include "led.h"
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

void render_main_menu(void);
int pixel_count = 76800;

//�õ�path·����,Ŀ���ļ����ܸ���
//path:·��		    
//����ֵ:����Ч�ļ���


int main(void) {
    DIR picdir; //ͼƬĿ¼
    FILINFO picfileinfo; //�ļ���Ϣ
    u8 * fn; //���ļ���
    u8 * pname; //��·�����ļ���
    u16 totpicnum; //ͼƬ�ļ�����
    u16 curindex; //ͼƬ��ǰ����
    u8 pause = 0; //��ͣ���
    u16 * picindextbl; //ͼƬ������
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
    mem_init(); //��ʼ���ڲ��ڴ��
    exfuns_init();
    f_mount(fs, "0:", 1); //����SD��
    res = f_mkdir("0:/DCIM");
    if (res != FR_EXIST && res != FR_OK) //�����˴���
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
    BACK_COLOR = WHITE;
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
                                if (set_camera_mode(tp_dev.y[0], & mode)) {
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
        //entering album
        delay_ms(500);
        BACK_COLOR = WHITE;

        while (f_opendir( & picdir, "0:/DCIM")) //��ͼƬ�ļ���
        {
            LCD_ShowString(60, 170, 240, 16, 16, "No picture folder!");
            delay_ms(200);
            LCD_Fill(60, 170, 240, 186, WHITE); //�����ʾ	     
            delay_ms(200);
        }
        totpicnum = pic_get_tnum("0:/DCIM"); //�õ�����Ч�ļ���
        while (totpicnum == NULL) //ͼƬ�ļ�Ϊ0		
        {
          LCD_Clear(WHITE); //�����ʾ	    
					LCD_ShowString(60, 170, 240, 16, 16, "No picture file!");
            delay_ms(400);
               
        }
        picfileinfo.lfsize = _MAX_LFN * 2 + 1; //���ļ�����󳤶�
        picfileinfo.lfname = mymalloc(picfileinfo.lfsize); //Ϊ���ļ������������ڴ�
        pname = mymalloc(picfileinfo.lfsize); //Ϊ��·�����ļ��������ڴ�
        picindextbl = mymalloc(2 * totpicnum); //����2*totpicnum���ֽڵ��ڴ�,���ڴ��ͼƬ����
        while (picfileinfo.lfname == NULL || pname == NULL || picindextbl == NULL) //�ڴ�������
        {
					          LCD_Clear(WHITE); //�����ʾ	    

            LCD_ShowString(60, 170, 240, 16, 16, "Memory allocation failure!");
            delay_ms(400);
            
        }
        //��¼����
        res = f_opendir( & picdir, "0:/DCIM"); //��Ŀ¼
        if (res == FR_OK) {
            curindex = 0; //��ǰ����Ϊ0
            while (1) //ȫ����ѯһ��
            {
                temp = picdir.index; //��¼��ǰindex
                res = f_readdir( & picdir, & picfileinfo); //��ȡĿ¼�µ�һ���ļ�
                if (res != FR_OK || picfileinfo.fname[0] == 0) break; //������/��ĩβ��,�˳�		  
                fn = (u8 * )( * picfileinfo.lfname ? picfileinfo.lfname : picfileinfo.fname);
                res = f_typetell(fn);
                if ((res & 0XF0) == 0X50) //ȡ����λ,�����ǲ���ͼƬ�ļ�	
                {
                    picindextbl[curindex] = temp; //��¼����
                    curindex++;
                }
            }
        }
        delay_ms(1500);
        piclib_init(); //��ʼ����ͼ	   	   
        curindex = 0; //��0��ʼ��ʾ
        res = f_opendir( & picdir, (const TCHAR * )
            "0:/DCIM"); //��Ŀ¼
        while (res == FR_OK) //�򿪳ɹ�
        {
            dir_sdi( & picdir, picindextbl[curindex]); //�ı䵱ǰĿ¼����	   
            res = f_readdir( & picdir, & picfileinfo); //��ȡĿ¼�µ�һ���ļ�
            if (res != FR_OK || picfileinfo.fname[0] == 0) break; //������/��ĩβ��,�˳�
            fn = (u8 * )( * picfileinfo.lfname ? picfileinfo.lfname : picfileinfo.fname);
            strcpy((char * ) pname, "0:/DCIM/"); //����·��(Ŀ¼)
            strcat((char * ) pname, (const char * ) fn); //���ļ������ں���
            LCD_Clear(BLACK);
            ai_load_picfile(pname, 0, 0, lcddev.width, lcddev.height, 1); //��ʾͼƬ    
            LCD_ShowString(2, 2, 240, 16, 16, pname); //��ʾͼƬ����
            t = 0;
            while (1) {
                if (t > 250) { //��һ��ͼƬ
                    curindex++;
                    if (curindex >= totpicnum) curindex = 0; //��ĩβ��ʱ��,�Զ���ͷ��ʼ
                    break;
                }
                if ((t % 20) == 0) LED0 = !LED0; //LED0��˸,��ʾ������������.
                tp_dev.scan(0);
                if (tp_dev.sta & TP_PRES_DOWN) {
                    if (tp_dev.y[0] <= 50 && tp_dev.x[0] <= 50) // go back
                    {
                        goto INIT;
                    } else if (tp_dev.x[0] <= 50 && tp_dev.y[0] >= lcddev.height - 50) //�������½ǣ�ɾ������
                    {
                        f_unlink((char * ) pname);
                        LCD_ShowString(60, 150, 200, 16, 16, "Deleted!");
                        delay_ms(500);
                        break;
                    } else if (tp_dev.y[0] < 120) //�����ϲ��ֵ�1/3����һ��
                    {
                        if (curindex) curindex--;
                        else curindex = totpicnum - 1;
                        break;
                    } else if (tp_dev.y[0] > 120 && tp_dev.y[0] < 240) //�����м��1/3����ͣ
                    {
                        pause = !pause;
                        LED1 = !pause; //��ͣ��ʱ��LED1��.
                    } else if (tp_dev.y[0] > 240) //���������1/3����һ��
                    {
                        curindex++;
                        if (curindex >= totpicnum) curindex = 0; //��ĩβ��ʱ��,�Զ���ͷ��ʼ
                        break;
                    }
                }
                if (pause == 0) t++;
                delay_ms(10);
            }
            res = 0;
        }
        myfree(picfileinfo.lfname); //�ͷ��ڴ�			    
        myfree(pname); //�ͷ��ڴ�			    
        myfree(picindextbl); //�ͷ��ڴ�
    }
}

void render_main_menu(void) {
    LCD_Clear(WHITE);
    POINT_COLOR = BLACK;
    LCD_DrawRectangle(70, 80, 170, 140);
    LCD_DrawRectangle(70, 180, 170, 240);
    LCD_ShowString(100, 100, 100, 16, 16, "Album");
    LCD_ShowString(95, 200, 100, 16, 16, "Camera");

}
