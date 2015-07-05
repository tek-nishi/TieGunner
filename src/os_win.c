/*

  Windows専用コード

 */

#include <windows.h>
#include <mmsystem.h>
#include <io.h>
#include <sys/types.h>
#include "co_os.h"
#include "co_debug.h"


struct _DIR {
	long handle;                            	/* _findfirst/_findnext handle */
	short offset;                           	/* offset into directory */
	short finished;                         	/* 1 if there are not more files */
	struct _finddata_t fileinfo;    			/* from _findfirst/_findnext */
	char *dir;									/* the dir we are reading */
	struct dirent dent;							/* the dirent to return */
};

static MMRESULT period_handle;

void OsInit(void)
{
	_set_error_mode(_OUT_TO_MSGBOX);
	period_handle = timeBeginPeriod(1);			// glutのタイマ割り込みの精度を上げるおまじない:P
}

void OsFin(void)
{
	timeEndPeriod(period_handle);
}


void OsPrintf(char *str)
{
	OutputDebugString(str);
}

void OsGetFileName(char *dst, char *src)
{
	char fname[256];
	char *p;

	strcpy(fname, src);
	p = fname;
	while(*p)
	{
		if(*p == '/')
			*p = '\\';
		++p;
	}
	strcpy(dst, fname);
}

void *OsCreateMemory(size_t size)
{
	return malloc(size);
}

void OsDestroyMemory(void *ptr)
{
	free(ptr);
}

/* 画面更新がモニタと同期可能か調べる(glutの初期化後に呼び出す事) */
BOOL OsIsVsyncSwap(void)
{
	BOOL res;
	const char* ext = (const char *)glGetString(GL_EXTENSIONS);
	res = strstr(ext, "WGL_EXT_swap_control") ? TRUE : FALSE;
#ifdef DEBUG
	if(res)
	{
		PRINTF("WGL_EXT_swap_control\n");
	}
#endif
	return res;
}

BOOL OsToggleVsyncSwap(int sync)
{
	typedef BOOL (APIENTRY *PFNWGLSWAPINTERVALFARPROC)( int );
	BOOL res = FALSE;

	PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALFARPROC)wglGetProcAddress( "wglSwapIntervalEXT" );
	if(wglSwapIntervalEXT)
	{
		res = wglSwapIntervalEXT(sync);
	}
	return res;
}

DIR *opendir(const char *dir)
{
	DIR *dp;
	char *filespec;
	long handle;
	size_t index;

	filespec = (char *)malloc(strlen(dir) + 2 + 1);
	strcpy(filespec, dir);
	index = strlen(filespec) - 1;
	if (index >= 0 && (filespec[index] == '/' || filespec[index] == '\\'))
		filespec[index] = '\0';
	strcat(filespec, "/*");

	dp = (DIR *) malloc(sizeof(DIR));
	dp->offset = 0;
	dp->finished = 0;
	dp->dir = strdup(dir);

	if ((handle = _findfirst(filespec, &(dp->fileinfo))) < 0)
	{
		free(filespec);
		free(dp);
		return 0;
	}
	dp->handle = handle;
	free(filespec);

	return dp;
}

struct dirent *readdir(DIR *dp)
{
	if (!dp || dp->finished)
		return 0;

	if (dp->offset != 0)
	{
		if (_findnext(dp->handle, &(dp->fileinfo)) < 0)
		{
			dp->finished = 1;
		#if 0
			if (ENOENT == errno)
			{
				/* Clear error set to mean no more files else that breaks things */
				errno = 0;
			}
		#endif
			return 0;
		}
	}
	++dp->offset;

	strncpy(dp->dent.d_name, dp->fileinfo.name, _MAX_FNAME);
	dp->dent.d_name[_MAX_FNAME] = '\0';
	dp->dent.d_ino = 1;
	/* reclen is used as meaning the length of the whole record */
	dp->dent.d_reclen = (u_short)strlen(dp->dent.d_name) + sizeof(char) + sizeof(dp->dent.d_ino) + sizeof(dp->dent.d_reclen) + sizeof(dp->dent.d_off);
	dp->dent.d_off = dp->offset;

	return &(dp->dent);
}

int closedir(DIR * dp)
{
	if (!dp)
		return 0;

	_findclose(dp->handle);
	if (dp->dir)
		free(dp->dir);
	if (dp)
		free(dp);

	return 0;
}
