
//==============================================================
#ifndef CO_OS_H
#define CO_OS_H
//==============================================================

#include "co_types.h"

#ifdef __cplusplus
extern              "C"
{
#endif

#if defined (_MSC_VER)
	typedef struct _DIR DIR;

	struct dirent {
		long d_ino;								/* inode (always 1 in WIN32) */
		off_t d_off;							/* offset to this dirent */
		unsigned short d_reclen;				/* length of d_name */
		char d_name[_MAX_FNAME + 1];			/* filename (null terminated) */
	};
#endif

extern void OsInit(void);
extern void OsFin(void);

extern void OsPrintf(char *str);
extern void OsGetFileName(char *dst, char *src);

extern void *OsCreateMemory(size_t size);
extern void OsDestroyMemory(void *ptr);

extern BOOL OsIsVsyncSwap(void);
extern BOOL OsToggleVsyncSwap(int sync);

#if defined (_MSC_VER)
	// ディレクトリを開く
	extern DIR *opendir(const char *dir);
	// 次のディレクトリを開く
	extern struct dirent *readdir(DIR *dp);
	// ハンドルを閉じる
	extern int closedir(DIR * dp);
#endif


#ifdef __cplusplus
}
#endif

//==============================================================
#endif
//==============================================================
