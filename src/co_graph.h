
/*

	prio �̒l�̓}�C�i�X�l�ł��ݒ�B�l���������قǑO�ʂɕ\������܂��B
	�܂��A�����v���C�I���e�B���m�ł́A�ォ���`���������O�ʂɂȂ�܂��B

*/

//==============================================================
#ifndef CO_GRAPH_H
#define CO_GRAPH_H
//==============================================================

#include "co_common.h"
#include "co_texture.h"

#ifdef __cplusplus
extern              "C"
{
#endif

/********************************************/
/*             �萔�E�}�N���錾             */
/********************************************/
enum enmGRP_TYPE {
	GRP_POINT,									// �_
	GRP_LINE,									// ���C��
	GRP_BOX,									// �{�b�N�X(�g�̂�)
	GRP_QUAD,									// �l�p

	GRP_CIRCLE,									// �~(�g�̂�)
	GRP_FILLCIRCLE,								// �h��Ԃ��~
	GRP_TRIANGLE,								// �O�p�|���S��
	GRP_FAN,									// ��
	GRP_FILLDONUT,								// �h��Ԃ��h�[�i�c
};

enum enmGRP_BLEND {
	GRP_BLEND_NORMAL,							// �ʏ�̃u�����f�B���O
	GRP_BLEND_ADD,								// ���Z������
	GRP_BLEND_REV,								// ���]�\��
	GRP_BLEND_XOR,								// XOR
	GRP_BLEND_MUL,								// ��Z
	GRP_BLEND_SCREEN,							// �X�N���[������
};


/********************************************/
/*                �\���̐錾                */
/********************************************/
typedef struct _sGRPOBJ sGRPOBJ;


/********************************************/
/*              �O���[�o���ϐ�              */
/********************************************/


/********************************************/
/*              �O���[�o���֐�              */
/********************************************/
// ������
extern void GrpInit(void);
// �I��
extern void GrpFin(void);

// �Z�b�g�A�b�v
extern void GrpSetup(void);
// �`��
extern void GrpDraw(void);

// �I�u�W�F�N�g�𐶐�
extern sGRPOBJ *GrpCreate(int type, int prio);
// GRP_POINT�ō쐬
#define GRPOBJ_POINT(prio)       GrpCreate(GRP_POINT, prio)
// GRP_LINE�ō쐬
#define GRPOBJ_LINE(prio)        GrpCreate(GRP_LINE, prio)
// GRP_BOX�ō쐬
#define GRPOBJ_BOX(prio)         GrpCreate(GRP_BOX, prio)
// GRP_QUAD�ō쐬
#define GRPOBJ_QUAD(prio)        GrpCreate(GRP_QUAD, prio)
// GRP_CIRCLE�ō쐬
#define GRPOBJ_CIRCLE(prio)      GrpCreate(GRP_CIRCLE, prio)
// GRP_FILLCIRCLE�ō쐬
#define GRPOBJ_FILLCIRCLE(prio)  GrpCreate(GRP_FILLCIRCLE, prio)
// GRP_TRIANGLE�ō쐬
#define GRPOBJ_TRIANGLE(prio)    GrpCreate(GRP_TRIANGLE, prio)
#define GRPOBJ_FAN(prio)         GrpCreate(GRP_FAN, prio)
#define GRPOBJ_FILLDONUT(prio)   GrpCreate(GRP_FILLDONUT, prio)

// �F�ݒ�
extern void GrpSetRGBA(sGRPOBJ *obj, REAL red, REAL green, REAL blue, REAL alpha);
// �F�ݒ�
extern void GrpSetRGBA4(sGRPOBJ *obj, sRGBA *c1, sRGBA *c2, sRGBA *c3, sRGBA *c4);
// ���W�ݒ�
extern void GrpSetPos(sGRPOBJ *obj, REAL x, REAL y);
// �T�C�Y�ݒ�
extern void GrpSetSize(sGRPOBJ *obj, REAL w, REAL h);
// �h�[�i�c�̃T�C�Y��ݒ�
extern void GrpSetDonutSize(sGRPOBJ *obj, REAL w1, REAL h1, REAL w2, REAL h2);
// ���C���̕`�搔��ݒ�
extern void GrpSetLineNum(sGRPOBJ *obj, int num);
// ��`�̊p�x��ݒ�
extern void GrpSetFanAngle(sGRPOBJ *obj, float angle);
// ���W�ݒ�(�S���_)
extern void GrpSetVtx(sGRPOBJ *obj, FVector2 *v1, FVector2 *v2, FVector2 *v3, FVector2 *v4);
// UV���W�ݒ�
extern void GrpSetUV(sGRPOBJ *obj, REAL x, REAL y);
// UV���W�ݒ�(�S���_�܂Ƃ߂Đݒ�)
extern void GrpSetUV4(sGRPOBJ *obj, FVector2 *v1, FVector2 *v2, FVector2 *v3, FVector2 *v4);
// �_����̕`��T�C�Y�ݒ�
extern void GrpSetDrawSize(sGRPOBJ *obj, REAL size);
// �X�P�[�����O�A��]���S��ݒ�
extern void GrpSetCenter(sGRPOBJ *obj, REAL x, REAL y);
// �X�P�[���l��ݒ�
extern void GrpSetScale(sGRPOBJ *obj, REAL x, REAL y);
// ��]�p��ݒ�
extern void GrpSetRot(sGRPOBJ *obj, REAL r);
// �t���b�v�ݒ�
extern void GrpSetFlip(sGRPOBJ *obj, BOOL flip_h, BOOL flip_v);
// �u�����f�B���O���[�h�ݒ�
extern void GrpSetBlendMode(sGRPOBJ *obj, int mode);
// �e�N�X�`����ݒ�
extern void GrpSetTexture(sGRPOBJ *obj, sTexture *tex);
// �G�C���A�V���O���[�h��ύX
extern void GrpSetSmooth(sGRPOBJ *obj, BOOL smooth);
// �G�C���A�V���O���[�h���擾
extern BOOL GrpGetSmooth(sGRPOBJ *obj);
// �t�B���^�����O���[�h��ύX
extern void GrpSetFilter(sGRPOBJ *obj, BOOL filter);
// �t�B���^�����O���[�h���擾
extern BOOL GrpGetFilter(sGRPOBJ *obj, BOOL filter);

#ifdef DEBUG
extern int GrpGetDrawPrimNum(void);
#else
#define GrpGetDrawPrimNum()
#endif


#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================

