//
// ボス処理
//

#include "nn_boss.h"
#include "co_graph.h"
#include "co_obj.h"
#include "co_random.h"
#include "co_misc.h"
#include "co_objlink.h"
#include "co_memory.h"
#include "co_stack.h"
#include "co_task.h"
#include "nn_main.h"
#include "nn_gamemain.h"
#include "nn_player.h"
#include "nn_effect.h"
#include "nn_weapon.h"
#include "nn_boss_weapon.h"
#include "nn_gamemisc.h"
#include "nn_camera.h"


typedef struct {
	FVector2 ofs;
	sOBJ *obj;
} ChildObj;


typedef struct {
	sParam *param;									/* ゲームのパラメータ群 */
	float disp_x, disp_y;

	FVector2 pos;
	FVector2 vct;
	float dir_vct;
	float dir;

	float speed;
	float decay;
	float rot_speed;
	float rot_decay;

	float radius;

	sLink *childs;
	int child_max;

	int power;
	int dmg_nohit;
	int interval;
	int death_disp;

	int move_int;
	float move_dir;
	float move_speed;

	FVector2 target_dist;
	float target_angle;
	
	float distance;
	float target_dir;
	float dr;

	int runaway_sample;
	float runaway_dir;
	BOOL runaway;

	BOOL shot_active;
	int shot_delay;
	int shot_time;
	int shot_int;

	BOOL death;
	BOOL disp;
	float red_eye;
	int dmg_col_timer;
	sRGBA col;

	int boss_level;
} ObjVar;


static void bossFixToObj(sOBJ *obj, ObjVar *var, sOBJ *child, FVector2 *ofs)
{
	if(ObjLinkGetNum(var->childs) < var->child_max)
	{
		void *p = ObjLinkNew(var->childs);
		ASSERT(p);
		ChildObj *childObj = (ChildObj *)p;
		childObj->obj = child;
		childObj->ofs = *ofs;
	}
}

static void bossChildUpdate(sOBJ *obj, ObjVar *var)
{
	void *p = ObjLinkGetTop(var->childs);
	while(p)
	{
		ChildObj *childObj = (ChildObj *)p;
		StkMakeFrame();
		StkPushP(obj);								// 0
		StkPushP(&childObj->ofs);					// 1
		ObjPostMsg(childObj->obj, MSG_GAME_BOSS_CHILD_UPDATE, 0, 0);
		StkDelFrame();
		
		p = ObjLinkGetNext(p);
	}
}

static void bossChildDelete(sOBJ *obj, ObjVar *var)
{
	void *p = ObjLinkGetTop(var->childs);
	while(p)
	{
		ChildObj *childObj = (ChildObj *)p;
		ObjPostMsg(childObj->obj, MSG_GAME_BOSS_CHILD_KILL, 0, 0);
		p = ObjLinkGetNext(p);
	}
}

/* ターゲットとの情報を更新 */
static void updateTargetInfo(sOBJ *obj, sOBJ *target, ObjVar *var, sParam *param)
{
	if(target)
	{
		FVector2 pos;
		SubV2d(&pos, ObjGetPos(target), ObjGetPos(obj));
		var->distance = MathLength(&pos);
		var->target_dir = MathVctAngleY(&pos);
		var->dr = DifAngle(var->target_dir, var->dir);
	}
}

