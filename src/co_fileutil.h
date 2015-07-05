
//==============================================================
#ifndef CO_FILEUTIL_H
#define CO_FILEUTIL_H
//==============================================================

#include "co_common.h"
#include "co_file.h"

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


/********************************************/
/*              �O���[�o���ϐ�              */
/********************************************/


/********************************************/
/*              �O���[�o���֐�              */
/********************************************/
// �t���p�X����t�@�C���� + �g���q�݂̂����o
extern const char *GetFileBaseName(const char *fname);
// �t�@�C��������g���q���擾
extern const char *GetFileExt(const char *file);
// �t�@�C�������p�X�Ǝ��t�@�C�����ɕ���
extern void GetPathName(const char *fname, char *path, char *file, BOOL flag);
// �J�����g�p�X����菜�����p�X��ԋp����
extern char *GetCurPath(char *path, char *cur);

// malloc ���ēǂݍ���
extern void *_MmFileLoadB(const char *file, int area);
// MEM_FS �̈�� malloc ���ēǂݍ���
#define MmFileLoadB(file)  _MmFileLoadB(file, MEM_FS)
// MmFileLoadB() �œǂݍ��񂾃T�C�Y�𓾂�
extern size_t MmFileGetSize(void);
// �t�@�C�������݂��邩�`�F�b�N
extern BOOL MmFileCheck(const char *file);

// �e�L�X�g�t�@�C����ǂݍ���
extern void *MmTextFileLoad(const char *file);

// �t�@�C���������o��
extern void MmFileWriteB(const char *file, void *ptr, size_t len);
// PATH_BACKUP �Ƀo�b�N�A�b�v���Ƃ�
extern void MakeBackupFile(const char *fname);


#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================

