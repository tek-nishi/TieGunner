//
// ïêäÌèàóù
//

//==============================================================
#ifndef NN_WEAPON_H
#define NN_WEAPON_H
//==============================================================

#include "co_common.h"
#include "co_obj.h"
#include "co_objlink.h"
#include "co_param.h"

#ifdef __cplusplus
extern              "C"
{
#endif


enum enumWEAPON_KIND {
	WEAPON_KIND_NONE = -1,
	WEAPON_KIND_SHOT,
	WEAPON_KIND_BOMB,
	WEAPON_KIND_LAZER,
	WEAPON_KIND_THUNDER,
	WEAPON_KIND_EXTRA,

	//-------------------
	WEAPON_KIND_MAX
};
	
enum enumWEAPON_LOST {
	WEAPON_LOST_NONE,
	WEAPON_LOST_PLAYER,
	WEAPON_LOST_ENEMY,
	WEAPON_LOST_BOSS,
};


typedef struct {
	sOBJ *obj;
	int index;
	float r, dr;
	float distance;
	BOOL shot;
	u_int target_flag;
} WeaponLink;


extern sOBJ *WeaponCreate(int type, int level, sParam *param);
extern void WeaponFix(sOBJ *obj, sOBJ *child, sLink *weapons, int weapon_max, float dist);
extern void WeaponFixOne(sOBJ *obj, sOBJ *child, sLink *weapons, float dist);
extern void WeaponUptate(sOBJ *obj, sLink *weapons, BOOL shot, u_int target_flag);
extern void WeaponFreeOne(sOBJ *obj, sLink *weapons, int prio, int lost_time);
extern void WeaponFreeAll(sLink *weapons, int lost_time);
extern void WeaponFixToObj(sOBJ *obj, sOBJ *target, int prio);
extern int WeaponUseOne(sOBJ *obj, sLink *weapons, int prio);

#ifdef __cplusplus
}
#endif

//==============================================================
#endif
//==============================================================
