//
// エネミー処理
//

#include "nn_enemy.h"
#include "co_graph.h"
#include "co_obj.h"
#include "co_random.h"
#include "co_misc.h"
#include "co_objlink.h"
#include "co_memory.h"
#include "co_task.h"
#include "co_stack.h"
#include "co_sound.h"
#include "nn_player.h"
#include "nn_weapon.h"
#include "nn_main.h"
#include "nn_effect.h"
#include "nn_gamemain.h"
#include "nn_gamemisc.h"


#define SHOT_NUM_MAX		4						/* 同時ショット最大数 */
#define ENEMY_WEAPON_MAXNUM 98


enum {
	ENEMY_MOVE_RANDOM,								/* ランダム→ターゲット→ランダム… */
	ENEMY_MOVE_ROTATE_MOVE,							/* 回転→移動→回転→移動… */
	ENEMY_MOVE_FORMATION,							/* 隊列移動 */

	ENEMY_DEATH										/* 破壊された */
};


typedef struct {
	sParam *param;									/* ゲームのパラメータ群 */

	FVector2 pos;
	FVector2 vct;
	float dir;
	float dir_vct;
	BOOL pos_wrap;									/* TRUE:座標を補正した */

	const FVector2 *speed;
	float rot_speed;
	float decay;
	float rot_decay;
	float rot_vct;

	int move_type;

	FVector2 target_dist;
	float target_angle;
	float obj_angle;								/* ターゲットがこっちを向いていると判定する角度 */
	float run_angle;
	float run_dist;
	FVector2 formation;
	
	float distance;
	float dr;

	sOBJ *target;
	BOOL target_chase;								/* TRUE: ターゲット補足中 */
	
	int move_mode;
	BOOL first_proc;
	BOOL moveing;
	int interval;
	float target_dir;
	float obj_dir;
	float obj_dir_d;
	FVector2 shot_angle;
	FVector2 shot_time;
	int in_range;									/* TRUE:プレイヤーが視界内 */
	int in_shot;									/* TRUE:ショット中 */
	float forword;									/* 先読み時間 */
	
	float disp_x, disp_y;

	sLink *weapons;
	int weapon_max;
	int weapon_num;
	int shot_int;
	int shot_num;
	FVector2 shot_ofs[SHOT_NUM_MAX];
	BOOL shot;										/* SHOT中 */
	BOOL shot_active;

	int power;
	float radius;
	int dmg_nohit;
	int weapon_type;
	int weapon_level;
	int levelup_time;

	BOOL death;
	float red_eye;
	int dmg_col_timer;
	sRGBA col;

	int enemy_level;
} ObjVar;


/* ターゲットが視界に入っているか判別 */
static BOOL isTargetInView(ObjVar *var, sOBJ *target)
{
	BOOL res = FALSE;
	if(target)
	{
		FVector2 *pos = ObjGetPos(target);
		FVector2 *vct = ObjGetVct(target);
		FVector2 v;
		v.x = (pos->x + vct->x * var->forword) - var->pos.x;
		v.y = (pos->y + vct->y * var->forword) - var->pos.y;
		float distance = MathLength(&v);
		var->distance = distance;

		float r = MathVctAngleY(&v);
		float dr = DifAngle(r, var->dir);
		var->target_dir = r;

		res = (distance < var->target_dist.x) || ((distance < var->target_dist.y) && (fabsf(dr) < var->target_angle * 0.5f));
	}
	return res;
}

/* ターゲットがこっちを向いているか判別 */
static BOOL isTargetLookAt(ObjVar *var, sOBJ *target)
{
	BOOL res = FALSE;
	if(target)
	{
		FVector2 *pos = ObjGetPos(target);
		FVector2 *vct = ObjGetVct(target);
		FVector2 v;
		v.x = var->pos.x - (pos->x + vct->x * var->forword);
		v.y = var->pos.y - (pos->y + vct->y * var->forword);

		float r = MathVctAngleY(&v);
		float angle = ObjGetDir(target);
		float dr = DifAngle(r, angle);
		var->obj_dir_d = dr;
		var->obj_dir = ObjGetDir(target);

		res = fabsf(dr) < (var->obj_angle * 0.5f);
		if(var->run_dist > 0.0f)
		{
			if(!res) res = var->distance < var->run_dist;
		}
	}
	return res;
}

