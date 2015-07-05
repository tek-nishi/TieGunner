//
//	�ėp�I�u�W�F�N�g�����N
//

//==============================================================
#ifndef CO_OBJLINK_H
#define CO_OBJLINK_H
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
typedef struct _sLink sLink;


/********************************************/
/*              �O���[�o���ϐ�              */
/********************************************/


/********************************************/
/*              �O���[�o���֐�              */
/********************************************/
// ����
extern sLink *ObjLinkCreate(int size, int num, int mem, BOOL new_area);
// �j��
extern void ObjLinkDestroy(sLink *link);

// �I�u�W�F�N�g���m��
extern void *ObjLinkNew(sLink *link);
// �I�u�W�F�N�g���폜
extern void ObjLinkDel(sLink *link, void *obj);
// �S�I�u�W�F�N�g���폜
extern void ObjLinkDelAll(sLink *link);
// �I�u�W�F�N�g�����X�g�ɑ}��
extern void ObjLinkInsert(sLink *link, void *obj, void *src, BOOL next);
// �ŏ��̃|�C���^���擾
extern void *ObjLinkGetTop(sLink *link);
// �Ō�̃|�C���^���擾
extern void *ObjLinkGetLast(sLink *link);
// ���̃|�C���^���擾
extern void *ObjLinkGetNext(void *ptr);
// �O�̃|�C���^���擾
extern void *ObjLinkGetPrev(void *ptr);
// �f�[�^�����擾
extern int ObjLinkGetNum(sLink *link);


#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================

