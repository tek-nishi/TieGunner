/*

  OSX��p�R�[�h
  
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
	/* Apple�̎�����M����Ȃ�΁A�ǂ�Mac�ł��Ή����Ă��� */
	return TRUE;
}

BOOL OsToggleVsyncSwap(int sync)
{
	/* FIXME:�������ǂ����Ԃ��H */
	CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &sync);
	return TRUE;
}