static float getRandomVctValue(char *id_str, sParam *param)
{
	FVector2 *vct = ParamGetFVec2(param, id_str);
	return vct->x + (vct->y - vct->x) * RndCH(RND_CH1);
}

static float randomDirection(ObjVar *var, sParam *param)
{
	return NormalAngle(var->dir + ANG2RAD(ParamGetReal(param, "move_angle") * RndmCH(RND_CH1)));
}

static float runawayDirection(ObjVar *var)
{
	float angle = var->run_angle;
	float dr = NormalAngle(var->obj_dir - var->dir);
	if(dr > 0.0f) angle = -angle;
	return NormalAngle(var->obj_dir + angle);
}

static void moveToTarget(ObjVar *var, float speed)
{
	float dr = DifAngle(var->dr, var->dir);
	if(fabsf(dr) > var->rot_speed)
	{
		dr = dr < 0.0f ? -var->rot_speed : var->rot_speed;
	}
	else
	{
		dr = 0.0f;
	}
	var->dir_vct = var->dir_vct * var->rot_decay + dr;
	dr = NormalAngle(var->dir + var->dir_vct);

	FVector2 vct;
	MathCalcVector(&vct, dr, speed);
	var->vct.x = var->vct.x * var->decay + vct.x;
	var->vct.y = var->vct.y * var->decay + vct.y;
	var->pos.x += var->vct.x;
	var->pos.y += var->vct.y;
	var->dir = dr;
}

//
// ランダム移動とターゲット移動の繰り返し
//
static void enemyMoveRandom(ObjVar *var, sParam *param, BOOL lockon, BOOL runaway)
{
	if(var->first_proc)
	{
		var->first_proc = FALSE;
		var->target_chase = FALSE;
		var->interval = 0;
	}
	
	if(var->target_chase)
	{
		if(runaway)
		{
			var->dr = runawayDirection(var);
		}
		else
		{
			var->dr = var->target_dir;
		}
	}

	var->interval -= 1;
	if(var->interval <= 0)
	{
		var->target_chase = lockon && !var->target_chase;
		if(!var->target_chase)
		{
			var->dr = randomDirection(var, param);
			var->interval = getRandomVctValue("move_time", param);
		}
		else
		{
			var->interval = getRandomVctValue("target_time", param);
		}
	}
	moveToTarget(var, runaway ? var->speed->y : var->speed->x);
}

//
// 移動とその場回転の繰り返し
//
static BOOL enemyMoveRotate(ObjVar *var, sParam *param, BOOL lockon, BOOL runaway)
{
	BOOL shot;
	if(var->first_proc)
	{
		var->first_proc = FALSE;
		var->moveing = FALSE;
		var->interval = 0;
	}

	if(var->moveing)
	{
		if(runaway)
		{
			runawayDirection(var);
		}
		else
		if(lockon)
		{
			var->dr = var->target_dir;
		}
	}
	float speed = var->moveing ? (runaway ? var->speed->y : var->speed->x) : 0.0f;
	shot = !var->moveing;							/* 停止中に攻撃 */
	
	var->interval -= 1;
	if(var->interval <= 0)
	{
		var->interval = getRandomVctValue("move_time", param);
		var->moveing = !var->moveing;
		if(!var->moveing) var->dr = randomDirection(var, param);
	}
	moveToTarget(var, speed);
	return shot;
}

