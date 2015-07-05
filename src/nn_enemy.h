//
// エネミー処理
//

//==============================================================
#ifndef NN_ENEMY_H
#define NN_ENEMY_H
//==============================================================

#include "co_common.h"
#include "co_obj.h"
#include "co_param.h"

#ifdef __cplusplus
extern              "C"
{
#endif

extern sOBJ *EnemyCreate(char *id_str, FVector2 *pos, float dir, BOOL shot, sParam *param);
extern void EnemySetTarget(sOBJ *obj, sOBJ *target);
extern void EnemySetFormation(sOBJ *obj, sOBJ *target, FVector2 *ofs);


#ifdef __cplusplus
}
#endif

//==============================================================
#endif
//==============================================================
