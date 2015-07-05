//
// ’Êí’e
//

#include "nn_bullet.h"
#include "co_debug.h"
#include "co_graph.h"
#include "co_obj.h"
#include "co_misc.h"
#include "co_stack.h"
#include "nn_main.h"
#include "nn_player.h"
#include "nn_gamemisc.h"


typedef struct {
	sParam *param;

	FVector2 pos;
	FVector2 vct;
	float dir;

	sRGBA col1;
	sRGBA col2;
	FVector2 *disp_size;

	int level;
	u_int target_flag;
	float disp_x, disp_y;
	float radius;
	float speed;
	int power;
} ObjVar;


static BOOL sendDamage(sOBJ *obj, u_int type, FVector2 *pos, FVector2 *vct, float radius, int power)
{
	if(radius == 0.0f) return FALSE;				/* ”¼Œaƒ[ƒ‚Í”»’è‚µ‚È‚¢(’…’e‚Ü‚Å”»’è‚µ‚È‚¢•ŠíŒü‚¯) */
	
	StkMakeFrame();

	StkPushP(obj);								// 0
	StkPushP(pos);								// 1
	StkPushP(vct);								// 2
	StkPushF(radius);							// 3
	StkPushI(power);							// 4
	StkPushI(0);

	BOOL abort = ObjPostMsgAll(type, MSG_GAME_DAMAGE, TRUE, 0, 0);
	
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
			FVector4 *col;
			col = ParamGetFVec4(param, "col1");
			SetRGBA(&var->col1, col->x, col->y, col->z, col->w);
			col = ParamGetFVec4(param, "col2");
			SetRGBA(&var->col2, col->x, col->y, col->z, col->w);
			var->disp_size = ParamGetFVec2(param, "disp_size");
			

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
				/* TODO:”ñí‚Éd‚¢ˆ—‚È‚Ì‚Å‰ðŒˆô‚ðl‚¦‚é */
				BOOL kill = sendDamage(obj, var->target_flag, &var->pos, &var->vct, var->radius, var->power);
				if(kill)
				{
					ObjKillReq(obj);
				}
			}
		}
		break;

	case MSG_DRAW:
		{
			sGRPOBJ *grp;
			grp = GRPOBJ_FILLCIRCLE(PRIO_SHOT);
			GrpSetPos(grp, var->disp_x, var->disp_y);
			GrpSetRot(grp, var->dir);
			GrpSetRGBA(grp, var->col1.red, var->col1.green, var->col1.blue, var->col1.alpha);
			GrpSetSize(grp, var->disp_size->x, var->disp_size->x);
			GrpSetLineNum(grp, 8);
			GrpSetSmooth(grp, TRUE);

			grp = GRPOBJ_FILLCIRCLE(PRIO_SHOT);
			GrpSetPos(grp, var->disp_x, var->disp_y);
			GrpSetRot(grp, var->dir);
			GrpSetRGBA(grp, var->col2.red, var->col2.green, var->col2.blue, var->col2.alpha);
			GrpSetSize(grp, var->disp_size->y, var->disp_size->y);
			GrpSetLineNum(grp, 6);
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

int BulletCreate(char *id_str, FVector2 *pos, FVector2 *vct, float r, u_int target, sParam *param, int level, float powofs)
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