//
// 編隊飛行
//
static void enemyMoveFormation(ObjVar *var, sParam *param)
{
	sOBJ *target = var->target;

	float dir = ObjGetDir(target);
	FVector2 *pos = ObjGetPos(target);
	FVector2 vct = var->formation;
	MathRotateXY(&vct, dir);
	var->vct = *ObjGetVct(target);

	if(var->first_proc)
	{
		var->pos.x = pos->x + vct.x;
		var->pos.y = pos->y + vct.y;
		var->dir = dir;
		var->first_proc = FALSE;
	}
	else
	{
		var->pos.x += (pos->x + vct.x - var->pos.x) * 0.07f;
		var->pos.y += (pos->y + vct.y - var->pos.y) * 0.07f;
		var->dir = NormalAngle(var->dir + DifAngle(dir, var->dir) * 0.05f);
	}
}

//
// やられ移動
//
static void enemyMoveDead(sOBJ *obj, ObjVar *var, sParam *param)
{

	var->dir_vct = var->dir_vct * var->rot_decay;
	var->dir = NormalAngle(var->dir + var->dir_vct);
//	MathRotateXY(&var->vct, var->dir_vct);

	var->vct.x = var->vct.x * var->decay;
	var->vct.y = var->vct.y * var->decay;
	var->pos.x += var->vct.x;
	var->pos.y += var->vct.y;

	var->interval -= 1;
	if(var->interval <= 0)
	{
		EffectCreate("explo", &var->pos, &FVec2Zero, var->dir, &FVec2One);
		EffectCreate("ene_ex", &var->pos, &FVec2Zero, var->dir, &FVec2One);
		SndPlay("enemy_dead", 1.0f);
		ObjKillReq(obj);
	}
}


static BOOL isShotTarget(sOBJ *obj, ObjVar *var, sParam *param)
{
	BOOL res = FALSE;

	sOBJ *target = var->target;
	if(target)
	{
		FVector2 *pos = ObjGetPos(target);
		FVector2 *vct = ObjGetVct(target);

		FVector2 *shot_range = ParamGetFVec2(param, "shot_range");
		
		FVector2 d;
		d.x = pos->x + vct->x * var->forword - var->pos.x;
		d.y = pos->y + vct->y * var->forword - var->pos.y;
		float dist = MathLength(&d);
		float dr = fabsf(DifAngleDeg(MathVctAngleY(&d), var->dir));

		res = (dist < shot_range->x) && (dr < (var->shot_angle.x / 2.0f));
		if(!res) res = (dist < shot_range->y) && (dr < (var->shot_angle.y / 2.0f));
		if(res)
		{
			FVector2 *abort = ParamGetFVec2(param, "shot_abort");
			res = (dist > abort->x) || (fabsf(var->obj_dir_d) > ANG2RAD(abort->y * 0.5f));
		}
#if 0
		if(dist < ParamGetReal(param, "shot_range"))
		{
			if(dr < ANG2RAD(ParamGetReal(param, "shot_angle") * 0.5f))
			{
				FVector2 *abort = ParamGetFVec2(param, "shot_abort");
				res = ((dist > abort->x) || (fabsf(var->obj_dir_d) > ANG2RAD(abort->y * 0.5f)));
			}
		}
#endif
	}
	
	return res;
}

static void createShot(sOBJ *obj, ObjVar *var, sParam *param)
{
	int type = var->weapon_type;
	FVector2 pos = *ParamGetFVec2(param, "shot_ofs");
	MathRotateXY(&pos, var->dir);
	pos.x += var->pos.x;
	pos.y += var->pos.y;

	char *type_name[] = {
		"bullet_e",
		"bullet_e",
		"bomb_e",
		"lazer_e",
		"thunder_e",
		"extra_p",
	};
	float powofs = 1.0f + (float)var->enemy_level * ParamGetReal(var->param, "enemy_shpow_lv");

	var->shot_int = CreateShot(obj, type, var->weapon_level, powofs, type_name[type + 1], OBJ_PLAYER, &pos, &var->vct, var->dir, var->param);
}


