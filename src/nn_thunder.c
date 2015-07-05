//
// 電撃弾
//

#include "nn_thunder.h"
#include "co_debug.h"
#include "co_graph.h"
#include "co_obj.h"
#include "co_misc.h"
#include "co_stack.h"
#include "co_random.h"
#include "nn_main.h"
#include "nn_player.h"
#include "nn_gamemisc.h"


typedef struct {
	sParam *param;

	FVector2 pos;
	FVector2 vct;
	float dir;

	sRGBA col;
	float radius;
	int l_size;

	FVector2 ofs;

	int level;
	u_int target_flag;
	FVector2 disp_p1, disp_p2;
	int power;
	int lost_num;									/* ダメージ時に落とす武器の数 */

	int time;										/* 攻撃時間 */
	FVector2 *target_dist;							/* 敵を見つける範囲 */
	float target_angle;								/* 距離 */
	float lock_angle;

	FVector2 start_pos;								/* 初期オフセット */
	float start_dir;
	FVector2 parent_pos;

	sOBJ *parent;
	sOBJ *target;
} ObjVar;


static BOOL isInTarget(ObjVar *var)
{
	BOOL res = FALSE;
	sOBJ *target = var->target;

	if(target && !ObjIsDead(target))
	{
		sOBJ *parent = var->parent;
		FVector2 *p_pos = ObjGetPos(parent);
		float dir = ObjGetDir(parent);

		FVector2 *pos = ObjGetPos(target);
		float dist = MathDistance(p_pos, pos);
		
		FVector2 dpos;
		SubV2d(&dpos, pos, p_pos);
		float dr = DifAngle(MathVctAngleY(&dpos), dir);
		res = ((dist < var->target_dist->x) || ((dist < var->target_dist->y) && (fabsf(dr) < var->lock_angle * 0.5f)));
	}

	
	return res;
}

static BOOL searchTarget(ObjVar *var)
{
	BOOL res = FALSE;
	sOBJ *parent = var->parent;
	FVector2 *p_pos = ObjGetPos(parent);
	float dir = ObjGetDir(parent);

	sOBJ *target = ObjGetNext(0, var->target_flag, 0);
	float near_dist = var->target_dist->y;
	float near_dist_min = var->target_dist->x;
	sOBJ *target_ptr[8];
	int target_num = 0;
	while(target && (target_num < 8))
	{
		if(!ObjIsDead(target))
		{
			FVector2 *pos = ObjGetPos(target);
			float dist = MathDistance(p_pos, pos);

			FVector2 dpos;
			SubV2d(&dpos, pos, p_pos);
			float dr = DifAngle(MathVctAngleY(&dpos), dir);
			if((dist < near_dist_min) || ((dist < near_dist) && (fabsf(dr) < var->target_angle * 0.5f)))
			{
				target_ptr[target_num] = target;
				target_num += 1;
				res = TRUE;
			}
		}
		target = ObjGetNext(target, var->target_flag, 0);
	}

	var->target = (target_num > 0) ? target_ptr[RndICH(RND_CH1, target_num)] : 0;
	return res;
}


static void sendDamage(sOBJ *obj, sOBJ *target, FVector2 *pos, int power, int lost_num)
{
	StkMakeFrame();

	StkPushP(obj);								// 0
	StkPushP(pos);								// 1
	StkPushP((void *)&FVec2Zero);				// 2
	StkPushF(0.0f);								// 3
	StkPushI(power);							// 4
	StkPushI(lost_num);

	ObjPostMsg(target, MSG_GAME_DAMAGE, TRUE, 0);
	
	StkDelFrame();
}


