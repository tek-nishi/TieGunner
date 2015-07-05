//
// エクストラ(画面内の全キャラにダメージ)
//

#include "nn_extra.h"
#include "co_debug.h"
#include "co_graph.h"
#include "co_obj.h"
#include "co_misc.h"
#include "co_stack.h"
#include "co_sound.h"
#include "nn_main.h"
#include "nn_gamemisc.h"
#include "nn_effect.h"


#define RAD_TBL_NUM  6


typedef struct {
	sParam *param;
	int level;
	u_int target_flag;

	int time;
	int time_max;
	FVector2 *rad_param;
	int fade_out;

	int power;

	FVector2 pos;
	FVector2 vct;
	float radius;
	float rad_tbl[RAD_TBL_NUM];
	float disp_x, disp_y;

	sRGBA col[3];
	float alpha[3];
} ObjVar;


static void sendDamage(sOBJ *obj, FVector2 *pos, u_int type, float r_in, float r_out, int power)
{
	sOBJ *target = 0;
	while(target = ObjGetNext(target, type, 0))
	{
		FVector2 *tpos = ObjGetPos(target);
		float radius = ObjGetRadius(target);
		float dist = MathDistance(pos, tpos);
		if(((dist + radius) > r_in) && ((dist - radius) < r_out))
		{
			StkMakeFrame();

			StkPushP(obj);								// 0
			StkPushP(tpos);								// 1
			StkPushP((void *)&FVec2Zero);				// 2
			StkPushF(0.0f);								// 3
			StkPushI(power);							// 4
			StkPushI(0);

			ObjPostMsg(target, MSG_GAME_DAMAGE, TRUE, 0);
	
			StkDelFrame();
		}
	}
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
			float powofs = StkRefFrameF(3);

			char id_str[ID_MAXLEN];
			sprintf(id_str, "%d.power", var->level);
			var->power = PowerConvertInterValue(ParamGetReal(param, id_str) * powofs);
			
			var->time_max = ParamGetReal(param, "time") * FRAME_RATE;
			var->rad_param = ParamGetFVec2(param, "radius");

			float radius = var->rad_param->x;
			var->radius = radius;
			for(int i = 0; i < RAD_TBL_NUM; i += 1)
			{
				var->rad_tbl[i] = radius;
			}

			for(int i = 0; i < 3; i += 1)
			{
				char id_str[ID_MAXLEN];
				FVector4 *col;

				sprintf(id_str, "col%d", i + 1);
				col = ParamGetFVec4(param, id_str);
				SetRGBA(&var->col[i], col->x, col->y, col->z, col->w);
				var->alpha[i] = var->col[i].alpha;
			}

			var->fade_out = ParamGetReal(param, "fade_out") * FRAME_RATE;

			ObjSetPos(obj, var->pos.x, var->pos.y);
			ObjSetVct(obj, var->vct.x, var->vct.y);
		}
		break;

	case MSG_KILL:
		{
		}
		break;

	case MSG_STEP:
		{
			AddV2d(&var->pos, &var->pos, &var->vct);
			if(var->time < var->time_max)
			{
				var->time += 1;
				float d = (float)var->time / (float)var->time_max;
				d = 0.5f + sinf(PI * cosf(PI * d) * -0.5f) * 0.5f;
				FVector2 *vct = var->rad_param;
				float r_in = var->radius;
				float r_out = vct->x + (vct->y - vct->x) * d;
				var->radius = r_out;

				int i;
				for(i = 0; i < (RAD_TBL_NUM - 1); i += 1)
				{
					var->rad_tbl[i] = var->rad_tbl[i + 1];
				}
				var->rad_tbl[i] = r_in;

				int time = var->time_max - var->time;
				if(time < var->fade_out)
				{
					for(int i = 0; i < 3; i += 1)
					{
						var->col[i].alpha = var->alpha[i] * (float)time / (float)var->fade_out;
					}
				}
				else
				{
					sendDamage(obj, &var->pos, var->target_flag, r_in, r_out, var->power);
				}

				if(var->time == var->time_max)
				{
					ObjKillReq(obj);
				}
			}
			ObjSetPos(obj, var->pos.x, var->pos.y);
			ObjSetVct(obj, var->vct.x, var->vct.y);
		}
		break;
		
	case MSG_DRAW:
		{
			sGRPOBJ *grp;

			grp = GRPOBJ_FILLDONUT(PRIO_SHOT);
			GrpSetPos(grp, var->disp_x, var->disp_y);
			float rad_out = var->radius;
			float rad_in = var->rad_tbl[0];
			GrpSetDonutSize(grp, rad_out, rad_out, rad_in, rad_in);
			GrpSetLineNum(grp, 20);
			GrpSetRGBA4(grp, &var->col[0], &var->col[1], 0, 0);

			grp = GRPOBJ_CIRCLE(PRIO_SHOT);
			GrpSetPos(grp, var->disp_x, var->disp_y);
			GrpSetSize(grp, rad_out, rad_out);
			GrpSetLineNum(grp, 20);
			GrpSetRGBA(grp, var->col[2].red, var->col[2].green, var->col[2].blue, var->col[2].alpha);
			GrpSetDrawSize(grp, 1.0f);
			GrpSetSmooth(grp, TRUE);
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
	}
	
	return res;
}

int ExtraCreate(char *id_str, FVector2 *pos, FVector2 *vct, float r, u_int target, sParam *param, int level, float powofs)
{
	char id_param[ID_MAXLEN];

	sParam *objParam = ObjGetSetupParam(id_str);
	ASSERT(objParam);

	int maxlevel = ParamGetReal(objParam, "maxlevel");
	if(level > maxlevel) level = maxlevel;
	FVector2 ofs = *ParamGetFVec2(objParam, "ofs");
	MathRotateXY(&ofs, r);
	AddV2d(&ofs, &ofs, pos);

	StkMakeFrame();
	StkPushP(param);							// 0
	StkPushP(&ofs);								// 1
	StkPushP(vct);								// 2
	StkPushF(powofs);							// 3
	ObjCreate(id_str, OBJ_SHOT, objProc, level, target);
	StkDelFrame();
	
	sprintf(id_param, "%d.interval", level);
	return ParamGetReal(objParam, id_param);
}
