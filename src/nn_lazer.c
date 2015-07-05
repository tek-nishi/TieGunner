//
// ホーミングレーザー
//

#include "nn_lazer.h"
#include "co_debug.h"
#include "co_graph.h"
#include "co_obj.h"
#include "co_misc.h"
#include "co_stack.h"
#include "nn_main.h"
#include "nn_player.h"
#include "nn_gamemisc.h"

#define LAZER_TAIL_MAXNUM  32


typedef struct {
	sParam *param;

	FVector2 pos;
	FVector2 vct;
	float dir;

	sRGBA col;
	float size;
	BOOL disp_top;
	float top_circle;

	int level;
	u_int target_flag;
	float disp_x, disp_y;
	float radius;
	float speed;
	int power;

	int time;

	int delay;
	float decay;
	float rot_speed;
	float target_angle;
	sOBJ *target;

	int tail_num;
	int tail_maxnum;
	FVector2 tail_pos[LAZER_TAIL_MAXNUM];
	int lazer_fin;
} ObjVar;


static void searchTarget(ObjVar *var)
{
	sOBJ *target = ObjGetNext(0, var->target_flag, 0);
	sOBJ *near_obj = 0;
	float near_dist = 400.0f;				/* 適当に大きな値 */
	float near_dr = 0.0f;
	while(target)
	{
		if(!ObjIsDead(target))
		{
			FVector2 *pos = ObjGetPos(target);
			float dist = MathDistance(&var->pos, pos);

			FVector2 dpos;
			dpos.x = pos->x - var->pos.x;
			dpos.y = pos->y - var->pos.y;
			float dr = DifAngle(MathVctAngleY(&dpos), var->dir);
			if(dist < near_dist && fabsf(dr) < var->target_angle)
			{
				near_dist = dist;
				near_obj = target;
				near_dr = dr;
			}
		}
		target = ObjGetNext(target, var->target_flag, 0);
	}
	var->target = near_obj;
}

static void moveToTarget(ObjVar *var)
{
	sOBJ *target = var->target;
	float dir = var->dir;
	if(target && !ObjIsDead(target))
	{
		FVector2 *pos = ObjGetPos(target);
		FVector2 dpos;
		dpos.x = pos->x - var->pos.x;
		dpos.y = pos->y - var->pos.y;
		float dr = DifAngle(MathVctAngleY(&dpos), var->dir);
		if(fabsf(dr) > var->rot_speed)
		{
			dr = dr < 0.0f ? -var->rot_speed : var->rot_speed;
		}
		dir = NormalAngle(var->dir + dr);
	}

	FVector2 vct;
	MathCalcVector(&vct, dir, var->speed);
	var->vct.x = var->vct.x * var->decay + vct.x;
	var->vct.y = var->vct.y * var->decay + vct.y;
	var->pos.x += var->vct.x;
	var->pos.y += var->vct.y;
	var->dir = dir;
}