/* 他のBOSSとは接近しないようにする */
static void bossRunaway(sOBJ *obj, ObjVar *var, sParam *param)
{
	FVector2 *dist = ParamGetFVec2(param, "runaway_dist");
	float angle = ANG2RAD(ParamGetReal(param, "runaway_angle")) / 2.0f;

	sOBJ *near_obj = 0;
	float near_dist = 1000.0f;						/* 適当に大きな値 */
	sOBJ *target = 0;
	/* FIXME:列挙処理が重い */
	while(target = ObjGetNext(target, OBJ_BOSS, 0))
	{
		if(obj != target)
		{
			FVector2 pos;
			SubV2d(&pos, ObjGetPos(target), &var->pos);

			float d = MathLength(&pos);
			float dr = DifAngle(MathVctAngleY(&pos), var->dir);
			if((d < dist->x) || ((d < dist->y) && (fabsf(dr) < angle)))
			{
				if(d < near_dist)
				{
					near_obj = target;
					near_dist = d;
				}
			}
		}
	}

	var->runaway = (near_obj != 0);
	if(near_obj)
	{
		FVector2 pos;
		FVector2 *vct = ObjGetVct(near_obj);
		float fwd = ParamGetReal(param, "runaway_fwd") * FRAME_RATE;
		float run_dir = ANG2RAD(ParamGetReal(param, "runaway_dir"));
		
		SubV2d(&pos, ObjGetPos(near_obj), &var->pos);
		float dr = DifAngle(MathVctAngleY(&pos), var->dir);
		pos.x += vct->x *= fwd;
		pos.y += vct->y *= fwd;
		var->runaway_dir = NormalAngle(var->dir + ((dr < 0.0f) ? run_dir : -run_dir));
		var->runaway = TRUE;
	}
}


/* 各種判定 */
static BOOL isTargetInView(ObjVar *var)
{
	return (var->distance < var->target_dist.x) || ((var->distance < var->target_dist.y) && (fabsf(var->dr) < var->target_angle * 0.5f));
}


#if 0
static BOOL isTargetShot(ObjVar *var, sParam *param)
{
	float shot_range = ParamGetReal(param, "shot_range");
	float shot_angle = ANG2RAD(ParamGetReal(param, "shot_angle"));
	
	return (var->distance < shot_range) && (fabsf(var->dr) < shot_angle * 0.5f);
}
#endif


static void moveToTarget(ObjVar *var, float dr, float speed, float rot_speed)
{
	if(fabsf(dr) > var->rot_speed)
	{
		dr = dr < 0.0f ? -var->rot_speed : var->rot_speed;
	}
	float rot_vct = rot_speed * (dr / var->rot_speed);
	var->dir_vct = var->dir_vct * var->rot_decay + rot_vct;
	var->dir = NormalAngle(var->dir + var->dir_vct);

	FVector2 vct;
	MathCalcVector(&vct, var->dir, speed);
	var->vct.x = var->vct.x * var->decay + vct.x;
	var->vct.y = var->vct.y * var->decay + vct.y;
	var->pos.x += var->vct.x;
	var->pos.y += var->vct.y;

}

static void randomMove(ObjVar *var, sParam *param)
{
	if(var->move_int > 0) var->move_int -= 1;
	if(var->move_int == 0)
	{
		FVector2 *value;
		value = ParamGetFVec2(param, "move_time");
		var->move_int = value->x * FRAME_RATE + (value->y - value->x) * FRAME_RATE * RndCH(RND_CH1);

		float dir = ANG2RAD(ParamGetReal(param, "move_dir"));
		var->move_dir = NormalAngle(dir * RndmCH(RND_CH1) + var->dir);

		value = ParamGetFVec2(param, "move_speed");
		float move_speed = value->x + (value->y - value->x) * RndCH(RND_CH1);
		var->move_speed = var->speed * move_speed;

		PRINTF("time:%d dir:%f speed:%f\n", var->move_int, var->move_dir, var->move_speed);
	}

	float dr = DifAngle(var->move_dir, var->dir);
	moveToTarget(var, dr, var->move_speed, var->rot_speed);
}

static void deadMove(sOBJ *obj, ObjVar *var, sParam *param)
{
	if(!(var->interval % 2))
	{
		FVector2 pos = { 0.0f, 0.0f };

		float radius = ParamGetReal(param, "death_eft_r");
		float dist = radius * Rnd();
		float dir = PI * Rndm();
		MathCalcVector(&pos, dir, dist);
		pos.x += var->pos.x;
		pos.y += var->pos.y;
		int eft_rnd = RndI(100);
		EffectCreate((eft_rnd < 50) ? "hit" : "hit2", &pos, &FVec2Zero, 0.0f, &FVec2One);
		SndPlay("enemy_dead", 0.8f);
	}

	if(!(var->interval -= 1))
	{
		EffectCreate("boss_e1", &var->pos, &FVec2Zero, 0.0f, &FVec2One);
		EffectCreate("boss_e2", &var->pos, &FVec2Zero, 0.0f, &FVec2One);
		EffectCreate("boss_e3", &var->pos, &FVec2Zero, 0.0f, &FVec2One);
		SndPlay("boss_dead", 1.0f);
		bossChildDelete(obj, var);

		StkMakeFrame();
		StkPushP(obj);								// 0
		ObjPostMsgAll(OBJ_WEAPON, MSG_GAME_BOSS_OBJKILL, FALSE, 0, 0);
		StkDelFrame();
		
		ObjKillReq(obj);
	}
	moveToTarget(var, 0.0f, 0.0f, 0.0f);
}

