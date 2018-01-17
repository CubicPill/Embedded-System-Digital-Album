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

//得到path路径下,目标文件的总个数
//path:路径		    
//返回值:总有效文件数


int main(void) {
    DIR picdir; //图片目录
    FILINFO picfileinfo; //文件信息
    u8 * fn; //长文件名
    u8 * pname; //带路径的文件名
    u16 totpicnum; //图片文件总数
    u16 curindex; //图片当前索引
    u8 pause = 0; //暂停标记
    u16 * picindextbl; //图片索引表
    u8 t;
    u16 temp;

    u16 mode;
    u8 res;
    u8 option = 0;
    delay_init(); //延时函数初始化
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 设置中断优先级分组2
    uart_init(9600);
    OV7670_Init();
    LED_Init(); //初始化与LED连接的硬件接口
    LCD_Init(); //初始化LCD

    tp_dev.init();

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
    mem_init(); //初始化内部内存池
    exfuns_init();
    f_mount(fs, "0:", 1); //挂载SD卡
    res = f_mkdir("0:/DCIM");
    if (res != FR_EXIST && res != FR_OK) //发生了错误
    {
        LCD_Clear(WHITE);
        LCD_ShowString(60, 150, 200, 16, 16, "Folder Error!");
        delay_ms(2000);
        return 0;
    }

    EXTI15_Init(); //使能定时器捕获
    OV7670_Window_Set(10, 174, 240, 320); //设置窗口	  
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
            camera_refresh(mode, pixel_count); //更新显示	 
        }

    } else if (option == 1) {
        //entering album
        delay_ms(500);
        BACK_COLOR = WHITE;

        while (f_opendir( & picdir, "0:/DCIM")) //打开图片文件夹
        {
            LCD_ShowString(60, 170, 240, 16, 16, "No picture folder!");
            delay_ms(200);
            LCD_Fill(60, 170, 240, 186, WHITE); //清除显示	     
            delay_ms(200);
        }
        totpicnum = pic_get_tnum("0:/DCIM"); //得到总有效文件数
        while (totpicnum == NULL) //图片文件为0		
        {
          LCD_Clear(WHITE); //清除显示	    
					LCD_ShowString(60, 170, 240, 16, 16, "No picture file!");
            delay_ms(400);
               
        }
        picfileinfo.lfsize = _MAX_LFN * 2 + 1; //长文件名最大长度
        picfileinfo.lfname = mymalloc(picfileinfo.lfsize); //为长文件缓存区分配内存
        pname = mymalloc(picfileinfo.lfsize); //为带路径的文件名分配内存
        picindextbl = mymalloc(2 * totpicnum); //申请2*totpicnum个字节的内存,用于存放图片索引
        while (picfileinfo.lfname == NULL || pname == NULL || picindextbl == NULL) //内存分配出错
        {
					          LCD_Clear(WHITE); //清除显示	    

            LCD_ShowString(60, 170, 240, 16, 16, "Memory allocation failure!");
            delay_ms(400);
            
        }
        //记录索引
        res = f_opendir( & picdir, "0:/DCIM"); //打开目录
        if (res == FR_OK) {
            curindex = 0; //当前索引为0
            while (1) //全部查询一遍
            {
                temp = picdir.index; //记录当前index
                res = f_readdir( & picdir, & picfileinfo); //读取目录下的一个文件
                if (res != FR_OK || picfileinfo.fname[0] == 0) break; //错误了/到末尾了,退出		  
                fn = (u8 * )( * picfileinfo.lfname ? picfileinfo.lfname : picfileinfo.fname);
                res = f_typetell(fn);
                if ((res & 0XF0) == 0X50) //取高四位,看看是不是图片文件	
                {
                    picindextbl[curindex] = temp; //记录索引
                    curindex++;
                }
            }
        }
        delay_ms(1500);
        piclib_init(); //初始化画图	   	   
        curindex = 0; //从0开始显示
        res = f_opendir( & picdir, (const TCHAR * )
            "0:/DCIM"); //打开目录
        while (res == FR_OK) //打开成功
        {
            dir_sdi( & picdir, picindextbl[curindex]); //改变当前目录索引	   
            res = f_readdir( & picdir, & picfileinfo); //读取目录下的一个文件
            if (res != FR_OK || picfileinfo.fname[0] == 0) break; //错误了/到末尾了,退出
            fn = (u8 * )( * picfileinfo.lfname ? picfileinfo.lfname : picfileinfo.fname);
            strcpy((char * ) pname, "0:/DCIM/"); //复制路径(目录)
            strcat((char * ) pname, (const char * ) fn); //将文件名接在后面
            LCD_Clear(BLACK);
            ai_load_picfile(pname, 0, 0, lcddev.width, lcddev.height, 1); //显示图片    
            LCD_ShowString(2, 2, 240, 16, 16, pname); //显示图片名字
            t = 0;
            while (1) {
                if (t > 250) { //下一张图片
                    curindex++;
                    if (curindex >= totpicnum) curindex = 0; //到末尾的时候,自动从头开始
                    break;
                }
                if ((t % 20) == 0) LED0 = !LED0; //LED0闪烁,提示程序正在运行.
                tp_dev.scan(0);
                if (tp_dev.sta & TP_PRES_DOWN) {
                    if (tp_dev.y[0] <= 50 && tp_dev.x[0] <= 50) // go back
                    {
                        goto INIT;
                    } else if (tp_dev.x[0] <= 50 && tp_dev.y[0] >= lcddev.height - 50) //触摸左下角，删除窗口
                    {
                        f_unlink((char * ) pname);
                        LCD_ShowString(60, 150, 200, 16, 16, "Deleted!");
                        delay_ms(500);
                        break;
                    } else if (tp_dev.y[0] < 120) //触摸上部分的1/3，上一张
                    {
                        if (curindex) curindex--;
                        else curindex = totpicnum - 1;
                        break;
                    } else if (tp_dev.y[0] > 120 && tp_dev.y[0] < 240) //触摸中间的1/3，暂停
                    {
                        pause = !pause;
                        LED1 = !pause; //暂停的时候LED1亮.
                    } else if (tp_dev.y[0] > 240) //触摸下面的1/3，下一张
                    {
                        curindex++;
                        if (curindex >= totpicnum) curindex = 0; //到末尾的时候,自动从头开始
                        break;
                    }
                }
                if (pause == 0) t++;
                delay_ms(10);
            }
            res = 0;
        }
        myfree(picfileinfo.lfname); //释放内存			    
        myfree(pname); //释放内存			    
        myfree(picindextbl); //释放内存
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