static BOOL sendDamage(sOBJ *obj, u_int type, FVector2 *pos, FVector2 *vct, float radius, int power)
{
	if(radius == 0.0f) return FALSE;				/* 半径ゼロは判定しない(着弾まで判定しない武器向け) */
	
	StkMakeFrame();

	StkPushP(obj);								// 0
	StkPushP(pos);								// 1
	StkPushP(vct);								// 2
	StkPushF(radius);							// 3
	StkPushI(power);							// 4
	StkPushI(0);

	BOOL abort = ObjPostMsgAll(type, MSG_GAME_DAMAGE, FALSE, 0, 0);
	
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
			var->speed = ParamGetReal(param, "speed");
			var->power = PowerConvertInterValue(ParamGetReal(param, "power") * powofs);
			var->decay = ParamGetReal(param, "decay");
			var->delay = ParamGetReal(param, "delay") * FRAME_RATE;

			sprintf(id_str, "%d.rot_speed", var->level);
			var->rot_speed = ANG2RAD(ParamGetReal(param, id_str));
			sprintf(id_str, "%d.target_angle", var->level);
			var->target_angle = ANG2RAD(ParamGetReal(param, id_str)) * 0.5f;
			sprintf(id_str, "%d.time", var->level);
			if(ParamIsExists(param, id_str))
			{
				var->time = ParamGetReal(param, id_str) * FRAME_RATE;
			}

			FVector4 *col = ParamGetFVec4(param, "col");
			SetRGBA(&var->col, col->x, col->y, col->z, col->w);
			var->size = ParamGetReal(param, "size");
			if(ParamIsExists(param, "top_circle"))
			{
				var->top_circle = ParamGetReal(param, "top_circle");
				var->disp_top = TRUE;
			}
			

			var->tail_pos[0] = var->pos;
			var->tail_num = 1;
			var->tail_maxnum = ParamIsExists(param, "tail_max") ? (ParamGetReal(param, "tail_max") - 1) : (LAZER_TAIL_MAXNUM - 1);

#if 0
			FVector2 vct; 
			MathCalcVector(&vct, var->dir, var->speed);
			var->vct.x += vct.x;
			var->vct.y += vct.y;
#endif

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

			if(var->delay > 0) var->delay -= 1;
			if(!var->target && !var->delay)
			{
				searchTarget(var);
			}
			moveToTarget(var);

			int i;
			for(i = var->tail_num; i > 0; i -= 1)
			{
				var->tail_pos[i] = var->tail_pos[i - 1];
			}
			var->tail_pos[i] = var->pos;
			if(var->tail_num < var->tail_maxnum) var->tail_num += 1;

			ObjSetPos(obj, var->pos.x, var->pos.y);
			ObjSetDir(obj, var->dir);
			ObjSetVct(obj, var->vct.x, var->vct.y);

			if(var->lazer_fin == 0)
			{
				BOOL abort = ((var->time > 0) && (!--var->time)) || FieldClip(&var->pos, 20.0f);
				if(abort)
				{
					var->lazer_fin = 0.5f * FRAME_RATE;
					var->speed = 0.0f;
				}
				
				/* TODO:非常に重い処理なので解決策を考える */
				BOOL kill = sendDamage(obj, var->target_flag, &var->pos, &var->vct, var->radius, var->power);
				if(kill)
				{
					ObjKillReq(obj);
				}
			}
			else
			{
				var->col.alpha *= 0.8f;
				if(!(var->lazer_fin -= 1))
				{
					ObjKillReq(obj);
				}
			}
		}
		break;

	case MSG_DRAW:
		{
			FVector2 tail_pos[LAZER_TAIL_MAXNUM];
			for(int i = 0; i < var->tail_num; i++)
			{
				tail_pos[i].x = var->tail_pos[i].x + var->disp_x;
				tail_pos[i].y = var->tail_pos[i].y + var->disp_y;
			}

			sGRPOBJ *grp;
			float alpha = var->col.alpha;
			for(int i = 0; i < (var->tail_num - 1); i++)
			{
				grp = GRPOBJ_LINE(PRIO_SHOT);
				GrpSetRGBA(grp, var->col.red, var->col.green, var->col.blue, alpha);
				GrpSetDrawSize(grp, var->size);
				GrpSetVtx(grp, &tail_pos[i], &tail_pos[i + 1], 0, 0);
				GrpSetSmooth(grp, TRUE);
				alpha *= 0.8f;
			}
			if(var->disp_top)
			{
				grp = GRPOBJ_FILLCIRCLE(PRIO_SHOT);
				GrpSetPos(grp, tail_pos[0].x, tail_pos[0].y);
				GrpSetRGBA(grp, var->col.red, var->col.green, var->col.blue, var->col.alpha);
				GrpSetSize(grp, var->top_circle, var->top_circle);
				GrpSetLineNum(grp, 8);
				GrpSetSmooth(grp, TRUE);
			}
			
#ifdef DEBUG
			if(isDispDebugInfo())
			{
				sGRPOBJ *grp = GRPOBJ_CIRCLE(PRIO_DEBUG_PRINT);
				GrpSetPos(grp, tail_pos[0].x, tail_pos[0].y);
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
			var->disp_x = -pos->x;
			var->disp_y = -pos->y;
		}
		break;


	case MSG_GAME_ENEMY_DESTROY:
	case MSG_GAME_PLAYER_DEAD:
	case MSG_GAME_BOSS_WEAPON_DEAD:
		{
			sOBJ *target = (sOBJ *)StkRefFrameP(0);
			if(var->target == target)
			{
				var->target = 0;
			}
		}
		break;
	}

	return res;
}

int LazerCreate(char *id_str, FVector2 *pos, FVector2 *vct, float r, u_int target, sParam *param, int level, float powofs)
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
		AddV2d(&v, &v, pos);

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
