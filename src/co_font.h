
/*

	���p�̃t�H���g�\�����s���܂��B��Ƀf�o�b�O��_�C�A���O�\���Ɏg���܂��B
	�e�N�X�`���̃T�C�Y�����͓��ɖ����A������16x16�Ŕz�u����Ă���΁B


	�g�����ꂽ�@�\

	�F�w��
		$C0 : ��
		  1 : ��
		  2 : ��
		  3 : �}�[���^
		  4 : ��
		  5 : �V�A��
		  6 : ��
		  7 : ��(default)
		  8 : �g���F-1
		  9 : �g���F-2

	�n�C���C�g
		$H0 : �ʏ�(default)
		  1 : �O���[
		  2 : ������

	�_��
		$B0 : �_�Ŗ���(default)
		  1 : �_��(��)
		  2 : �_��(�x)

	�t�H���g�؂�ւ�
		$F0 : FONT_SYS(default)
		  1 : FONT_APP_1
		  2 : FONT_APP_2
		  3 : FONT_APP_3
		  4 : FONT_APP_4

	�A���t�@�l
	    $A<falue>$

	�g���t�H���g�R�[�h
		$E0 : 

	'$'��\�����������́A'$$'�ƂQ���ׂď����B

*/

//==============================================================
#ifndef CO_FONT_H
#define CO_FONT_H
//==============================================================

#include "co_common.h"

#ifdef __cplusplus
extern              "C"
{
#endif

/********************************************/
/*             �萔�E�}�N���錾             */
/********************************************/
enum enmFONT_IDX {
	FONT_SYS,									// �V�X�e���t�H���g
	FONT_APP_1,									// �A�v���P�[�V�����t�H���g
	FONT_APP_2,
	FONT_APP_3,
	FONT_APP_4,

	//--------------
	FONT_NUM
};


/********************************************/
/*                �\���̐錾                */
/********************************************/
typedef struct {
	char tex_id[ID_MAXLEN];						// �e�N�X�`����
	int width;									// ������
	int height;									// ���s��
} sFontInfo;


/********************************************/
/*              �O���[�o���ϐ�              */
/********************************************/


/********************************************/
/*              �O���[�o���֐�              */
/********************************************/
// ������
extern void FontInit(void);
// �I��
extern void FontFin(void);

// �t�H���g�̃Z�b�g�A�b�v
extern void FontSetup(int index, char *file, int next_x, int next_y);
// �t�H���g���̎擾
extern void FontGetInfo(sFontInfo *res, int index);

// �\��
extern void FontPrint(int x, int y, int prio, char *str);
// �\��(printf����)
extern void FontPrintF(int x, int y, int prio, char *fmt, ...);

// �\�������擾
extern int FontGetPrintWidth(char *str);
// �g���F��ݒ�
extern void FontSetExColor(int index, sRGBA *col);


#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================

