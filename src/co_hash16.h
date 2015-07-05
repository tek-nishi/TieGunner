//
//	�L�[������ 15bytes ����̃n�b�V���֐�
//

//==============================================================
#ifndef CO_HASH16_H
#define CO_HASH16_H
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
typedef struct _sHASH_KEY sHASH_KEY;
typedef struct _sHASH	  sHASH;


/********************************************/
/*              �O���[�o���ϐ�              */
/********************************************/


/********************************************/
/*              �O���[�o���֐�              */
/********************************************/
// ����
extern sHASH *HashCreate(char *id_str);
// �j��
extern void HashKill(sHASH *hash);
// ����
extern void HashCleanup(sHASH *hash);

// �o�^
extern void HashAdd(sHASH *hash, char *id_str, void *ptr);
// �擾
extern void *HashGet(sHASH *hash, char *id_str);
// �폜
extern void HashDel(sHASH *hash, char *id_str);

// �o�^�����擾
extern int HashGetKeyNum(sHASH *hash);
// �Փː����擾
extern int HashGetCollision(sHASH *hash);

// �o�^���X�g���쐬
extern sHASH_KEY **HashGetKeyList(sHASH *hash);
// �L�[��ID���擾
extern char *HashGetKeyId(sHASH_KEY *key);
// �L�[�̓o�^���e���擾
extern void *HashGetKeyValue(sHASH_KEY *key);


#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================

