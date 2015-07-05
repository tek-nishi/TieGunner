//
// エフェクト
//

//==============================================================
#ifndef NN_EFFECT_H
#define NN_EFFECT_H
//==============================================================

#include "co_common.h"

#ifdef __cplusplus
extern              "C"
{
#endif

extern void EffectStart(void);
extern void EffectCreate(char *id_str, FVector2 *pos, const FVector2 *vct, float dir, const FVector2 *scale);
extern void EffectDeleteAll(void);


#ifdef __cplusplus
}
#endif

//==============================================================
#endif
//==============================================================
