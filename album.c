#include "album.h"
#include "sys.h"
#include "ff.h"
#include "malloc.h"
#include "exfuns.h"

u16 pic_get_tnum(u8 * path) {
    u8 res;
    u16 rval = 0;
    DIR tdir; //临时目录
    FILINFO tfileinfo; //临时文件信息	
    u8 * fn;
    res = f_opendir( & tdir, (const TCHAR * ) path); //打开目录
    tfileinfo.lfsize = _MAX_LFN * 2 + 1; //长文件名最大长度
    tfileinfo.lfname = mymalloc(tfileinfo.lfsize); //为长文件缓存区分配内存
    if (res == FR_OK && tfileinfo.lfname != NULL) {
        while (1) //查询总的有效文件数
        {
            res = f_readdir( & tdir, & tfileinfo); //读取目录下的一个文件
            if (res != FR_OK || tfileinfo.fname[0] == 0) break; //错误了/到末尾了,退出		  
            fn = (u8 * )( * tfileinfo.lfname ? tfileinfo.lfname : tfileinfo.fname);
            res = f_typetell(fn);
            if ((res & 0XF0) == 0X50) //取高四位,看看是不是图片文件	
            {
                rval++; //有效文件数增加1
            }
        }
    }
    return rval;
}