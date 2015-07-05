//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
//  < fpack >
//   2004 ASTROLL Inc. All Rights Reserved.
//--------------------------------------------------------------
//
//	�n�b�V���@�ɂ�錟��
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  $Id: hash.h,v 1.1.1.1 2004/02/09 08:13:36 nishi Exp $
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
/*

	�n�b�V���@���g��������������񋟂��܂�

	�C�ӂ̒����̕�����ƁA�C�Ӓ��̒l�������܂��B

	hash = OpenHash(256)				// �e�[�u���I�[�v��(�e�[�u���T�C�Y�͂Q�ׂ̂���)
	p = InstallString(hash, "test");	// �������o�^

	p->s = "hoge"						// �K���ɒl����
	p->a = hoge;

	p = LookupString(hash, "test");		// �����񂩂猟��
	hoge = p->a;						// �l�����o��

	CloseHash(hash)						// ��n��

*/

//==============================================================
#ifndef _HASH_H_
#define _HASH_H_
//==============================================================

/********************************************/
/*             �萔�E�}�N���錾             */
/********************************************/


/********************************************/
/*                �\���̐錾                */
/********************************************/
typedef struct hash {
	char *s;
	int a, b;
	struct hash *n;								// ���̍\���̂ւ̃|�C���^
} HASH;

typedef struct {
	int	hashMax, hashMask;
	int	words, strings;
	HASH **hashTop;
	int	imageSize;
} IHASH;


/********************************************/
/*              �O���[�o���ϐ�              */
/********************************************/


/********************************************/
/*              �O���[�o���֐�              */
/********************************************/
extern IHASH *OpenHash(int hashMax);
extern void CloseHash(IHASH *info);
extern HASH *LookupString(IHASH *info, const char *s);
extern HASH *LookupImage(IHASH *info, const char *s);
extern HASH *InstallString(IHASH *info, const char *s);
extern HASH *InstallImage(IHASH *info, const char *s);
extern void UninstallString(IHASH *info, const char *s);
extern void UninstallImage(IHASH *info, const char *s);
extern HASH **CreateHashList(IHASH *info);
extern HASH **SortHashList(IHASH *info);
extern HASH **SortHashValueA(IHASH *info);
extern HASH **SortHashValueB(IHASH *info);
extern void ListtingHashList(IHASH *info, FILE *fp);
extern HASH **AddHashPointer(HASH **hpp, HASH *hp);


//==============================================================
#endif
//==============================================================

