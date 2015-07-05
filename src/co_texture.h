
//==============================================================
#ifndef CO_TEXTURE_H
#define CO_TEXTURE_H
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
typedef struct _sTexture sTexture;


/********************************************/
/*              �O���[�o���ϐ�              */
/********************************************/


/********************************************/
/*              �O���[�o���֐�              */
/********************************************/
// ������
extern void TexInit(void);
// �I��
extern void TexFin(void);

// �e�N�X�`���ǂݍ���
extern sTexture *TexRead(char *file);
// �e�N�X�`���̃N���[��
extern sTexture *TexClone(sTexture *texture);
// �e�N�X�`���j��
extern void TexDestroy(sTexture *texture);

// �n���h�����擾
extern sTexture *TexGetFromName(char *id_str);
// ���ʎq���擾
extern char *TexGetName(sTexture *texture);
// �T�C�Y���擾
extern void TexGetSize(FVector2 *res, sTexture *texture);
// UV�ő�l���擾
extern void TexGetUV(FVector2 *res, sTexture *texture);

// �e�N�X�`����L���ɂ���
extern void TexBind(sTexture *texture);


#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================

