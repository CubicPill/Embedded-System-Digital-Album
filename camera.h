#ifndef __CAMERA_H
#define __CAMERA_H
#include "sys.h"
__inline u16 gray_scale(u16 color);
__inline  u16 binarization(u16 color);
void save_picture(void);
void camera_new_pathname(u8 *pname);
void camera_refresh(u8 mode,int fill_pixels_count);
__inline u16 read_data(void);


#endif
