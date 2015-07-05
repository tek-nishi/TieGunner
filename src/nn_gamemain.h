//
// ƒŒƒxƒ‹’²®‚Æ“G‚ÌoŒ»ˆ—
//

//==============================================================
#ifndef NN_GAMEMAIN_H
#define NN_GAMEMAIN_H
//==============================================================

#include "co_common.h"
#include "co_obj.h"
#include "co_param.h"

#ifdef __cplusplus
extern              "C"
{
#endif

extern void GameMainStart(sParam *param);
extern int GameMainGetEnemyLevel(void);
extern int GameMainGetBossLevel(void);
extern void GameMainPlayerGetWeapon(void);
extern int GameMainPostMessage(int msg, int lParam, int rParam);
extern sOBJ *GameMainWeaponEntry(void);


#ifdef __cplusplus
}
#endif

//==============================================================
#endif
//==============================================================
