/*

  UNIX系OS専用コード
  
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <GLUT/glut.h>
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
	/* TODO:実行環境を構築して実装 */
	return FALSE;
}

BOOL OsToggleVsyncSwap(int sync)
{
	/* TODO:実行環境を構築して実装 */
	return FALSE;
}
