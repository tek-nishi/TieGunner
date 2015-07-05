
/*

	zlib �𗘗p�����f�[�^�̈��k/�W�J���s���܂��B

	���k���ꂽ�f�[�^�̐擪�S�o�C�g�ɁA���k�O�ƈ��k��̃f�[�^�T�C�Y��
	�L�^����Ă��܂��B

*/

//==============================================================
#ifndef CO_ZLIB_H
#define CO_ZLIB_H
//==============================================================

#include "co_common.h"
#if defined (_MSC_VER)
#ifdef DEBUG
#pragma comment (lib, "zlibd.lib")
#else
#pragma comment (lib, "zlib.lib")
#endif
#endif

#ifdef __cplusplus
extern              "C"
{
#endif

// zlib �ɂ�鈳�k
extern void *ZlibEncode(void *ptr, int size);
// zlib �ɂ��W�J
#define ZlibDecode(ptr, size)  _ZlibDecode(ptr, size, MEM_APP)
// zlib �ɂ��W�J(�������w��)
extern void *_ZlibDecode(void *ptr, int size, int mem);

// ���k��̃f�[�^�T�C�Y
extern int ZlibEncodeSize(void *ptr);
// ���k�O�̃f�[�^�T�C�Y
extern int ZlibDecodeSize(void *ptr);

#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================

