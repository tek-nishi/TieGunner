/*

  OSX専用コード
  
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <OpenGL/OpenGL.h>
#include "co_os.h"


void OsInit(void)
{
}

void OsFin(void)
{
}

void OsPrintf(char *str)
{
	printf("%s", str);
}

void OsGetFileName(char *dst, char *src)
{
	strcpy(dst, src);							// 加工しないで使える
}

void *OsCreateMemory(size_t size)
{
	return malloc(size);
}

void OsDestroyMemory(void *ptr)
{
	free(ptr);
}

BOOL OsIsVsyncSwap(void)
{
	/* Appleの資料を信じるならば、どのMacでも対応している */
	return TRUE;
}

BOOL OsToggleVsyncSwap(int sync)
{
	/* FIXME:成功かどうか返す？ */
	CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &sync);
	return TRUE;
}

