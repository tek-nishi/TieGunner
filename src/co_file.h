
/*

	�ʏ�̃t�@�C���ƁA�p�b�N���ꂽ�t�@�C���Ƃ̈Ⴂ���ӎ����Ȃ��ň�����t�@�C���A�N�Z�X
	��񋟂��܂��B
	���Z�p���[�^�[�ɂ́u/�v���g���Ă��������B

	����:
	�E�}�E���g�ł���t�@�C���͓����ɂP�����ł�
	�EFsWrite() �ō쐬�����t�@�C���͏������ݐ�p�ŊJ���Ă���̂ŁA���̂܂ܓǂݍ��ގ��͏o���܂���

*/

//==============================================================
#ifndef CO_FILE_H
#define CO_FILE_H
//==============================================================

#include "co_common.h"

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
typedef struct _sFILE sFILE;


/********************************************/
/*              �O���[�o���ϐ�              */
/********************************************/


/********************************************/
/*              �O���[�o���֐�              */
/********************************************/
// ������
extern void FsInit(void);
// �I��
extern void FsFin(void);

// �C���[�W�t�@�C���̃}�E���g
extern void FsMountImage(const char *image);
// �C���[�W�t�@�C���̃A���}�E���g
extern void FsUnMountImage(void);

// �t�@�C���I�[�v��
extern sFILE *FsOpen(const char *name);
// �t�@�C���N���[�Y
extern void FsClose(sFILE *fp);
// �t�@�C���ǂݍ���
extern size_t FsRead(sFILE *fp, void *ptr, size_t size);
// �t�@�C���V�[�N
extern size_t FsSeek(sFILE *fp, size_t ofs, int whence);
// �t�@�C���̓ǂݍ��݈ʒu���擾
extern size_t FsTell(sFILE *fp);

// �t�@�C���T�C�Y�̎擾(�Z�N�^�T�C�Y�ɐ؂�グ���l)
extern size_t FsGetSize(sFILE *fp);
// ���t�@�C���T�C�Y�̎擾
extern size_t FsGetSizeOrig(sFILE *fp);
// �t�@�C��������t�@�C���T�C�Y���擾
extern size_t FsGetFileSize(const char *name);
// �t�@�C����������t�@�C���T�C�Y���擾
extern size_t FsGetFileSizeOrig(const char *name);

// �t�@�C����V�K�쐬����
extern sFILE *FsCreate(const char *name);
// �f�[�^���t�@�C���ɏ����o��
extern void FsWrite(sFILE *fp, void *ptr, size_t size);


#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================

