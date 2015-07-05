//
// プレイヤー処理
//

#include "nn_player.h"
#include "co_input.h"
#include "co_graph.h"
#include "co_obj.h"
#include "co_objlink.h"
#include "co_random.h"
#include "co_misc.h"
#include "co_memory.h"
#include "co_param.h"
#include "co_font.h"
#include "co_stack.h"
#include "co_task.h"
#include "co_sound.h"
#include "nn_main.h"
#include "nn_weapon.h"
#include "nn_camera.h"
#include "nn_effect.h"
#include "nn_sndeffect.h"
#include "nn_gamemain.h"
#include "nn_gamemisc.h"


#define PLAYER_MOVE_LENGTH	 3.0f					/* プレイヤーとマウスカーソルの最低距離 */
#define PLAYER_WEAPON_MAXNUM (99 - 1)


typedef struct {
	sParam *param;									/* ゲーム全体のパラメーター */
	
	FVector2 pos;
	FVector2 vct;
	float dir;

	float decay;
	
	float disp_x, disp_y;

	sLink *weapons;
	int weapon_max;
	int shot_int;
	float radius;

	int power;
	int dmg_nohit;
	int weapon_type;
	int weapon_level;
	int levelup_time;
	int weapon_num;

	BOOL death;
	int dead_count;
	float red_eye;
	Sprite thrust;									/* 推進力演出 */
	FVector2 *th_pos;
	int dmg_col_timer;
} ObjVar;


static sOBJ *getTailWeapon(ObjVar *var)
{
	sOBJ *obj = 0;
	void *p = ObjLinkGetLast(var->weapons);
	if(p)
	{
		WeaponLink *link = (WeaponLink *)p;
		obj = link->obj;
	}
	return obj;
}

static void analyzeWeaponType(ObjVar *var)
{
	sOBJ *child = getTailWeapon(var);
	if(child)
	{
		int type = ObjPostMsg(child, MSG_GAME_WEAPON_TYPE, 0, 0);
		if(type != WEAPON_KIND_EXTRA) var->weapon_type = type;
	}
}

static void createShot(sOBJ *obj, int type, ObjVar *var, sParam *param)
{
	FVector2 pos = *ParamGetFVec2(param, "shot_ofs");
	MathRotateXY(&pos, var->dir);
	pos.x += var->pos.x;
	pos.y += var->pos.y;

	char *type_name[] = {
		"bullet_p",
		"bullet_p",
		"bomb_p",
		"lazer_p",
		"thunder_p",
		"extra_p",
	};

	var->shot_int = CreateShot(obj, type, var->weapon_level, 1.0f, type_name[type + 1], OBJ_ENEMY | OBJ_BOSS | OBJ_BOSS_WEAPON, &pos, &var->vct, var->dir, var->param);

	char *se_name[] = {
		"shot_bullet",
		"shot_bullet",
		"shot_bomb",
		"shot_lazer",
		"shot_th",
		"shot_extra",
	};
	float se_vol[] = {
		1.0f,
		1.0f,
		1.0f,
		1.0f,
		1.0f,
		0.9f,
	};
	SndPlay(se_name[type + 1], se_vol[type + 1]);
}


