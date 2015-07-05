//
// スプレッドボム
//

#include "nn_bomb.h"
#include "co_debug.h"
#include "co_graph.h"
#include "co_obj.h"
#include "co_misc.h"
#include "co_stack.h"
#include "co_sound.h"
#include "nn_main.h"
#include "nn_player.h"
#include "nn_gamemisc.h"
#include "nn_effect.h"


typedef struct {
	sParam *param;

	FVector2 pos;
	FVector2 vct;
	float dir;

	sRGBA col;
	sRGBA bomb_col[4];

	int level;
	u_int target_flag;
	float disp_x, disp_y;
	float radius;
	float speed;
	int power;
	BOOL thrust;
	int time;
	int ex_time;

	float bomb_radius;
	int bomb_time;
} ObjVar;


static BOOL isEnemyInRange(ObjVar *var)
{
	float dist_near = var->bomb_radius;
	sOBJ *target = ObjGetNext(0, var->target_flag, 0);
	while(target)
	{
		if(!ObjIsDead(target))
		{
			FVector2 *pos = ObjGetPos(target);
			float dist = MathDistance(&var->pos, pos);
			if(dist < dist_near)
			{
				break;
			}
		}
		target = ObjGetNext(target, var->target_flag, 0);
	}

	return target != 0;
}

static BOOL sendDamage(sOBJ *obj, u_int type, FVector2 *pos, FVector2 *vct, float radius, int power, BOOL thrust)
{
	if(radius == 0.0f) return FALSE;				/* 半径ゼロは判定しない(着弾まで判定しない武器向け) */
	
	StkMakeFrame();

	StkPushP(obj);								// 0
	StkPushP(pos);								// 1
	StkPushP(vct);								// 2
	StkPushF(radius);							// 3
	StkPushI(power);							// 4
	StkPushI(0);


	BOOL abort = ObjPostMsgAll(type, MSG_GAME_DAMAGE, !thrust, 0, 0);
	
	StkDelFrame();

	return abort;
}


