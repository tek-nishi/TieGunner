//
// ボス上の武器処理
//

#include "nn_boss_weapon.h"
#include "co_graph.h"
#include "co_obj.h"
#include "co_random.h"
#include "co_misc.h"
#include "co_objlink.h"
#include "co_memory.h"
#include "co_stack.h"
#include "co_task.h"
#include "nn_main.h"
#include "nn_player.h"
#include "nn_effect.h"
#include "nn_weapon.h"
#include "nn_gamemain.h"
#include "nn_gamemisc.h"


typedef struct {
	sParam *param;									/* ゲームのパラメータ群 */
	sOBJ *parent;
	float disp_x, disp_y;

	FVector2 pos;
	FVector2 vct;
	float dir_vct;
	float dir;
	float default_dir;

	float speed;
	float decay;
	float rot_speed;
	float rot_max;

	float radius;

	sLink *weapons;
	int weapon_max;

	int power;
	int dmg_nohit;
	int interval;
	int death_effect;
	int death_disp;

	BOOL shot_exists;
	FVector2 target_dist;
	float target_angle;
	FVector2 shot_range;
	float shot_angle;
	
	float distance;
	float target_dir;
	float dr;

	BOOL shot_in;									/* TRUE:相手が見えている */
	int shot_delay;									/* タメ時間 */
	int shot_time;									/* SHOT時間 */
	int shot_int;
	int shot_type;
	float forword;									/* 先読み時間 */
	
	BOOL death;
	BOOL disp;
	BOOL parent_death;
	float red_eye;
	int dmg_col_timer;
	sRGBA col;

	int boss_level;
} ObjVar;


/* 武器の決定 */
static void setShotType(ObjVar *var, sParam *param)
{
	int num;
	int rand_max = 0;
	for(num = 0; ; num += 1)
	{
		char id_str[ID_MAXLEN];
		sprintf(id_str, "shot_type.%d", num + 1);
		if(!ParamIsExists(param, id_str)) break;

		FVector3 *vct = ParamGetFVec3(param, id_str);
		int val = vct->y + vct->z * var->boss_level;
		if(val < 0) val = 0;
		rand_max += val;
	}
	
	if(!num)
	{
		var->shot_type = ParamGetReal(param, "shot_type");
	}
	else
	{
		int rand = RndICH(RND_CH1, rand_max);
		for(int i = 0; i < num; i += 1)
		{
			char id_str[ID_MAXLEN];
			sprintf(id_str, "shot_type.%d", i + 1);
			FVector3 *vct = ParamGetFVec3(param, id_str);
			int val = vct->y + vct->z * var->boss_level;
			if(val < 0) val = 0;
			if(rand < val)
			{
				var->shot_type = vct->x;
				break;
			}
			rand -= val;
		}
	}
}


/* ターゲットとの情報を更新 */
static void updateTargetInfo(sOBJ *obj, sOBJ *target, ObjVar *var, sParam *param)
{
	if(target)
	{
		FVector2 pos;
		SubV2d(&pos, ObjGetPos(target), ObjGetPos(obj));

		FVector2 *vct = ObjGetVct(target);
		pos.x += vct->x * var->forword;
		pos.y += vct->y * var->forword;

		var->distance = MathLength(&pos);
		var->target_dir = MathVctAngleY(&pos);
		var->dr = DifAngle(var->target_dir, ObjGetDir(obj));
	}
}

/* 各種判定 */
static BOOL isTargetInView(ObjVar *var)
{
	return (var->distance < var->target_dist.x) || ((var->distance < var->target_dist.y) && (fabsf(var->dr) < var->target_angle * 0.5f));
}

static BOOL isTargetShot(ObjVar *var, sParam *param)
{
	return (var->distance < var->shot_range.x) || ((var->distance < var->shot_range.y) && (fabsf(var->dr) < var->shot_angle / 2.0f));
}


static void moveToTarget(ObjVar *var)
{
	float dr = var->dr;
	if(fabsf(var->dr) > var->rot_speed)
	{
		dr = var->dr < 0.0f ? -var->rot_speed : var->rot_speed;
	}
	var->dir = NormalAngle(var->dir + dr);
	float rotate_max = NormalAngle(var->dir - var->default_dir);
	if(fabsf(rotate_max) > var->rot_max)
	{
		var->dir = NormalAngle(var->default_dir + (rotate_max > 0.0f ? var->rot_max : -var->rot_max));
	}
}

