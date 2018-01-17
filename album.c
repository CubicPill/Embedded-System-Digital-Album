#include "album.h"
#include "sys.h"
#include "ff.h"
#include "malloc.h"
#include "exfuns.h"

u16 pic_get_tnum(u8 * path) {
    u8 res;
    u16 rval = 0;
    DIR tdir; //��ʱĿ¼
    FILINFO tfileinfo; //��ʱ�ļ���Ϣ	
    u8 * fn;
    res = f_opendir( & tdir, (const TCHAR * ) path); //��Ŀ¼
    tfileinfo.lfsize = _MAX_LFN * 2 + 1; //���ļ�����󳤶�
    tfileinfo.lfname = mymalloc(tfileinfo.lfsize); //Ϊ���ļ������������ڴ�
    if (res == FR_OK && tfileinfo.lfname != NULL) {
        while (1) //��ѯ�ܵ���Ч�ļ���
        {
            res = f_readdir( & tdir, & tfileinfo); //��ȡĿ¼�µ�һ���ļ�
            if (res != FR_OK || tfileinfo.fname[0] == 0) break; //������/��ĩβ��,�˳�		  
            fn = (u8 * )( * tfileinfo.lfname ? tfileinfo.lfname : tfileinfo.fname);
            res = f_typetell(fn);
            if ((res & 0XF0) == 0X50) //ȡ����λ,�����ǲ���ͼƬ�ļ�	
            {
                rval++; //��Ч�ļ�������1
            }
        }
    }
    return rval;
}