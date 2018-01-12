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


__inline void render_main_menu(void);
__inline void render_side_bar(void);
int pixel_count = 76800;
int main(void) {
    u16 mode;
    u8 option = 0;
    delay_init(); //延时函数初始化
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 设置中断优先级分组2
    uart_init(9600);
    OV7670_Init();
    LED_Init(); //初始化与LED连接的硬件接口
    LCD_Init(); //初始化LCD

    tp_dev.init();

    usmart_dev.init(72); //初始化USMART	
    POINT_COLOR = BLACK;
    while (OV7670_Init()) //初始化OV7670
    {
        LCD_ShowString(60, 150, 200, 200, 16, "OV7670 Error!!");
        delay_ms(200);
        LCD_Fill(60, 150, 239, 166, WHITE);
        delay_ms(200);
    }
    while (SD_Initialize()) //检测SD卡
    {
        LCD_ShowString(60, 150, 200, 16, 16, "SD Card Error!");
        delay_ms(200);
        LCD_Fill(60, 150, 240, 150 + 16, WHITE); //清除显示			  
        delay_ms(200);
    }
    exfuns_init();
    mem_init();
    f_mount(fs, "0:", 1); //挂载SD卡 

    EXTI15_Init(); //使能定时器捕获
    OV7670_Window_Set(10, 174, 240, 320); //设置窗口	  
    OV7670_CS = 0;

    INIT:
        render_main_menu();
    pixel_count = 76800;
    mode = 0;

    option = 0;

    while (!option) {
        tp_dev.scan(0);
        if (tp_dev.sta & TP_PRES_DOWN) {
            BACK_COLOR = LIGHTBLUE;
            if (tp_dev.x[0] <= 170 && tp_dev.x[0] >= 70 && tp_dev.y[0] <= 140 && tp_dev.y[0] >= 80) //触摸屏被按下
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
            if (tp_dev.sta & TP_PRES_DOWN) //触摸屏被按下
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
                }

            }
            camera_refresh(mode, pixel_count); //更新显示	 
        }

    } else if (option == 1) {
        delay_ms(500);
        // album
        LCD_Clear(WHITE);
        LCD_ShowString(60, 150, 200, 16, 16, "Not implemented!");
        delay_ms(1000);
        goto INIT;

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