static void createShot(sOBJ *obj, ObjVar *var, sParam *param)
{
//	int type = ParamGetReal(param, "shot_type");
	int type = var->shot_type;
	int weapon_level = ParamGetReal(param, "shot_level");
	float dir = ObjGetDir(obj);
	FVector2 pos = *ParamGetFVec2(param, "shot_ofs");
	MathRotateXY(&pos, dir);
	pos.x += var->pos.x;
	pos.y += var->pos.y;

	char *type_name[] = {
		"bullet_b",
		"bullet_b",
		"bomb_b",
		"lazer_b",
		"thunder_b"
	};
	float powofs = 1.0f + (float)var->boss_level * ParamGetReal(var->param, "boss_shpow_lv");

	var->shot_int = CreateShot(obj, type, weapon_level, powofs, type_name[type + 1], OBJ_PLAYER, &pos, &var->vct, dir, var->param);
}


/* メインプロシージャ */
static int objProc(sOBJ *obj, sParam *param, int msg, int lParam, int rParam)
{
	int res = 0;

	ObjVar *var = (ObjVar *)ObjGetVar(obj, sizeof(ObjVar));
	switch(msg)
	{
	case MSG_CREATE:
		{
			var->boss_level = GameMainGetBossLevel();
			var->param = (sParam *)StkRefFrameP(0);
			var->parent = (sOBJ *)StkRefFrameP(1);
			var->radius = ParamGetReal(param, "radius");
			var->power = PowerConvertInterValue(ParamGetReal(param, "power") + (float)var->boss_level * ParamGetReal(var->param, "boss_pow_lv"));

			var->speed = ParamGetReal(param, "speed");
			var->decay = ParamGetReal(param, "decay");
			var->rot_speed = ANG2RAD(ParamGetReal(param, "rot_speed"));
			var->rot_max = ANG2RAD(ParamGetReal(param, "rot_max"));

			var->shot_exists = ParamIsExists(param, "target_dist");
			if(var->shot_exists)
			{
				var->target_dist = *ParamGetFVec2(param, "target_dist");
				var->target_angle = ANG2RAD(ParamGetReal(param, "target_angle"));
				var->forword = ParamGetReal(param, "shot_forword") * FRAME_RATE;
				var->shot_angle = ANG2RAD(ParamGetReal(param, "shot_angle") + (float)var->boss_level * ParamGetReal(var->param, "boss_shang_lv"));

				FVector2 *shot_range = ParamGetFVec2(param, "shot_range");
				float shot_dist = ParamGetReal(var->param, "boss_shdist_lv");
				var->shot_range.x = shot_range->x + (float)var->boss_level * shot_dist;
				var->shot_range.y = shot_range->y + (float)var->boss_level * shot_dist;
				setShotType(var, param);
			}

			var->weapons = ObjLinkCreate(sizeof(WeaponLink), 4, MEM_APP, TRUE);
			var->weapon_max = ParamGetReal(param, "weapon_max");

			FVector4 *col_vct = ParamGetFVec4(param, "col");
			SetRGBA(&var->col, col_vct->x, col_vct->y, col_vct->z, col_vct->w);

			var->disp = ParamGetReal(param, "disp");
			ObjSetRadius(obj, var->radius);
		}
		break;

	case MSG_KILL:
		{
			ObjLinkDestroy(var->weapons);
		}
		break;

	case MSG_STEP:
		{
			if(var->shot_exists && !var->death && !var->parent_death)
			{
				updateTargetInfo(obj, PlayerGetObj(), var, param);
				BOOL target_in = isTargetInView(var);
				BOOL shot_in = isTargetShot(var, param);
				if(!target_in)
				{
					var->dr = DifAngle(var->default_dir, var->dir);
					/* 見えない場合は初期位置へ戻す */
				}
				moveToTarget(var);

				if(shot_in)
				{
					FVector2 *shot_time = ParamGetFVec2(param, "shot_time");
					if(!var->shot_in)
					{
						var->shot_delay = shot_time->x;
					}
					else
					{
						if(var->shot_delay > 0)
						{
							var->shot_delay -= 1;
							if(var->shot_delay == 0)
							{
								var->shot_time = shot_time->y;
							}
						}
						if(!var->shot_delay && (var->shot_time > 0))
						{
							if(!var->shot_int)
							{
								createShot(obj, var, param);
							}
							if(!--var->shot_time)
							{
								var->shot_delay = shot_time->x;
							}
						}
					}
				}
				var->shot_in = shot_in;
				if(var->shot_int > 0) var->shot_int -= 1;
			}
			
/* 			ObjSetPos(obj, var->pos.x, var->pos.y); */
/* 			ObjSetDir(obj, var->dir); */
/* 			ObjSetVct(obj, var->vct.x, var->vct.y); */

			WeaponUptate(obj, var->weapons, FALSE, OBJ_BOSS);

			if(var->dmg_nohit > 0) var->dmg_nohit -= 1;
			if(var->dmg_col_timer > 0) var->dmg_col_timer -= 1;
		}
		break;

	case MSG_DRAW:
		{
			if(var->disp)
			{
				FVector2 *size = ParamGetFVec2(param, "size");
				FVector2 *center = ParamGetFVec2(param, "center");
				float y_ofs = 0.0f;
				if(var->death) y_ofs = size->y;

				sGRPOBJ *grp;
				grp = GRPOBJ_QUAD(PRIO_BOSS_WEAPON);
				GrpSetPos(grp, var->disp_x, var->disp_y);
				GrpSetRot(grp, ObjGetDir(obj));
				GrpSetSize(grp, size->x, size->y);
				GrpSetCenter(grp, center->x, center->y);
				GrpSetUV(grp, 0.0f, y_ofs);
				GrpSetTexture(grp, ParamGetTex(param, "texture"));
				GrpSetFilter(grp, TRUE);

				if(!var->death)
				{
					sRGBA col;
					if(var->dmg_col_timer > 0)
					{
						DmgHitColor(&col, var->dmg_col_timer, param);
					}
					else
					{
						col = var->col;
					}
					GrpSetRGBA(grp, col.red, col.green, col.blue, col.alpha);
				}
				

				if(!var->death)
				{
					grp = GRPOBJ_QUAD(PRIO_BOSS_WEAPON);
					GrpSetPos(grp, var->disp_x, var->disp_y);
					GrpSetRot(grp, ObjGetDir(obj));
					GrpSetSize(grp, size->x, size->y);
					GrpSetCenter(grp, center->x, center->y);
					GrpSetUV(grp, size->x, 0.0f);
					GrpSetTexture(grp, ParamGetTex(param, "texture"));
					GrpSetFilter(grp, TRUE);
					if(var->parent_death)
					{
						var->red_eye *= 0.9f;
					}
					else
					{
						var->red_eye = 0.5f + fabsf(sinf((float)g.time / 40.0f)) * 0.5f;
					}
					GrpSetRGBA(grp, var->red_eye, 0.0f, 0.0f, 1.0f);
				}
			}

#ifdef DEBUG
			if(isDispDebugInfo())
			{
				sGRPOBJ *grp;
				
				grp = GRPOBJ_CIRCLE(PRIO_DEBUG_PRINT);
				GrpSetPos(grp, var->disp_x, var->disp_y);
				GrpSetSize(grp, var->radius, var->radius);
				GrpSetLineNum(grp, 12);
				GrpSetRGBA(grp, 1.0f, 0.0f, 0.0f, 1.0f);
				GrpSetDrawSize(grp, 1.0f);

				if(var->shot_exists)
				{
					float dir = ObjGetDir(obj);

					grp = GRPOBJ_CIRCLE(PRIO_DEBUG_PRINT);
					GrpSetPos(grp, var->disp_x, var->disp_y);
					GrpSetSize(grp, var->target_dist.x, var->target_dist.x);
					GrpSetLineNum(grp, 20);
					GrpSetRGBA(grp, 0.0f, 0.5f, 1.0f, 1.0f);
					GrpSetDrawSize(grp, 1.0f);

					grp = GRPOBJ_FAN(PRIO_DEBUG_PRINT);
					GrpSetPos(grp, var->disp_x, var->disp_y);
					GrpSetSize(grp, var->target_dist.y, var->target_dist.y);
					GrpSetLineNum(grp, 12);
					GrpSetFanAngle(grp, var->target_angle);
					GrpSetRGBA(grp, 0.0f, 0.5f, 1.0f, 1.0f);
					GrpSetDrawSize(grp, 1.0f);
					GrpSetRot(grp, NormalAngle(dir - var->target_angle / 2.0f));

					grp = GRPOBJ_FAN(PRIO_DEBUG_PRINT);
					GrpSetPos(grp, var->disp_x, var->disp_y);
					GrpSetSize(grp, var->shot_range.y, var->shot_range.y);
					GrpSetLineNum(grp, 12);
					GrpSetFanAngle(grp, var->shot_angle);
					GrpSetRGBA(grp, 1.0f, 1.0f, 0.0f, 1.0f);
					GrpSetDrawSize(grp, 1.0f);
					GrpSetRot(grp, NormalAngle(dir - var->shot_angle / 2.0f));

					grp = GRPOBJ_CIRCLE(PRIO_DEBUG_PRINT);
					GrpSetPos(grp, var->disp_x, var->disp_y);
					GrpSetSize(grp, var->shot_range.x, var->shot_range.x);
					GrpSetLineNum(grp, 12);
					GrpSetRGBA(grp, 1.0f, 1.0f, 0.0f, 1.0f);
					GrpSetDrawSize(grp, 1.0f);
				}
			}
#endif
		}
		break;


	case MSG_GAME_ENEMY_SETUP:
		{
			var->pos = *ObjGetPos(obj);
			var->vct = *ObjGetVct(obj); 
			var->dir = ObjGetDir(obj);
			var->default_dir = var->dir;
		}
		break;

	case MSG_GAME_CAMERA:
		{
			FVector2 *pos = (FVector2 *)StkRefFrameP(0);
			float x = var->pos.x - pos->x;
			float y = var->pos.y - pos->y;
			var->disp_x = x;
			var->disp_y = y;
		}
		break;

	case MSG_GAME_BOSS_CHILD_UPDATE:
		{
			sOBJ *parent = (sOBJ *)StkRefFrameP(0);
			FVector2 ofs = *(FVector2 *)StkRefFrameP(1);
			float dir = ObjGetDir(parent);
			MathRotateXY(&ofs, dir);
			
			FVector2 *pos = ObjGetPos(parent);
			AddV2d(&var->pos, pos, &ofs);
			//	var->dir = NormalAngle(var->dir + dir);
			
			ObjSetPos(obj, var->pos.x, var->pos.y);
			ObjSetDir(obj, NormalAngle(var->dir + dir));
		}
		break;

	case MSG_GAME_BOSS_CHILD_KILL:
		{
			WeaponFreeAll(var->weapons, WEAPON_LOST_BOSS);
			ObjKillReq(obj);
		}
		break;

	case MSG_GAME_WEAPON_FIXED:
		{
			sOBJ *child = (sOBJ *)StkRefFrameP(0);
			WeaponFixOne(obj, child, var->weapons, ParamGetReal(param, "weapon_ofs"));
		}
		break;
		
	case MSG_GAME_BOSS_DESTROY:
		{
			sOBJ *parent = (sOBJ *)StkRefFrameP(0);
			if(var->parent == parent) var->parent_death = TRUE;
		}
		break;

	case MSG_GAME_DAMAGE:
		if((var->disp) && (!var->death) && (!var->parent_death) && (var->dmg_nohit == 0))
		{
			int power = ShotGetDamagePower();
			float radius = ShotGetDamageRadius();
			FVector2 *pos = ShotGetDamagePos();
/* 			FVector2 *vct = ShotGetDamageVct(); */
			if(lParam || MathCrossBox(&var->pos, var->radius, pos, radius))
			{
				res = TRUE;
				int damaged = PowerCalcDamage(&var->power, power);
				if(damaged > 0)
				{
					var->dmg_nohit = ParamGetReal(param, "dmg_nohit") * FRAME_RATE;

					if(var->power <= 0)
					{
						var->death = TRUE;
						ObjSetDeath(obj, TRUE);
#if 0
						StkMakeFrame();
						StkPushP(obj);					// 0
						ObjPostMsg(var->parent, MSG_GAME_BOSS_WEAPON_DEAD, 0, 0);
						StkDelFrame();
#endif
						StkMakeFrame();
						StkPushP(obj);					// 0
						TaskPostMsgAll(TASK_PRI_NONE, MSG_GAME_BOSS_WEAPON_DEAD, ParamGetReal(param, "score"), 0);
						StkDelFrame();
					}
					var->dmg_col_timer =  var->death ? 0 : ParamGetReal(param,"dmg_col_timer") * FRAME_RATE;

					EffectCreate(var->death ? "explo" : "hit", pos, &FVec2Zero, 0.0f, &FVec2One);
					if(var->death)
					{
						char *id_str = ParamGetStr(param, "dead_eft");
						EffectCreate(id_str, pos, &FVec2Zero, 0.0f, &FVec2One);
					}
				
					SndPlay(var->death ? "enemy_dead" : "shot_hit", 1.0f);
				}
			}
		}
		break;
	}
	
	return res;
}


sOBJ *BossWeaponCreate(char *id_str, u_int type, sOBJ *parent, FVector2 *pos, float dir, sParam *param)
{
	StkMakeFrame();
	StkPushP(param);								// 0
	StkPushP(parent);								// 1
	sOBJ *obj = ObjCreate(id_str, type, objProc, 0, 0);
	StkDelFrame();
	
	ObjSetPos(obj, pos->x, pos->y);
	ObjSetDir(obj, dir);
	ObjSetVct(obj, 0.0f, 0.0f);
	ObjPostMsg(obj, MSG_GAME_ENEMY_SETUP, 0, 0);

	return obj;
}
