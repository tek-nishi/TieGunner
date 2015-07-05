//
// ホーミングレーザー
//

//==============================================================
#ifndef NN_LAZER_H
#define NN_LAZER_H
//==============================================================

#include "co_common.h"
#include "co_param.h"

#ifdef __cplusplus
extern              "C"
{
#endif

extern int LazerCreate(char *id_str, FVector2 *pos, FVector2 *vct, float r, u_int target, sParam *param, int level, float powofs);

#ifdef __cplusplus
}
#endif

//==============================================================
#endif
//==============================================================
