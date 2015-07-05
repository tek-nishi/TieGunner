
/*

	�f�[�^�^�C�v

	PNG_COLOR_TYPE_PALETTE		�C���f�b�N�X�J���[
	PNG_COLOR_TYPE_RGB			RGB
	PNG_COLOR_TYPE_RGB_ALPHA	RGBA

*/

//==============================================================
#ifndef CO_PNG_H
#define CO_PNG_H
//==============================================================

#include "co_common.h"
#if defined (_MSC_VER)
#ifdef DEBUG
#pragma comment (lib, "libpngd.lib")
#else
#pragma comment (lib, "libpng.lib")
#endif
#endif

#ifdef __cplusplus
extern              "C"
{
#endif

/********************************************/
/*             �萔�E�}�N���錾             */
/********************************************/


/********************************************/
/*                �\���̐錾                */
/********************************************/
typedef struct {
	u_char red, green, blue, alpha;
} sPalette;

typedef struct {
	int type;									// �f�[�^�^�C�v
	int width, height;							// �f�[�^�T�C�Y
	u_char *image;								// �C���[�W�i�[�|�C���^

	int palnum;									// �p���b�g��
	sPalette *pal;								// �p���b�g�i�[�|�C���^
} sPNG;


/********************************************/
/*              �O���[�o���ϐ�              */
/********************************************/


/********************************************/
/*              �O���[�o���֐�              */
/********************************************/
extern sPNG *PngRead(void *ptr, size_t size);
extern void PngDestroy(sPNG *hdr);

extern void PngWrite(char *file, int w, int h, u_char *ptr);


#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================