static int objProc(sOBJ *obj, sParam *param, int msg, int lParam, int rParam)
{
	int res = 0;

	ObjVar *var = (ObjVar *)ObjGetVar(obj, sizeof(ObjVar));
	switch(msg)
	{
	case MSG_CREATE:
		{
			char id_str[ID_MAXLEN];
			
			var->level = lParam;
			var->target_flag = (u_int)rParam;
			var->param = (sParam *)StkRefFrameP(0);
			var->pos = *(FVector2 *)StkRefFrameP(1);
			var->vct = *(FVector2 *)StkRefFrameP(2);
			var->dir = StkRefFrameF(3);
			float powofs = StkRefFrameF(4);

			var->radius = ParamGetReal(param, "radius");
			var->ex_time = ParamGetReal(param, "ex_time") * FRAME_RATE;
			sprintf(id_str, "%d.speed", var->level);
			var->speed = ParamGetReal(param, id_str);
			sprintf(id_str, "%d.power", var->level);
			var->power = PowerConvertInterValue(ParamGetReal(param, id_str) * powofs);
			FVector4 *col = ParamGetFVec4(param, "col");
			SetRGBA(&var->col, col->x, col->y, col->z, col->w);
			for(int i = 0; i < 4; i += 1)
			{
				sprintf(id_str, "bomb_col%d", i);
				FVector4 *col = ParamGetFVec4(param, id_str);
				SetRGBA(&var->bomb_col[i], col->x, col->y, col->z, col->w);
			}

			sprintf(id_str, "%d.radius", var->level);
			var->bomb_radius = ParamGetReal(param, id_str);
			sprintf(id_str, "%d.time", var->level);
			var->bomb_time = ParamGetReal(param, id_str);

			FVector2 vct; 
			MathCalcVector(&vct, var->dir, var->speed);
			var->vct.x += vct.x;
			var->vct.y += vct.y;

			ObjSetPos(obj, var->pos.x, var->pos.y);
			ObjSetDir(obj, var->dir);
			ObjSetVct(obj, var->vct.x, var->vct.y);
		}
		break;

	case MSG_KILL:
		{
		}
		break;

	case MSG_STEP:
		{
#if 0
			GameFieldRange(&var->pos);
#endif

			var->pos.x += var->vct.x;
			var->pos.y += var->vct.y;
			
			ObjSetPos(obj, var->pos.x, var->pos.y);
			ObjSetDir(obj, var->dir);
			ObjSetVct(obj, var->vct.x, var->vct.y);

			res = FieldClip(&var->pos, 20.0f);
			if(res)
			{
				ObjKillReq(obj);
			}
			else
			{
				/* TODO:非常に重い処理なので解決策を考える */
				BOOL kill = FALSE;
				if(var->time <= 0) kill = isEnemyInRange(var);
				if((var->ex_time > 0) && !(--var->ex_time > 0)) kill = TRUE;
				kill = kill || sendDamage(obj, var->target_flag, &var->pos, &var->vct, var->radius, var->power, var->thrust);
				if(kill)
				{
					if(!var->thrust)
					{
						var->thrust = TRUE;
						var->vct = FVec2Zero;
						var->time = var->bomb_time;
						var->radius = var->bomb_radius;
						var->ex_time = 0;

						FVector2 scale;
						SetV2d(&scale, var->radius, var->radius);
						EffectCreate("bomb", &var->pos, &FVec2Zero, 0.0f, &scale);
						SndPlay("bomb_ex", 0.95f);
					}
				}
				
				if((var->time > 0) && !(--var->time))
				{
					ObjKillReq(obj);
				}
			}
		}
		break;

	case MSG_DRAW:
		{
			sGRPOBJ *grp = GRPOBJ_FILLCIRCLE(PRIO_SHOT);
			GrpSetPos(grp, var->disp_x, var->disp_y);
//			GrpSetRot(grp, var->dir);
			sRGBA *col;
			if(var->thrust)
			{
				int ofs = g.time & (4 -1);
				col = &var->bomb_col[ofs];
				GrpSetBlendMode(grp, GRP_BLEND_REV);
			}
			else
			{
				col = &var->col;
			}
			GrpSetRGBA(grp, col->red, col->green, col->blue, col->alpha);
			GrpSetSize(grp, var->radius, var->radius);
			GrpSetLineNum(grp, var->thrust ? 20 : 8);
			GrpSetSmooth(grp, TRUE);
			
#ifdef DEBUG
			if(isDispDebugInfo())
			{
				sGRPOBJ *grp = GRPOBJ_CIRCLE(PRIO_DEBUG_PRINT);
				GrpSetPos(grp, var->disp_x, var->disp_y);
				GrpSetSize(grp, var->radius, var->radius);
				GrpSetLineNum(grp, 12);
				GrpSetRGBA(grp, 1.0f, 1.0f, 0.0f, 1.0f);
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
	}

	return res;
}

int BombCreate(char *id_str, FVector2 *pos, FVector2 *vct, float r, u_int target, sParam *param, int level, float powofs)
{
	char id_param[ID_MAXLEN];

	sParam *objParam = ObjGetSetupParam(id_str);
	ASSERT(objParam);

	int maxlevel = ParamGetReal(objParam, "maxlevel");
	if(level > maxlevel) level = maxlevel;
	
	for(int i = 1; ; i += 1)
	{
		sprintf(id_param, "%d.shot_pos%d", level, i);
		if(!ParamIsExists(objParam, id_param)) break;
		
		FVector3 *p = ParamGetFVec3(objParam, id_param);
		FVector2 v;
		v.x = p->x;
		v.y = p->y;
		MathRotateXY(&v, r);
		v.x += pos->x;
		v.y += pos->y;

		StkMakeFrame();
		StkPushP(param);							// 0
		StkPushP(&v);								// 1
		StkPushP(vct);								// 2
		StkPushF(NormalAngle(r + ANG2RAD(p->z)));	// 3
		StkPushF(powofs);							// 4
		ObjCreate(id_str, OBJ_SHOT, objProc, level, target);
		StkDelFrame();
	}

	sprintf(id_param, "%d.interval", level);
	return ParamGetReal(objParam, id_param);
}
