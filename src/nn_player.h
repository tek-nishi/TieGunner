//
// ÉvÉåÉCÉÑÅ[èàóù
//

//==============================================================
#ifndef NN_PLAYER_H
#define NN_PLAYER_H
//==============================================================

#include "co_common.h"
#include "co_param.h"
#include "co_obj.h"

#ifdef __cplusplus
extern              "C"
{
#endif

extern sOBJ *PlayerCreate(sParam *param);
extern sOBJ *PlayerGetObj(void);
extern FVector2 *PlayerGetPos(void);
extern REAL PlayerGetDir(void);
extern FVector2 *PlayerGetVct(void);
extern void PlayerSetPos(FVector2 *pos);
extern void PlayerSetVct(FVector2 *vct);
extern void PlayerSetDir(float dir);
extern int PlayerGetWeaponNum(void);
extern int PlayerGetPower(void);

#ifdef __cplusplus
}
#endif

//==============================================================
#endif
//==============================================================
