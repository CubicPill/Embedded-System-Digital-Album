#include "camera.h"
#include "ov7670.h" 
#include "lcd.h"
#include "exti.h"
#include "exfuns.h"
#include "stdio.h"
#include "ff.h"
#include "malloc.h"
#include "delay.h"
#include "bmp.h"
extern u8 ov_sta;
u8 THRESHOLD = 0x10;
const u8 MODE_NONE = 0;
const u8 MODE_REVERT = 1;
const u8 MODE_GRAY = 2;
const u8 MODE_BIN = 3;

void save_picture(void) {

    u8 * pname;
    u8 errno = 0;
    pname = mymalloc(30);
    while (pname == NULL) //内存分配出错
    {
        LCD_ShowString(60, 150, 200, 24, 24, "Malloc Error!");

        delay_ms(200);
    }
    camera_new_pathname(pname);
    errno = bmp_encode(pname, 0, 0, 240, 320, 1);
    if (errno) {
        LCD_ShowString(60, 150, 200, 24, 24, "Write Error!");
    } else {
        LCD_ShowString(60, 150, 200, 24, 24, "Success!");

    }
    delay_ms(1000);

}

__inline u16 read_data(void) {
    u16 color;
    GPIOB -> CRL = 0X88888888;
    OV7670_RCK = 0;
    color = OV7670_DATA; //读数据
    OV7670_RCK = 1;
    color <<= 8;
    OV7670_RCK = 0;
    color |= OV7670_DATA; //读数据      
    OV7670_RCK = 1;
    GPIOB -> CRL = 0X33333333;
    return color;
}

void camera_refresh(u8 mode, int fill_pixels_count) {
    u32 j;
    u16 color;
    if (ov_sta == 2) {
        LCD_Scan_Dir(U2D_L2R); //从上到下,从左到右 
        LCD_SetCursor(0x00, 0x0000); //设置光标位置 
        LCD_WriteRAM_Prepare(); //开始写入GRAM  
        OV7670_CS = 0;
        OV7670_RRST = 0; //开始复位读指针 
        OV7670_RCK = 0;
        OV7670_RCK = 1;
        OV7670_RCK = 0;
        OV7670_RRST = 1; //复位读指针结束 
        OV7670_RCK = 1;
        for (j = 0; j < fill_pixels_count; j++) {
            color = read_data();
            switch (mode) {
            case MODE_NONE:
                break;
            case MODE_REVERT:
                color = ~color;
                break;
            case MODE_BIN:
                color = binarization(color);
                break;
            case MODE_GRAY:
                color = gray_scale(color);
                break;
            }
            LCD_WR_DATA(color);
        }
        OV7670_CS = 1;
        OV7670_RCK = 0;
        OV7670_RCK = 1;
        EXTI -> PR = 1 << 15; //清除LINE8上的中断标志位
        ov_sta = 0; //开始下一次采集
        LCD_Scan_Dir(DFT_SCAN_DIR); //恢复默认扫描方向                 
    }
}

__inline u16 gray_scale(u16 color) {
    u16 r, g, b, gray;
    r = (color & 0xf800) >> 11;
    g = (color & 0x07e0) >> 6;
    b = color & 0x0010;
    gray = (r * 28 + g * 151 + b * 77) >> 8;
    color = (gray << 11) | (gray << 6) | gray;
    return color;

}

__inline u16 binarization(u16 color) {
    u16 r, g, b, gray;
    r = (color & 0xf800) >> 11;
    g = (color & 0x07e0) >> 6;
    b = color & 0x0010;
    gray = (r * 28 + g * 151 + b * 77) >> 8;
    color = (gray > THRESHOLD) ? 0xffff : 0x0;
    return color;

}
void camera_new_pathname(u8 * pname) {
    u8 res;
    u16 index = 0;
    while (index < 0XFFFF) {
        sprintf((char * ) pname, "0:DCIM/PIC%05d.bmp", index);
        res = f_open(ftemp, (const TCHAR * ) pname, FA_READ);

        if (res == FR_NO_FILE) {
            break;
        }
        ++index;
    }
}

void render_side_bar(void) {
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

u8 set_camera_mode(u16 p, u16 * mode) {
    if (p <= 80) {
        LCD_Fill(180, 0, 240, 80, LIGHTBLUE);
        LCD_ShowChar(205, 30, 'N', 24, 1);
  			*mode = 0;
        return 1;
    } else if (p > 80 && p <= 160) {
        LCD_Fill(180, 80, 240, 160, LIGHTBLUE);
        LCD_ShowChar(205, 110, 'R', 24, 1); 
			  *mode = 1;
        return 1;
    } else if (p > 160 && p <= 240) {
        LCD_Fill(180, 160, 240, 240, LIGHTBLUE);
        LCD_ShowChar(205, 190, 'G', 24, 1); 
			  *mode = 2;
        return 1;
    } else if (p > 240) {
        LCD_Fill(180, 240, 240, 320, LIGHTBLUE);
        LCD_ShowChar(205, 270, 'B', 24, 1); 
			  *mode = 3;
        return 1;
    }
    return 0;
}
