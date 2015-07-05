
//==============================================================
#ifndef CO_STRINGS_H
#define CO_STRINGS_H
//==============================================================

#include "co_common.h"

#ifdef __cplusplus
extern              "C"
{
#endif

/********************************************/
/*             �萔�E�}�N���錾             */
/********************************************/
#define L_ID_FLD_MAXLEN	 26						// �r�b�g�t�B�[���h������

// �\�[�X�t�@�C������؂�o��
//----------------------------------------
#define MSG_SRC(_f)		l_strstr("src", _f)

// 16 �����R�s�[(�K���R�s�[��̏I�[�� '\0' ������)
//---------------------------------------------------
#define STRCPY16(dst, src)  StrCopyLength(dst, 16, src)

// �t�@�C���p�X���̕�����R�s�[(�K���R�s�[��̏I�[�� '\0' ������)
//---------------------------------------------------
#define PATHCOPY(dst, src)  StrCopyLength(dst, FNAME_MAXLEN, src)


/********************************************/
/*                �\���̐錾                */
/********************************************/


/********************************************/
/*              �O���[�o���ϐ�              */
/********************************************/


/********************************************/
/*              �O���[�o���֐�              */
/********************************************/
// ������̐؂�o��
extern const char *l_strstr(const char *str, const char *fname);
// �w�蒷�̕�����R�s�[
extern void StrCopyLength(char *dst, int len, const char *src);
// ���j�[�N�ȕ�����𐶐�����
extern const char *StrMakeUniqueName(void);

// �z���C�g�X�y�[�X���΂�
extern char *StrSkipSpace(char *ptr);
// ���̃g�[�N���ֈړ�
extern char *StrNextToken(char *ptr);
// ���̍s��Ԃ�
extern char *StrNextLine(char *ptr);

// �R�����g�s���`�F�b�N
extern BOOL StrIsComment(char *ptr);
// �e�L�X�g�I�[���`�F�b�N
extern BOOL StrIsTextEnd(char *ptr);
// �u�����N���`�F�b�N
extern BOOL StrIsBlank(char *ptr);
// �������`�F�b�N
extern BOOL StrIsValue(char *ptr);
// SJIS�R�[�h������
extern BOOL StrIsSJIS(char *ptr);

// ������擾
extern int StrGetTextId(char *dst, char *src);
// ������ϊ�
extern int StrGetValue(char *ptr);
// ������ϊ�
extern REAL StrGetReal(char *ptr);
// �������X�L�b�v
extern char *StrSkipValue(char *ptr);
// �P�s�̃T�C�Y���擾(SJIS������)
extern int StrGetLineLen(char *ptr);

// Bin�^�������Int�^10�i���ɕϊ�����
extern int StrBinStrToInt(char *src);
// Int�^10�i����Bin�^������ɕϊ�����
extern void StrIntToBinStr(char *dst, int src);

// ��������������֕ϊ�
extern void StrToLower(char *dst);
// �������啶���֕ϊ�
extern void StrToUpper(char *dst);


//--------------------------------------------
// �C�����C���֐��Q
//--------------------------------------------

//===============================================================
static const char *MSG_FILE(const char *_f)
//--------------------------------------------------------------
// �t�@�C�������� PATH_DEVELOP ���������|�C���^��Ԃ�
//--------------------------------------------------------------
//===============================================================
{
	const char *out = l_strstr(PATH_DEVELOP, _f);
	return l_strstr("/", out);
}

#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================