#if 0
static void createShot(sOBJ *obj, ObjVar *var, sParam *param)
{
	for(int i = 0; ; i += 1)
	{
		char id_str[ID_MAXLEN];
		sprintf(id_str, "%d.shot", i + 1);
		if(!ParamIsExists(param, id_str)) break;
						
		FVector3 *value = ParamGetFVec3(param, id_str);
		sprintf(id_str, "%d.shot_type", i + 1);
		FVector2 *type = ParamGetFVec2(param, id_str);
		int weapon_level = type->y;

		FVector2 pos;
		SetV2d(&pos, value->x, value->y);
		MathRotateXY(&pos, var->dir);
		pos.x += var->pos.x;
		pos.y += var->pos.y;
		float dir = NormalAngle(var->dir + ANG2RAD(value->z));

		int shot_int = 60;
		switch((int)type->x)
		{
		case WEAPON_KIND_NONE:
		case WEAPON_KIND_SHOT:
			{
				shot_int = BulletCreate("bullet_p", &pos, &var->vct, dir, OBJ_PLAYER, var->param, weapon_level);
			}
			break;

		case WEAPON_KIND_BOMB:
			{
				shot_int = BombCreate("bomb_p", &pos, &var->vct, dir, OBJ_PLAYER, var->param, weapon_level);
			}
			break;

		case WEAPON_KIND_LAZER:
			{
				shot_int = LazerCreate("lazer_p", &pos, &var->vct, dir, OBJ_PLAYER, var->param, weapon_level);
			}
			break;
		}
		var->shot_int = shot_int;
			
	}
}
#endif


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
			var->radius = ParamGetReal(param, "radius");
			var->power = PowerConvertInterValue(ParamGetReal(param, "power") + (float)var->boss_level * ParamGetReal(var->param, "boss_pow_lv"));

			var->speed = ParamGetReal(param, "speed");
			var->decay = ParamGetReal(param, "decay");
			var->rot_speed = ANG2RAD(ParamGetReal(param, "rot_speed"));
			var->rot_decay = ParamGetReal(param, "rot_decay");

			var->target_dist = *ParamGetFVec2(param, "target_dist");
			var->target_angle = ANG2RAD(ParamGetReal(param, "target_angle"));

			var->childs = ObjLinkCreate(sizeof(ChildObj), 8, MEM_APP, TRUE);
			var->child_max = ParamGetReal(param, "child_max");

			FVector4 *col_vct = ParamGetFVec4(param, "col");
			SetRGBA(&var->col, col_vct->x, col_vct->y, col_vct->z, col_vct->w);

			var->disp = TRUE;

			/* 砲台設置 */
			int battery_num;
			for(battery_num = 0; ; battery_num += 1)
			{
				char id_str[ID_MAXLEN];
				sprintf(id_str, "%d.battery", battery_num + 1);
				if(!ParamIsExists(param, id_str)) break;

				FVector3 *val = ParamGetFVec3(param, id_str);
				FVector2 pos;
				float dir;
				pos.x = val->x;
				pos.y = val->y;
				dir = ANG2RAD(val->z);
				
				sprintf(id_str, "%d.battery_n", battery_num + 1);
				sOBJ *child = BossWeaponCreate(ParamGetStr(param, id_str), OBJ_BOSS_WEAPON, obj, &pos, dir, var->param);
				bossFixToObj(obj, var, child, &pos);
			}

			/* アイテム生成 */
			for(int i = 0; ; i += 1)
			{
				char id_str[ID_MAXLEN];
				sprintf(id_str, "%d.weapon", i + 1);
				if(!ParamIsExists(param, id_str)) break;

				FVector4 *value = ParamGetFVec4(param, id_str);
				FVector2 ofs;
				SetV2d(&ofs, value->x, value->y);
				sOBJ *child = BossWeaponCreate("boss_tail", OBJ_BOSS_TAIL, obj, &ofs, ANG2RAD(value->z), var->param);
				bossFixToObj(obj, var, child, &ofs);
				int item_num = value->w;
				for(int h = 0; h < item_num; h += 1)
				{
					sOBJ *weapon = GameMainWeaponEntry();
					WeaponFixToObj(weapon, child, PRIO_BOSS + 1);
				}
			}
			ObjSetRadius(obj, var->radius);
		}
		break;

	case MSG_KILL:
		{
			ObjLinkDestroy(var->childs);
		}
		break;

	case MSG_STEP:
		{
			if(GameFieldRange(&var->pos) && !var->death)
			{
				var->vct = FVec2Zero;
				/* ラップした直後、他のBOSSを見失って回避できなくなるのを抑え込む */
			}

			if(var->death)
			{
				deadMove(obj, var, param);
			}
			else
			{
				updateTargetInfo(obj, PlayerGetObj(), var, param);

				BOOL target_in = isTargetInView(var);
//				BOOL shot_in = isTargetShot(var, param);

				if(--var->runaway_sample < 0)
				{
					var->runaway_sample = ParamGetReal(param, "runaway_sample") * FRAME_RATE;
					bossRunaway(obj, var, param);
				}

				if(var->runaway)
				{
					float dr = DifAngle(var->runaway_dir, var->dir);
					moveToTarget(var, dr, var->speed, var->rot_speed);
				}
				else
				if(target_in)
				{
					moveToTarget(var, var->dr, var->speed, var->rot_speed);
					var->move_int = 0;
				}
				else
				{
					randomMove(var, param);
				}
				
#if 0
				if(shot_in)
				{
					FVector2 *shot_time = ParamGetFVec2(param, "shot_time");
					if(!var->shot_active)
					{
						var->shot_delay = shot_time->x;
						var->shot_time = 0;
					}

					if(var->shot_time && !var->shot_int)
					{
						createShot(obj, var, param);
					}

					if(var->shot_delay > 0)
					{
						var->shot_delay -= 1;
						if(var->shot_delay == 0)
						{
							var->shot_time = shot_time->y;
						}
					}
					else
					{
						var->shot_time -= 1;
						if(var->shot_time <= 0)
						{
							var->shot_delay = shot_time->x;
						}
					}
				}
				var->shot_active = shot_in;
				if(var->shot_int > 0) var->shot_int -= 1;
#endif
			}

			ObjSetPos(obj, var->pos.x, var->pos.y);
			ObjSetDir(obj, var->dir);
			ObjSetVct(obj, var->vct.x, var->vct.y);

			bossChildUpdate(obj, var);

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

				sGRPOBJ *grp;
				grp = GRPOBJ_QUAD(PRIO_BOSS);
				GrpSetPos(grp, var->disp_x, var->disp_y);
				GrpSetRot(grp, var->dir);
				GrpSetSize(grp, size->x, size->y);
				GrpSetCenter(grp, center->x, center->y);
				GrpSetUV(grp, 0.0f, 0.0f);
				GrpSetTexture(grp, ParamGetTex(param, "body"));
				GrpSetFilter(grp, TRUE);

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

				grp = GRPOBJ_QUAD(PRIO_BOSS);
				GrpSetPos(grp, var->disp_x, var->disp_y);
				GrpSetRot(grp, var->dir);
				GrpSetSize(grp, size->x, size->y);
				GrpSetCenter(grp, center->x, center->y);
				GrpSetUV(grp, 0.0f, 0.0f);
				GrpSetTexture(grp, ParamGetTex(param, "eye"));
				GrpSetFilter(grp, TRUE);
				if(var->death)
				{
					var->red_eye *= 0.9f;
				}
				else
				{
					var->red_eye = 0.5f + fabsf(sinf((float)g.time / 40.0f)) * 0.5f;
				}
				GrpSetRGBA(grp, var->red_eye, 0.0f, 0.0f, 1.0f);
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
				GrpSetRot(grp, NormalAngle(var->dir - var->target_angle / 2.0f));

#if 0
				float shot_range = ParamGetReal(param, "shot_range");
				float shot_angle = ANG2RAD(ParamGetReal(param, "shot_angle"));
				grp = GRPOBJ_FAN(PRIO_DEBUG_PRINT);
				GrpSetPos(grp, var->disp_x, var->disp_y);
				GrpSetSize(grp, shot_range, shot_range);
				GrpSetLineNum(grp, 12);
				GrpSetFanAngle(grp, shot_angle);
				GrpSetRGBA(grp, 1.0f, 1.0f, 0.0f, 1.0f);
				GrpSetDrawSize(grp, 1.0f);
				GrpSetRot(grp, NormalAngle(var->dir - shot_angle / 2.0f));
#endif
			}