static int objProc(sOBJ *obj, sParam *param, int msg, int lParam, int rParam)
{
	int res = 0;

	ObjVar *var = (ObjVar *)ObjGetVar(obj, sizeof(ObjVar));
	switch(msg)
	{
	case MSG_CREATE:
		{
			var->param = (sParam *)StkRefFrameP(0);
			var->pos = FVec2Zero;
			var->vct = FVec2Zero;
			var->weapons = ObjLinkCreate(sizeof(WeaponLink), 4, MEM_APP, TRUE);
			var->weapon_max = (int)ParamGetReal(param, "weapon_max");
			var->radius = ParamGetReal(param, "radius");
			var->power = PowerConvertInterValue(ParamGetReal(param, "power"));
			var->weapon_type = WEAPON_KIND_NONE;
			var->decay = ParamGetReal(param, "decay");
			var->red_eye = 1.0f;
			SpriteSetup(&var->thrust, "th", param);
			var->thrust.prio = PRIO_PLAYER + 1;
			var->thrust.blend = GRP_BLEND_ADD;
			var->th_pos = ParamGetFVec2(param, "th.pos");

			ObjSetPos(obj, var->pos.x, var->pos.y);
			ObjSetDir(obj, var->dir);
			ObjSetVct(obj, var->vct.x, var->vct.y);
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
			FVector2 acc;
			if(!var->death)
			{
				FVector2 *camera = CameraGetPos();
				if(InputIsAppExec(INP_CH0))
				{
					float x = (float)InputGetAppMouseX(INP_CH0) - (float)(WINDOW_WIDTH / 2) + camera->x;
					float y = (float)InputGetAppMouseY(INP_CH0) - (float)(WINDOW_HEIGHT / 2) + camera->y;

					/* 向きを求める */
					FVector2 vct;
					SetV2d(&vct, x - var->pos.x, y - var->pos.y);
					if(MathLength(&vct) > PLAYER_MOVE_LENGTH)
					{
						var->dir = MathVctAngleY(&vct);
					}

				}
				/* 移動ベクトルを求める */
				MathCalcVector(&acc, var->dir, ParamGetReal(param, "speed"));
			}
			else
			{
				acc = FVec2Zero;
			}

			var->vct.x = var->vct.x * var->decay + acc.x;
			var->vct.y = var->vct.y * var->decay + acc.y;
			var->pos.x += var->vct.x;
			var->pos.y += var->vct.y;

#if 0
			if(InputIsPlayBack())
			{
				PRINTF("P:%f %f(%f %f) %f %f\n", var->pos.x, var->pos.y, var->vct.x, var->vct.y, acc.x, acc.y);
			}
#endif
			
			ObjSetPos(obj, var->pos.x, var->pos.y);
			ObjSetDir(obj, var->dir);
			ObjSetVct(obj, var->vct.x, var->vct.y);

			if(InputGetAppBtnTD(INP_CH0, MOUSE_RIGHT) || InputGetAppKeyPush(INP_CH0, ' '))
			{
				int weapon_num = var->weapon_num;
				int power = var->power;
				/*  WeaponUseOne()を実行すると書き替えられるので、予め保持しておく */

				if(weapon_num > 0)
				{
					sOBJ *child = getTailWeapon(var);
					ASSERT(child);
					int type = ObjPostMsg(child, MSG_GAME_WEAPON_TYPE, 0, 0);
					int level = ObjPostMsg(child, MSG_GAME_WEAPON_LEVEL, 0, 0);

					WeaponUseOne(obj, var->weapons, PRIO_PLAYER + 1);
					var->weapon_num = weapon_num - 1;
					var->power = PowerAddValue(power, -1);
					if(type == WEAPON_KIND_EXTRA)
					{
						createShot(obj, type, var, param);
						if(!var->levelup_time) analyzeWeaponType(var);
						/* EXTRAはパワーアップしないでショット扱い */
					}
					else
					{
						int levelup_time = ParamGetReal(param, "levelup_time") * FRAME_RATE;
						levelup_time *= level;
						if(var->weapon_type == type)
						{
							var->weapon_level += level;
							if(var->weapon_level > 1) levelup_time /= 2;
							var->levelup_time += levelup_time;
						}
						else
						{
							var->weapon_level = level;
							var->weapon_type = type;
							var->levelup_time = levelup_time;
						}
/* 						PRINTF("weapon level:%d %d\n", var->weapon_level, var->levelup_time); */
						SndPlay("power_up", 1.0f);
						TaskPostMsgAll(TASK_PRI_NONE, MSG_GAME_PLAYER_LVUP, var->weapon_type, var->levelup_time);
					}
				}
			}
			WeaponUptate(obj, var->weapons, FALSE, OBJ_ENEMY | OBJ_BOSS);

			/* 弾発射 */
			BOOL shot = (var->shot_int == 0) && InputGetAppBtnP(INP_CH0, MOUSE_LEFT);
			if(shot)
			{
				createShot(obj, var->weapon_type, var, param);
			}

			if((var->levelup_time > 0) && !(var->levelup_time -= 1))
			{
				var->weapon_level = 0;
				analyzeWeaponType(var);
			}
			
			if(var->shot_int > 0) var->shot_int -= 1;
			if(var->dmg_nohit > 0) var->dmg_nohit -= 1;
			if(var->dmg_col_timer > 0) var->dmg_col_timer -= 1;

			if(var->death)
			{
				var->dead_count -= 1;
				if(var->dead_count == 0)
				{
					EffectCreate("explo", &var->pos, &FVec2Zero, var->dir, &FVec2One);
					EffectCreate("plr_ex", &var->pos, &FVec2Zero, var->dir, &FVec2One);
					SndPlay("player_dead", 1.0f);

					ObjKillReq(obj);
				}
			}
		}
		break;

	case MSG_DRAW:
		{
			FVector2 *size = ParamGetFVec2(param, "size");
			FVector2 *center = ParamGetFVec2(param, "center");
			sTexture *texture = ParamGetTex(param, "texture");

			sGRPOBJ *grp = GRPOBJ_QUAD(PRIO_PLAYER);
			GrpSetPos(grp, var->disp_x, var->disp_y);
			GrpSetRot(grp, var->dir);
			GrpSetSize(grp, size->x, size->y);
			GrpSetCenter(grp, center->x, center->y);
			GrpSetUV(grp, 0.0f, 0.0f);
			GrpSetTexture(grp, texture);
			GrpSetFilter(grp, TRUE);
			if(var->dmg_col_timer > 0)
			{
				sRGBA col;
				DmgHitColor(&col, var->dmg_col_timer, param);
				GrpSetRGBA(grp, col.red, col.green, col.blue, col.alpha);
			}

			grp = GRPOBJ_QUAD(PRIO_PLAYER);
			GrpSetPos(grp, var->disp_x, var->disp_y);
			GrpSetRot(grp, var->dir);
			GrpSetSize(grp, size->x, size->y);
			GrpSetCenter(grp, center->x, center->y);
			GrpSetUV(grp, size->x, 0.0f);
			GrpSetTexture(grp, texture);
			GrpSetFilter(grp, TRUE);
			if(var->death)
			{
				var->red_eye *= 0.9f;
			}
			float red = var->red_eye;
			GrpSetRGBA(grp, red, red, red * 0.25f, 1.0f);

			var->thrust.pos = *var->th_pos;
			MathRotateXY(&var->thrust.pos, var->dir);
			var->thrust.pos.x += var->disp_x;
			var->thrust.pos.y += var->disp_y;
			var->thrust.rot = var->dir;
			if(var->death)
			{
				var->thrust.col.alpha *= 0.9f;
			}
			else
			{
				var->thrust.col.alpha = 0.7f + 0.3f * Rnd();
			}
			SpriteDraw(&var->thrust, texture);
#ifdef DEBUG
			if(isDispDebugInfo())
			{
				grp = GRPOBJ_CIRCLE(PRIO_DEBUG_PRINT);
				GrpSetPos(grp, var->disp_x, var->disp_y);
				GrpSetSize(grp, var->radius, var->radius);
				GrpSetLineNum(grp, 12);
				GrpSetRGBA(grp, 1.0f, 0.0f, 0.0f, 1.0f);
				GrpSetDrawSize(grp, 1.0f);
			}
#endif
		}
		break;


	case MSG_GAME_CAMERA:
		{
			FVector2 *pos = (FVector2 *)StkRefFrameP(0);
			var->disp_x = var->pos.x - pos->x;
			var->disp_y = var->pos.y - pos->y;
		}
		break;

	case MSG_GAME_WEAPON_FIXED:
		{
			int type = lParam;
			int level = rParam;
			sOBJ *child = (sOBJ *)StkRefFrameP(0);

			var->weapon_num += 1;
			var->power = PowerAddValue(var->power, 1);
			WeaponFixOne(obj, child, var->weapons, ParamGetReal(param, "weapon_ofs"));
			if((var->weapon_num == 1) && (var->levelup_time == 0) && (type != WEAPON_KIND_EXTRA))
			{
				var->weapon_type = type;
/* 				var->weapon_type = ObjPostMsg(child, MSG_GAME_WEAPON_TYPE, 0, 0); */
			}
		}
		break;

	case MSG_GAME_WEAPON_IS_FIX:
		{
			res = var->weapon_num < PLAYER_WEAPON_MAXNUM;
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
#ifdef DEBUG
				if(isPlayerPowerInf()) power = 0;
#endif
				int damaged = PowerCalcDamage(&var->power, power);
				if(damaged > 0)
				{
					var->dmg_nohit = ParamGetReal(param, "dmg_nohit") * FRAME_RATE;
					int lost_num = ShotGetLostWeapon(); /* 雷撃だけは特殊 */
					if(lost_num > 0) PowerCalcDamage(&var->power, PowerConvertInterValue(lost_num));
					
					damaged += lost_num;
					int num = var->weapon_num;
					for(int i = 0; i < damaged; i += 1)
					{
						/* ダメージ量に応じて武器を落とす */
						int weapon_num = var->weapon_num;
						int power = var->power;
						/* ↑WeaponFreeOne()で書きかえられるための措置 */

						WeaponFreeOne(obj, var->weapons, PRIO_PLAYER + 1, WEAPON_LOST_PLAYER);
						if(!var->levelup_time) analyzeWeaponType(var);
						var->weapon_num = weapon_num - 1;
						var->power = power;
					}
					PRINTF("damage:%d (%d %d)\n", damaged, num, var->weapon_num);
					if(num > 0 && (var->weapon_num <= 0))
					{
						var->power = PowerConvertInterValue(1.0f);
						var->weapon_num = 0;
						// ラストチャンス
					}

					EffectCreate("hit", pos, &FVec2Zero, 0.0f, &FVec2One);
					SndPlay("shot_hit", 1.0f);

					if(var->power <= 0)
					{
						ObjSetDeath(obj, TRUE);
						var->death = TRUE;
						var->dead_count = ParamGetReal(param, "death_time") * FRAME_RATE;

						var->decay = 0.8f;

						WeaponFreeAll(var->weapons, WEAPON_LOST_PLAYER);
						InputSetAppExec(INP_CH0, FALSE);
						StkMakeFrame();
						StkPushP(obj);					// 0
						TaskPostMsgAll(TASK_PRI_NONE, MSG_GAME_PLAYER_DEAD, 0, 0);
						StkDelFrame();
					}
					var->dmg_col_timer = var->death ? 0 : ParamGetReal(param, "dmg_col_timer") * FRAME_RATE;
				}
			}
		}
		break;

	case MSG_GAME_START:
		{
			var->shot_int = 0;
		}
		break;
	}

	return res;
}