static int objProc(sOBJ *obj, sParam *param, int msg, int lParam, int rParam)
{
	int res = 0;

	ObjVar *var = (ObjVar *)ObjGetVar(obj, sizeof(ObjVar));
	switch(msg)
	{
	case MSG_CREATE:
		{
			var->shot_active = lParam;
			var->enemy_level = GameMainGetEnemyLevel();
			var->param = (sParam *)StkRefFrameP(0);
			var->move_type = ParamGetReal(param, "move_type");
			var->radius = ParamGetReal(param, "radius");
			var->power = PowerConvertInterValue(ParamGetReal(param, "power"));

			var->weapons = ObjLinkCreate(sizeof(WeaponLink), 4, MEM_APP, TRUE);
			var->weapon_max = ParamGetReal(param, "weapon_max");

			var->speed = ParamGetFVec2(param, "speed");
			var->decay = ParamGetReal(param, "decay");
			var->rot_speed = ANG2RAD(ParamGetReal(param, "rot_speed"));
			var->rot_decay = ParamGetReal(param, "rot_decay");

			var->target_dist = *ParamGetFVec2(param, "target_dist");
			var->target_angle = ANG2RAD(ParamGetReal(param, "target_angle"));
			var->obj_angle = ANG2RAD(ParamGetReal(param, "obj_angle"));
			var->run_angle = ANG2RAD(ParamGetReal(param, "run_angle"));
			var->run_dist = ParamGetReal(param, "run_dist");
			var->first_proc = TRUE;
			var->weapon_type = WEAPON_KIND_NONE;
			var->forword = ParamGetReal(param, "shot_forword") * FRAME_RATE;

			FVector2 *shot_angle = ParamGetFVec2(param, "shot_angle");
			float dr = ParamGetReal(var->param, "enemy_shang_lv");
			var->shot_angle.x = ANG2RAD(shot_angle->x + (float)var->enemy_level * dr);
			var->shot_angle.y = ANG2RAD(shot_angle->y + (float)var->enemy_level * dr);

			FVector4 *col_vct = ParamGetFVec4(param, "col");
			SetRGBA(&var->col, col_vct->x, col_vct->y, col_vct->z, col_vct->w);

			var->shot_time = *ParamGetFVec2(param, "shot_time");
			char id_str[ID_MAXLEN];
			int i;
			for(i = 0; i < SHOT_NUM_MAX; i += 1)
			{
				sprintf(id_str, "shot_ofs%d", i + 1);
				if(!ParamIsExists(param, id_str)) break;
				var->shot_ofs[i] = *ParamGetFVec2(param, id_str);
			}
			var->shot_num = i;

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
			BOOL wrap = FALSE;
			if(var->move_type != ENEMY_MOVE_FORMATION)
			{
				wrap = GameFieldRange(&var->pos);
			}
			var->pos_wrap = wrap;

			BOOL lockon = FALSE;
			BOOL runaway = FALSE;
			if(!var->death && (var->move_type != ENEMY_MOVE_FORMATION))
			{
				lockon = isTargetInView(var, var->target);
				runaway = isTargetLookAt(var, var->target);
			}

			BOOL shot = FALSE;
			switch(var->move_type)
			{
			case ENEMY_MOVE_RANDOM:
				{
					enemyMoveRandom(var, param, lockon, runaway);
				}
				break;

			case ENEMY_MOVE_ROTATE_MOVE:
				{
					shot = enemyMoveRotate(var, param, lockon, runaway);
				}
				break;

			case ENEMY_MOVE_FORMATION:
				{
					enemyMoveFormation(var, param);
				}
				break;

			case ENEMY_DEATH:
				{
					enemyMoveDead(obj, var, param);
				}
				break;
			}

			ObjSetPos(obj, var->pos.x, var->pos.y);
			ObjSetDir(obj, var->dir);
			ObjSetVct(obj, var->vct.x, var->vct.y);

			WeaponUptate(obj, var->weapons, FALSE, OBJ_ENEMY);

			if(var->move_type == ENEMY_MOVE_FORMATION)
			{
				shot = ObjPostMsg(var->target, MSG_GAME_ENEMY_IS_SHOT, 0, 0);
			}
			else
			{
				shot = shot || isShotTarget(obj, var, param);
			}
			shot = shot && var->shot_active;
			var->shot = shot;
			if(shot)
			{
				if(var->in_range > 0)
				{
					var->in_range -= 1;
				}
				else
				{
					if((var->in_shot > 0) && (var->shot_int == 0))
					{
						createShot(obj, var, param);
					}
					var->in_shot -= 1;
					if(var->in_shot == 0)
					{
						var->in_range = var->shot_time.x;
						var->in_shot = var->shot_time.y;
					}
				}
			}
			else
			{
				var->in_range = var->shot_time.x;
				var->in_shot = var->shot_time.y;
			}

			if(var->shot_int > 0) var->shot_int -= 1;
			if(var->dmg_nohit > 0) var->dmg_nohit -= 1;
			if(var->dmg_col_timer > 0) var->dmg_col_timer -= 1;
		}
		break;

	case MSG_UPDATE:
		{
			if(var->move_type == ENEMY_MOVE_FORMATION)
			{
				var->first_proc = ObjPostMsg(var->target, MSG_GAME_ENEMY_IS_WRAP, 0, 0);
			}
		}
		break;

	case MSG_DRAW:
		{
			FVector2 *size = ParamGetFVec2(param, "size");
			FVector2 *center = ParamGetFVec2(param, "center");

			sGRPOBJ *grp = GRPOBJ_QUAD(PRIO_ENEMY);
			GrpSetPos(grp, var->disp_x, var->disp_y);
			GrpSetRot(grp, var->dir);
			GrpSetSize(grp, size->x, size->y);
			GrpSetCenter(grp, center->x, center->y);
			GrpSetUV(grp, 0.0f, 0.0f);
			GrpSetTexture(grp, ParamGetTex(param, "texture"));
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

			grp = GRPOBJ_QUAD(PRIO_ENEMY);
			GrpSetPos(grp, var->disp_x, var->disp_y);
			GrpSetRot(grp, var->dir);
			GrpSetSize(grp, size->x, size->y);
			GrpSetCenter(grp, center->x, center->y);
			GrpSetUV(grp, size->x, 0.0f);
			GrpSetTexture(grp, ParamGetTex(param, "texture"));
			GrpSetFilter(grp, TRUE);
			if(var->death)
			{
				var->red_eye *= 0.8;
			}
			else
			{
				var->red_eye = 0.5f + fabsf(sinf((float)g.time / 30.0f)) * 0.5f;
			}
			GrpSetRGBA(grp, var->red_eye, 0.0f, 0.0f, 1.0f);
			
#ifdef DEBUG
			if(isDispDebugInfo())
			{
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

				FVector2 *shot_range = ParamGetFVec2(param, "shot_range");

				grp = GRPOBJ_FAN(PRIO_DEBUG_PRINT);
				GrpSetPos(grp, var->disp_x, var->disp_y);
				GrpSetSize(grp, shot_range->x, shot_range->x);
				GrpSetLineNum(grp, 12);
				GrpSetFanAngle(grp, var->shot_angle.x);
				GrpSetRGBA(grp, 1.0f, 1.0f, 0.0f, 1.0f);
				GrpSetDrawSize(grp, 1.0f);
				GrpSetRot(grp, NormalAngle(var->dir - (var->shot_angle.x / 2.0f)));

				grp = GRPOBJ_FAN(PRIO_DEBUG_PRINT);
				GrpSetPos(grp, var->disp_x, var->disp_y);
				GrpSetSize(grp, shot_range->y, shot_range->y);
				GrpSetLineNum(grp, 12);
				GrpSetFanAngle(grp, var->shot_angle.y);
				GrpSetRGBA(grp, 1.0f, 1.0f, 0.0f, 1.0f);
				GrpSetDrawSize(grp, 1.0f);
				GrpSetRot(grp, NormalAngle(var->dir - (var->shot_angle.y / 2.0f)));
			}
#endif
		}
		break;

	case MSG_GAME_ENEMY_SETUP:
		{
			var->pos = *ObjGetPos(obj);
			var->vct = *ObjGetVct(obj); 
			var->dir = ObjGetDir(obj);

			WeaponUptate(obj, var->weapons, FALSE, OBJ_ENEMY);
		}
		break;

	case MSG_GAME_ENEMY_SET_TARGET:
		{
			var->target = (sOBJ *)StkRefFrameP(0);
		}
		break;

	case MSG_GAME_ENEMY_SET_FORMATION:
		{
			var->target = (sOBJ *)StkRefFrameP(0);
			var->formation = *(FVector2 *)StkRefFrameP(1);
			var->move_type = ENEMY_MOVE_FORMATION;
		}
		break;

	case MSG_GAME_ENEMY_IS_SHOT:
		{
			res = var->shot;
		}
		break;

	case MSG_GAME_ENEMY_IS_WRAP:
		{
			res = var->pos_wrap;
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

	case MSG_GAME_WEAPON_FIXED:
		{
			int type = lParam;
			int level = rParam;
			if(var->weapon_type == WEAPON_KIND_NONE)
			{
				var->weapon_type = type;
				var->weapon_level = level - 1;
			}
			else
			if(var->weapon_type == type)
			{
				var->weapon_level += level;
			}
			var->weapon_num += 1;
			
			sOBJ *child = (sOBJ *)StkRefFrameP(0);
			WeaponFixOne(obj, child, var->weapons, ParamGetReal(param, "weapon_ofs"));
		}
		break;


	case MSG_GAME_WEAPON_IS_FIX:
		{
			res = var->weapon_num < ENEMY_WEAPON_MAXNUM;
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
						var->interval = ParamGetReal(param, "death_time") * FRAME_RATE;
						var->move_type = ENEMY_DEATH;

						var->speed = &FVec2Zero;
						var->decay = 0.998f;
						var->rot_decay = 0.998f;
						var->dir_vct *= 0.8f;

						WeaponFreeAll(var->weapons, WEAPON_LOST_ENEMY);

						/* 他のタスクに通知 */
						StkMakeFrame();
						StkPushP(obj);					// 0
						TaskPostMsgAll(TASK_PRI_NONE, MSG_GAME_ENEMY_DESTROY, ParamGetReal(param, "score"), 0);
						StkDelFrame();
					}
					var->dmg_col_timer = ParamGetReal(param, var->death ? "death_col_t" : "dmg_col_timer") * FRAME_RATE;
				}
			}
		}
		break;

	case MSG_GAME_ENEMY_DESTROY:
		{
			if(var->target == (sOBJ *)StkRefFrameP(0))
			{
				var->target = PlayerGetObj();
				if(var->move_type == ENEMY_MOVE_FORMATION)
				{
					var->move_type = ParamGetReal(param, "move_type");
					var->first_proc = TRUE;
				}
			}
			var->shot_active = TRUE;
		}
		break;

	case MSG_GAME_PLAYER_DEAD:
		{
			if(var->target == (sOBJ *)StkRefFrameP(0))
			{
				var->target = 0;
			}
		}
		break;
	}

	return res;
}


sOBJ *EnemyCreate(char *id_str, FVector2 *pos, float dir, BOOL shot, sParam *param)
{
	StkMakeFrame();
	StkPushP(param);								// 0
	sOBJ *obj = ObjCreate(id_str, OBJ_ENEMY, objProc, shot, 0);
	StkDelFrame();
	
	ObjSetPos(obj, pos->x, pos->y);
	ObjSetDir(obj, dir);
	ObjSetVct(obj, 0.0f, 0.0f);
	ObjPostMsg(obj, MSG_GAME_ENEMY_SETUP, 0, 0);

	return obj;
}

void EnemySetTarget(sOBJ *obj, sOBJ *target)
{
	StkMakeFrame();
	StkPushP(target);								// 0
	ObjPostMsg(obj, MSG_GAME_ENEMY_SET_TARGET, 0, 0);
	StkDelFrame();
}

void EnemySetFormation(sOBJ *obj, sOBJ *target, FVector2 *ofs)
{
	StkMakeFrame();
	StkPushP(target);								// 0
	StkPushP(ofs);									// 1
	ObjPostMsg(obj, MSG_GAME_ENEMY_SET_FORMATION, 0, 0);
	StkDelFrame();
}
