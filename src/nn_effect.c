//
// エフェクト
//

#include "nn_effect.h"
#include "co_task.h"
#include "co_param.h"
#include "co_objlink.h"
#include "co_graph.h"
#include "co_memory.h"
#include "co_misc.h"
#include "co_stack.h"
#include "nn_main.h"


#define EFFECT_MAXNUM  256


typedef struct {
	BOOL steped;									/* TRUE: 初期化済み */
	int delay;
	int type;
	FVector2 pos;
	FVector2 vct;
	float dir;
	sRGBA col;
	sRGBA col_c;
	float alpha;
	float alpha_c;
	FVector2 size;
	FVector2 center;
	FVector2 uv;
	FVector2 scale;
	int blend;

	FVector2 uv_ofs;

	int pat_num;
	int pat_time;
	int pat_time_value;
	int pat_num_value;

	sTexture *texture;
	FVector2 tex_size;

	FVector2 rad_start;
	FVector2 rad_end;
	int vtx_num;
	int l_size;
	int count, frame;
	BOOL slow;
} EffectObj;


typedef struct {
	sParam *param;
	sLink *link;
	FVector2 disp_pos;
} EffectVar;


static sTaskBody *eftTask;


static int mainProc(sTaskBody *body, int msg, int lParam, int rParam)
{
	int res = 0;
	EffectVar *var = (EffectVar *)TaskGetVar(body, sizeof(EffectVar), MEM_APP);
	switch(msg)
	{
	case MSG_CREATE:
		{
			eftTask = body;
			var->param = ParamRead(PATH_DATA"/effect.param");
			var->link = ObjLinkCreate(sizeof(EffectObj), EFFECT_MAXNUM, MEM_APP, FALSE);
		}
		break;

	case MSG_KILL:
		{
			eftTask = 0;
			ObjLinkDelAll(var->link);
			ParamDestroy(var->param);
		}
		break;

	case MSG_STEP:
		{
			void *p;
			p = ObjLinkGetTop(var->link);
			while(p)
			{
				EffectObj *obj = (EffectObj *)p;

				obj->pos.x += obj->vct.x;
				obj->pos.y += obj->vct.y;

				void *next = ObjLinkGetNext(p);
				if(obj->delay > 0)
				{
					obj->delay -= 1;
				}
				else
				{
					switch(obj->type)
					{
					case 0:
						{
							if(!(obj->pat_time -= 1))
							{

								PatCalcUV(&obj->uv, obj->pat_num, obj->tex_size.x, obj->tex_size.y, obj->size.x, obj->size.y);
								obj->uv.x += obj->uv_ofs.x;
								obj->uv.y += obj->uv_ofs.y;
								obj->pat_time = obj->pat_time_value;

								obj->pat_num += 1;
								if(obj->pat_num > obj->pat_num_value)
								{
									ObjLinkDel(var->link, p);
								}
							}
						}
						break;

					case 1:
					case 2:
						{
							obj->pat_time += 1;
							float d = (float)obj->pat_time / obj->pat_time_value;
						
							obj->col.alpha = obj->alpha * (1.0f - d);
							obj->col_c.alpha = obj->alpha_c * (1.0f - d);

							if(obj->slow) d = d * d;
							else          d = (2.0f - d) * d;
							obj->size.x = obj->rad_start.x + (obj->rad_end.x - obj->rad_start.x) * d;
							obj->size.y = obj->rad_start.y + (obj->rad_end.y - obj->rad_start.y) * d;

							if(obj->pat_time == obj->pat_time_value)
							{
								ObjLinkDel(var->link, p);
							}
						}
						break;
					}
					obj->steped = TRUE;
				}
				p = next;
			}
		}
		break;

	case MSG_DRAW:
		{
			void *p;
			p = ObjLinkGetTop(var->link);
			while(p)
			{
				EffectObj *obj = (EffectObj *)p;
				if(obj->steped)
				{
					sGRPOBJ *grp;
					switch(obj->type)
					{
					case 0:
						{
							grp = GRPOBJ_QUAD(PRIO_EFFECT);
							GrpSetPos(grp, obj->pos.x - var->disp_pos.x, obj->pos.y - var->disp_pos.y);
							GrpSetSize(grp, obj->size.x, obj->size.y);
							GrpSetUV(grp, obj->uv.x, obj->uv.y);
							GrpSetCenter(grp, obj->center.x, obj->center.y);
							GrpSetRot(grp, obj->dir);
							GrpSetRGBA(grp, obj->col.red, obj->col.green, obj->col.blue, obj->col.alpha);   
							GrpSetTexture(grp, obj->texture);
							GrpSetFilter(grp, TRUE);
						}
						break;

					case 1:
						{
							grp = GRPOBJ_CIRCLE(PRIO_EFFECT);
							GrpSetPos(grp, obj->pos.x - var->disp_pos.x, obj->pos.y - var->disp_pos.y);
							GrpSetScale(grp, obj->scale.x, obj->scale.y);
							GrpSetRGBA(grp, obj->col.red, obj->col.green, obj->col.blue, obj->col.alpha);
							GrpSetSize(grp, obj->size.x, obj->size.y);
							GrpSetLineNum(grp, obj->vtx_num);
							GrpSetDrawSize(grp, obj->l_size);
							GrpSetSmooth(grp, TRUE);
						}
						break;

					case 2:
						{
							grp = GRPOBJ_FILLCIRCLE(PRIO_EFFECT);
							GrpSetPos(grp, obj->pos.x - var->disp_pos.x, obj->pos.y - var->disp_pos.y);
							GrpSetRGBA4(grp, &obj->col_c, &obj->col, 0, 0);
							GrpSetSize(grp, obj->size.x, obj->size.y);
							GrpSetLineNum(grp, obj->vtx_num);
							GrpSetSmooth(grp, TRUE);
						}
						break;
					}
					GrpSetBlendMode(grp, obj->blend);
				}
				
				p = ObjLinkGetNext(p);
			}
		}
		break;

	case MSG_GAME_CAMERA:
		{
			FVector2 *pos = (FVector2 *)StkRefFrameP(0);
			var->disp_pos = *pos;
		}
		break;

	case MSG_GAME_EFFECT_CREATE:
		{
			EffectObj *obj = (EffectObj *)ObjLinkNew(var->link);
			ASSERT(obj);

			char *name = (char *)StkRefFrameP(0);
			obj->pos = *(FVector2 *)StkRefFrameP(1);
			obj->vct = *(FVector2 *)StkRefFrameP(2);
			obj->dir = StkRefFrameF(3);
			obj->scale = *(FVector2 *)StkRefFrameP(4);
			
			char id_str[ID_MAXLEN];
			sprintf(id_str, "%s.type", name);
			obj->type = ParamGetReal(var->param, id_str);
			switch(obj->type)
			{
			case 0:
				{
					sprintf(id_str, "%s.tex", name);
					char *tex_name = ParamGetStr(var->param, id_str);
					sTexture *texture = ParamGetTex(var->param, tex_name);
					obj->texture = texture;
					TexGetSize(&obj->tex_size, texture);

			
					sprintf(id_str, "%s.size", name);
					obj->size = *ParamGetFVec2(var->param, id_str);

					sprintf(id_str, "%s.center", name);
					obj->center = *ParamGetFVec2(var->param, id_str);

					sprintf(id_str, "%s.uv", name);
					obj->uv_ofs = *ParamGetFVec2(var->param, id_str);

					sprintf(id_str, "%s.pat", name);
					obj->pat_num_value = ParamGetReal(var->param, id_str);

					sprintf(id_str, "%s.time", name);
					obj->pat_time_value = ParamGetReal(var->param, id_str);
					obj->pat_time = 1;
				}
				break;

			case 1:
				{
					sprintf(id_str, "%s.rad_st", name);
					obj->rad_start = *ParamGetFVec2(var->param, id_str);
					sprintf(id_str, "%s.rad_ed", name);
					obj->rad_end = *ParamGetFVec2(var->param, id_str);
					sprintf(id_str, "%s.time", name);
					obj->pat_time_value = ParamGetReal(var->param, id_str) * FRAME_RATE;
					sprintf(id_str, "%s.slow", name);
					obj->slow = ParamGetReal(var->param, id_str);
					sprintf(id_str, "%s.vtx", name);
					obj->vtx_num = ParamGetReal(var->param, id_str);
					sprintf(id_str, "%s.lsz", name);
					obj->l_size = ParamGetReal(var->param, id_str);
				}
				break;

			case 2:
				{
					sprintf(id_str, "%s.rad_st", name);
					obj->rad_start = *ParamGetFVec2(var->param, id_str);
					sprintf(id_str, "%s.rad_ed", name);
					obj->rad_end = *ParamGetFVec2(var->param, id_str);
					sprintf(id_str, "%s.time", name);
					obj->pat_time_value = ParamGetReal(var->param, id_str) * FRAME_RATE;
					sprintf(id_str, "%s.slow", name);
					obj->slow = ParamGetReal(var->param, id_str);
					sprintf(id_str, "%s.vtx", name);
					obj->vtx_num = ParamGetReal(var->param, id_str);

					sprintf(id_str, "%s.col_c", name);
					FVector4 *col = ParamGetFVec4(var->param, id_str);
					SetRGBA(&obj->col_c, col->x, col->y, col->z, col->w);
					obj->alpha_c = col->w;
				}
				break;
			}

			sprintf(id_str, "%s.delay", name);
			obj->delay = ParamGetReal(var->param, id_str) * FRAME_RATE;
			sprintf(id_str, "%s.col", name);

			FVector4 *col = ParamGetFVec4(var->param, id_str);
			SetRGBA(&obj->col, col->x, col->y, col->z, col->w);
			obj->alpha = col->w;
			sprintf(id_str, "%s.bl", name);
			obj->blend = ParamGetReal(var->param, id_str);
		}
		break;
		
	case MSG_GAME_EFFECT_DELETE:
		{
			ObjLinkDelAll(var->link);
		}
		break;
	}
	return res;
}


void EffectStart(void)
{
	TaskCreate("Effect", TASK_PRI_SYS, mainProc, 0, 0);	
}

void EffectCreate(char *id_str, FVector2 *pos, const FVector2 *vct, float dir, const FVector2 *scale)
{
	if(eftTask)
	{
		StkMakeFrame();
		StkPushP(id_str);							// 0
		StkPushP(pos);								// 1
		StkPushP((void *)vct);						// 2
		StkPushF(dir);								// 3
		StkPushP((void *)scale);					// 4
		TaskPostMsg(eftTask, MSG_GAME_EFFECT_CREATE, 0, 0);
		StkDelFrame();
	}
}

void EffectDeleteAll(void)
{
	if(eftTask)
	{
		TaskPostMsg(eftTask, MSG_GAME_EFFECT_DELETE, 0, 0);
	}
}