#endif
		}
		break;


	case MSG_GAME_ENEMY_SETUP:
		{
			var->pos = *ObjGetPos(obj);
			var->vct = *ObjGetVct(obj);
			var->dir = ObjGetDir(obj);

			bossChildUpdate(obj, var);
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
		
	case MSG_GAME_DAMAGE:
		if((!var->death) && (var->dmg_nohit == 0))
		{
			int power = ShotGetDamagePower();
			float radius = ShotGetDamageRadius();
			FVector2 *pos = ShotGetDamagePos();
			FVector2 *vct = ShotGetDamageVct();
			if(lParam || MathCrossBox(&var->pos, var->radius, pos, radius))
			{
				res = TRUE;
				int damaged = PowerCalcDamage(&var->power, power);
				if(damaged > 0)
				{
					var->dmg_nohit = ParamGetReal(param, "dmg_nohit") * FRAME_RATE;

					EffectCreate("hit", pos, &FVec2Zero, 0.0f, &FVec2One);
					SndPlay("shot_hit", 1.0f);

					if(var->power <= 0)
					{
						ObjSetDeath(obj, TRUE);
						var->death = TRUE;
						var->interval = ParamGetReal(param, "death_interval") * FRAME_RATE;

						var->speed = 0.0f;
						var->decay = 0.998f;
						var->dir_vct *= 0.8f;

						/* 他のタスクに通知 */
						StkMakeFrame();
						StkPushP(obj);					// 0
						TaskPostMsgAll(TASK_PRI_NONE, MSG_GAME_BOSS_DESTROY, ParamGetReal(param, "score"), 0);
						StkDelFrame();
					}
					var->dmg_col_timer = ParamGetReal(param, var->death ? "death_col_t" : "dmg_col_timer") * FRAME_RATE;
				}
			}
		}
		break;


#if 0
	case MSG_GAME_BOSS_WEAPON_DEAD:
		{
			sOBJ *child = (sOBJ *)StkRefFrameP(0);
			void *p = ObjLinkGetTop(var->childs);
			while(p)
			{
				ChildObj *childObj = (ChildObj *)p;
				if(childObj->obj == child)
				{
					ObjLinkDel(var->childs, child);
					break;
				}
				p = ObjLinkGetNext(p);
			}
		}
		break;
#endif
	}
	
	return res;
}


sOBJ *BossCreate(char *id_str, FVector2 *pos, float dir, sParam *param)
{
	StkMakeFrame();
	StkPushP(param);								// 0
	sOBJ *obj = ObjCreate(id_str, OBJ_BOSS, objProc, 0, 0);
	StkDelFrame();
	
	ObjSetPos(obj, pos->x, pos->y);
	ObjSetDir(obj, dir);
	ObjSetVct(obj, 0.0f, 0.0f);
	ObjPostMsg(obj, MSG_GAME_ENEMY_SETUP, 0, 0);
	
	return obj;
}

