//
//	�ėp�p�����[�^�Ǘ�
//

//==============================================================
#ifndef CO_PARAM_H
#define CO_PARAM_H
//==============================================================

#include "co_common.h"
#include "co_texture.h"
#include "co_sound.h"

#ifdef __cplusplus
extern              "C"
{
#endif

/********************************************/
/*             �萔�E�}�N���錾             */
/********************************************/
// �p�����[�^�̌^�ꗗ
enum enmPARAM_TYPE {
	PARAM_NONE = -1,							// �f�[�^����

	PARAM_REAL,
	PARAM_V2,
	PARAM_V3,
	PARAM_V4,
	PARAM_STR,

	PARAM_TEX,
	PARAM_SND,
};


/********************************************/
/*                �\���̐錾                */
/********************************************/
typedef struct _sParam sParam;


/********************************************/
/*              �O���[�o���ϐ�              */
/********************************************/


/********************************************/
/*              �O���[�o���֐�              */
/********************************************/
// ������
extern void ParamInit(void);
// �I��
extern void ParamFin(void);

// �ǂݍ���
extern sParam *ParamRead(char *file);
// �����o��
extern void ParamWrite(char *file, sParam *param);
// �f�[�^�j��
extern void ParamDestroy(sParam *param);
// �S�f�[�^�j��
extern void ParamDestroyAll(void);

// �p�����[�^�����݂��邩�`�F�b�N
extern BOOL ParamIsExists(sParam *param, char *id_str);
// �p�����[�^�̌^���擾
extern int ParamGetType(sParam *param, char *id_str);
// ������p�����[�^�擾
extern char *ParamGetStr(sParam *param, char *id_str);
// REAL�^�p�����[�^�擾
extern REAL ParamGetReal(sParam *param, char *id_str);
// FVector2�^�p�����[�^�擾
extern FVector2 *ParamGetFVec2(sParam *param, char *id_str);
// FVector3�^�p�����[�^�擾
extern FVector3 *ParamGetFVec3(sParam *param, char *id_str);
// FVector4�^�p�����[�^�擾
extern FVector4 *ParamGetFVec4(sParam *param, char *id_str);
// �e�N�X�`���p�����[�^�擾
extern sTexture *ParamGetTex(sParam *param, char *id_str);
extern SndObj *ParamGetSnd(sParam *param, char *id_str);

// ������p�����[�^�ύX
extern BOOL ParamSetStr(sParam *param, char *id_str, char *str);
// REAL�^�p�����[�^�ύX
extern BOOL ParamSetReal(sParam *param, char *id_str, REAL r);
// FVector2�^�p�����[�^�ύX
extern BOOL ParamSetFVec2(sParam *param, char *id_str, FVector2 *vec);
// FVector3�^�p�����[�^�ύX
extern BOOL ParamSetFVec3(sParam *param, char *id_str, FVector3 *vec);
// FVector4�^�p�����[�^�ύX
extern BOOL ParamSetFVec4(sParam *param, char *id_str, FVector4 *vec);

#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================