sOBJ *PlayerCreate(sParam *param)
{
	StkMakeFrame();
	StkPushP(param);								// 0
	sOBJ *obj = ObjCreate("player", OBJ_PLAYER, objProc, 0, 0);
	StkDelFrame();

	return obj;
}

sOBJ *PlayerGetObj(void)
{
	return ObjGetNext(0, OBJ_PLAYER, 0);
}

FVector2 *PlayerGetPos(void)
{
	sOBJ *obj = PlayerGetObj();
	return obj ? ObjGetPos(obj) : 0;
}

REAL PlayerGetDir(void)
{
	sOBJ *obj = PlayerGetObj();
	return obj ? ObjGetDir(obj) : 0.0f;
}

FVector2 *PlayerGetVct(void)
{
	sOBJ *obj = PlayerGetObj();
	return obj ? ObjGetVct(obj) : 0;
}

void PlayerSetPos(FVector2 *pos)
{
	sOBJ *obj = PlayerGetObj();
	if(obj)
	{
		ObjVar *var = (ObjVar *)ObjGetVar(obj, sizeof(ObjVar));
		var->pos = *pos;
		ObjSetPos(obj, pos->x, pos->y);
	}
}

void PlayerSetVct(FVector2 *vct)
{
	sOBJ *obj = PlayerGetObj();
	if(obj)
	{
		ObjVar *var = (ObjVar *)ObjGetVar(obj, sizeof(ObjVar));
		var->vct = *vct;
		ObjSetVct(obj, vct->x, vct->y);
	}
}

void PlayerSetDir(float dir)
{
	sOBJ *obj = PlayerGetObj();
	if(obj)
	{
		ObjVar *var = (ObjVar *)ObjGetVar(obj, sizeof(ObjVar));
		var->dir = dir;
		ObjSetDir(obj, dir);
	}
}

int PlayerGetWeaponNum(void)
{
	int num = 0;
	sOBJ *obj = PlayerGetObj();
	if(obj)
	{
		ObjVar *var = (ObjVar *)ObjGetVar(obj, sizeof(ObjVar));
		num = var->weapon_num;
	}
	return num;
}

int PlayerGetPower(void)
{
	int power = 0;
	sOBJ *obj = PlayerGetObj();
	if(obj)
	{
		ObjVar *var = (ObjVar *)ObjGetVar(obj, sizeof(ObjVar));
		power = PowerConvertOuterValue(var->power);
	}
	return power;
}
