/*

  UNIX�nOS��p�R�[�h
  
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
	strcpy(dst, src);							// ���H���Ȃ��Ŏg����
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
	/* TODO:���s�����\�z���Ď��� */
	return FALSE;
}

BOOL OsToggleVsyncSwap(int sync)
{
	/* TODO:���s�����\�z���Ď��� */
	return FALSE;
}
