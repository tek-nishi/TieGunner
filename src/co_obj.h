
/*

  TODO:������

 */

//==============================================================
#ifndef CO_OBJ_H
#define CO_OBJ_H
//==============================================================

#include "co_common.h"
#include "co_param.h"

#ifdef __cplusplus
extern              "C"
{
#endif

/********************************************/
/*             �萔�E�}�N���錾             */
/********************************************/
// ��p���b�Z�[�W
enum enmMSG_OBJ {
	_MSG_OBJ = MSG_OBJ,							// �_�~�[
};

// �^�C�v�̓A�v���P�[�V�������Ŏw��
//------------------------------
#define OBJ_TYPE_ALL     (~0)


/********************************************/
/*                �\���̐錾                */
/********************************************/
typedef struct _sOBJ sOBJ;
typedef int (*OBJ_PROC)(sOBJ *obj, sParam *param, int msg, int lParam, int rParam);


/********************************************/
/*              �O���[�o���ϐ�              */
/********************************************/


/********************************************/
/*              �O���[�o���֐�              */
/********************************************/
// ������
extern void ObjInit(void);
// �I�u�W�F�N�g�̃Z�b�g�A�b�v
extern sParam *ObjSetup(char *id_str);
// �Z�b�g�A�b�v�ς݂̃p�����[�^���擾
extern sParam *ObjGetSetupParam(char *id_str);
// �Z�b�g�A�b�v�����f�[�^�̔j��
extern void ObjDeleteParam(char *id_str);
// �Z�b�g�A�b�v�����S�f�[�^�̔j��
extern void ObjDeleteParamAll(void);
extern void ObjDeleteAll(u_int type);

// �I�u�W�F�N�g�擾
extern sOBJ *ObjGetNext(sOBJ *prev, u_int type, char *id_str);
// ���b�Z�[�W���M
extern int ObjPostMsg(sOBJ *obj, int msg, int lParam, int rParam);
// �S�I�u�W�F�N�g�փ��b�Z�[�W���M
extern BOOL ObjPostMsgAll(u_int type, int msg, BOOL abort, int lParam, int rParam);

// �I�u�W�F�N�g����
extern sOBJ *ObjCreate(char *id_str, u_int type, OBJ_PROC func, int lParam, int rParam);
// �I�u�W�F�N�g�폜�v��
extern void ObjKillReq(sOBJ *obj);

// ���[�N�擾
extern void *ObjGetVar(sOBJ *obj, int size);
// �p�����[�^�擾
extern sParam *ObjGetParam(sOBJ *obj);
// �^�C�v�擾
extern u_int ObjGetType(sOBJ *obj);
// �^�C�v�ύX
extern void ObjSetType(sOBJ *obj, u_int type);
// �ʒu���擾
extern FVector2 *ObjGetPos(sOBJ *obj);
// �ʒu��ύX
extern void ObjSetPos(sOBJ *obj, REAL x, REAL y);

extern REAL ObjGetDir(sOBJ *obj);
extern void ObjSetDir(sOBJ *obj, REAL dir);
extern FVector2 *ObjGetVct(sOBJ *obj);
extern void ObjSetVct(sOBJ *obj, REAL x, REAL y);
extern REAL ObjGetRadius(sOBJ *obj);
extern void ObjSetRadius(sOBJ *obj, REAL radius);
extern BOOL ObjIsDead(sOBJ *obj);
extern void ObjSetDeath(sOBJ *obj, BOOL death);


#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================

