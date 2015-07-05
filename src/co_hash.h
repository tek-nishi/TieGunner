//
//	�n�b�V���@�ɂ��f�[�^����
//

/*

	�n�b�V���@���g��������������񋟂��܂�

	�C�ӂ̒����̕�����ƁA�C�Ӓ��̒l�������܂��B

	hash = OpenHash(256)				// �e�[�u���I�[�v��
	p = InstallString(hash, "test");	// �������o�^

	p->s = "hoge"						// �K���ɒl����
	p->a = hoge;

	p = LookupString(hash, "test");		// �����񂩂猟��
	hoge = p->a;						// �l�����o��

	CloseHash(hash)						// ��n��

*/

//==============================================================
#ifndef CO_HASH_H
#define CO_HASH_H
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
typedef struct hash {
	char s[ID_MAXLEN];
	char *str;
	int a, b;
	void *p;
	struct hash *n;								// ���̍\���̂ւ̃|�C���^

	struct hash *next;
} HASH;

typedef struct _IHASH IHASH;


/********************************************/
/*              �O���[�o���ϐ�              */
/********************************************/


/********************************************/
/*              �O���[�o���֐�              */
/********************************************/
// �����e�[�u�����J��
extern IHASH *OpenHash(int hashMax);
// �����e�[�u�������
extern void CloseHash(IHASH *info);
// �����e�[�u�����ď�����
extern void ClearHash(IHASH *info);
// �C���[�W����
extern HASH *LookupString(IHASH *info, const char *s);
// ������̓o�^
extern HASH *InstallString(IHASH *info, const char *s);
// �o�^��������폜
extern void UninstallString(IHASH *info, const char *s);
// �n���h���̕�������擾
extern char *GetHashString(HASH *hp);

// �n�b�V���̑S���X�g�����
extern HASH **CreateHashList(IHASH *info);
// �n�b�V���̃\�[�g
extern HASH **SortHashList(IHASH *info);
// �l�`�ɑ΂��Ẵn�b�V���l�̃\�[�g
extern HASH **SortHashValueA(IHASH *info);
// �l�a�ɑ΂��Ẵn�b�V���l�̃\�[�g
extern HASH **SortHashValueB(IHASH *info);
// ���X�g�����n�b�V�����t�@�C���ɏ����o��
extern void ListtingHashList(IHASH *info, FILE *fp);
// �n�b�V�����X�g�Ɋۂ��Ɖ�����
extern HASH **AddHashPointer(HASH **hpp, HASH *hp);


#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================