static int objProc(sOBJ *obj, sParam *param, int msg, int lParam, int rParam)
{
	int res = 0;

	ObjVar *var = (ObjVar *)ObjGetVar(obj, sizeof(ObjVar));
	switch(msg)
	{
	case MSG_CREATE:
		{
			var->level = lParam;
			var->target_flag = (u_int)rParam;
			var->param = (sParam *)StkRefFrameP(0);
			var->start_pos = *(FVector2 *)StkRefFrameP(1);
			var->parent = (sOBJ *)StkRefFrameP(2);
			var->start_dir = StkRefFrameF(3);
			float powofs = StkRefFrameF(4);
			var->lost_num = ceilf(powofs) - 1;

			FVector4 *col = ParamGetFVec4(param, "col");
			SetRGBA(&var->col, col->x, col->y, col->z, col->w);
			var->radius = ParamGetReal(param, "radius");
			var->l_size = ParamGetReal(param, "l_size");

/* 			FVector2 vct;  */
/* 			MathCalcVector(&vct, var->dir, var->speed); */
/* 			var->vct.x += vct.x; */
/* 			var->vct.y += vct.y; */

			char id_str[ID_MAXLEN];
			sprintf(id_str, "%d.time", var->level);
			var->time = ParamGetReal(param, id_str);
			sprintf(id_str, "%d.range", var->level);
			var->target_dist = ParamGetFVec2(param, id_str);
			sprintf(id_str, "%d.angle", var->level);
			var->target_angle = ANG2RAD(ParamGetReal(param, id_str));
			sprintf(id_str, "%d.lock_angle", var->level);
			var->lock_angle = ANG2RAD(ParamGetReal(param, id_str));
			sprintf(id_str, "%d.power", var->level);
			var->power = PowerConvertInterValue(ParamGetReal(param, id_str));

#if 0
			FVector2 vct;
			SetV2d(&vct, 0.0f, var->dist_range);
			MathRotateXY(&vct, var->start_dir);
			
			ObjSetPos(obj, var->pos.x, var->pos.y);
			ObjSetDir(obj, var->dir);
			ObjSetVct(obj, var->vct.x, var->vct.y);
#endif
		}
		break;

	case MSG_KILL:
		{
		}
		break;

	case MSG_STEP:
		{
			if(!isInTarget(var))
			{
				searchTarget(var);
			}

			FVector2 *pos = ObjGetPos(var->parent);
			float dir = ObjGetDir(var->parent);
			var->dir = NormalAngle(dir + var->start_dir);
			if(var->target)
			{
				var->pos = *ObjGetPos(var->target);
			}
			else
			{
				FVector2 vct;
				SetV2d(&vct, 0.0f, -var->target_dist->y);
				AddV2d(&vct, &vct, &var->ofs);
				MathRotateXY(&vct, var->dir);
				AddV2d(&var->pos, &vct, pos);
				if(!(var->time % 10))
				{
					var->ofs.x = Rndm() * 5.0f;
					var->ofs.y = Rndm() * 5.0f;
				}
			}

			{
				FVector2 vct = var->start_pos;
				MathRotateXY(&vct, var->dir);
				AddV2d(&var->parent_pos, &vct, pos);
			}

			if(var->target)
			{
				sendDamage(obj, var->target, &var->pos, var->power, var->lost_num);
			}

			ObjSetPos(obj, var->pos.x, var->pos.y);
			ObjSetDir(obj, var->dir);
//			ObjSetVct(obj, var->vct.x, var->vct.y);

			res = (var->time > 0) && (!--var->time);
			if(res)
			{
				ObjKillReq(obj);
			}
		}
		break;

	case MSG_DRAW:
		{
			int div = ParamGetReal(param, "div");
			float r_val = ParamGetReal(param, "r_val");
			FVector2 st = var->disp_p2;
			for(int i = 0; i < div; i += 1)
			{
				FVector2 vct;
				SubV2d(&vct, &var->disp_p1, &st);
				vct.x = vct.x / (float)(div - i);
				vct.y = vct.y / (float)(div - i);
				
				FVector2 ed;
				ed.x = st.x + vct.x + vct.y * r_val * Rndm();
				ed.y = st.y + vct.y + vct.x * r_val * Rndm();

				sGRPOBJ *grp = GRPOBJ_LINE(PRIO_SHOT);
				GrpSetVtx(grp, &st, &ed, 0, 0);
				GrpSetRGBA(grp, var->col.red, var->col.green, var->col.blue, var->col.alpha);
				GrpSetDrawSize(grp, var->l_size);
				st = ed;
				if(i == (div - 2)) r_val = 0.0f;
			}
			sGRPOBJ *grp = GRPOBJ_FILLCIRCLE(PRIO_SHOT);
			GrpSetPos(grp, st.x, st.y);
			GrpSetRGBA(grp, var->col.red, var->col.green, var->col.blue, var->col.alpha);
			GrpSetSize(grp, var->radius, var->radius);
			GrpSetLineNum(grp, 8);
			
#ifdef DEBUG
			if(isDispDebugInfo())
			{
				sGRPOBJ *grp = GRPOBJ_POINT(PRIO_DEBUG_PRINT);
				GrpSetPos(grp, var->disp_p1.x, var->disp_p1.y);
				GrpSetDrawSize(grp, 2.0f);
				GrpSetRGBA(grp, 1.0f, 1.0f, 0.0f, 1.0f);
				GrpSetDrawSize(grp, 1.0f);
			}
#endif
		}
		break;

	case MSG_GAME_CAMERA:
		{
			FVector2 *pos = (FVector2 *)StkRefFrameP(0);
			SubV2d(&var->disp_p1, &var->pos, pos);
			SubV2d(&var->disp_p2, &var->parent_pos, pos);
		}
		break;

		
	case MSG_GAME_ENEMY_DESTROY:
	case MSG_GAME_PLAYER_DEAD:
	case MSG_GAME_BOSS_WEAPON_DEAD:
		{
			sOBJ *target = (sOBJ *)StkRefFrameP(0);
			if(var->parent == target)
			{
				ObjKillReq(obj);
			}
			else
			if(var->target == target)
			{
				var->target = 0;
			}
		}
		break;
	}

	return res;
}

int ThunderCreate(char *id_str, sOBJ *parent, u_int target, sParam *param, int level, float powofs)
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

		StkMakeFrame();
		StkPushP(param);							// 0
		StkPushP(&v);								// 1
		StkPushP(parent);							// 2
		StkPushF(ANG2RAD(p->z));					// 3
		StkPushF(powofs);							// 4
		ObjCreate(id_str, OBJ_SHOT, objProc, level, target);
		StkDelFrame();
	}

	sprintf(id_param, "%d.interval", level);
	return ParamGetReal(objParam, id_param);
}

