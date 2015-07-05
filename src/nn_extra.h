//
// エクストラ(画面内の全キャラにダメージ)
//

//==============================================================
#ifndef NN_EXTRA_H
#define NN_EXTRA_H
//==============================================================

#include "co_common.h"
#include "co_param.h"

#ifdef __cplusplus
extern              "C"
{
#endif

extern int ExtraCreate(char *id_str, FVector2 *pos, FVector2 *vct, float r, u_int target, sParam *param, int level, float powofs);

#ifdef __cplusplus
}
#endif

//==============================================================
#endif
//==============================================================
