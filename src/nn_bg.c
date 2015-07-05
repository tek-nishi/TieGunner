//
// ”wŒiˆ—
//

#include "nn_bg.h"
#include "co_obj.h"
#include "co_random.h"
#include "co_graph.h"
#include "co_stack.h"
#include "co_memory.h"
#include "nn_main.h"


#define STAR_ALPHA_TBL 12


typedef struct {
	FVector2 pos;
	FVector2 disp_pos[2];
	sRGBA col;
	sRGBA col_tail;
	float alpha;
	float ofs;
	int size;
	int prio;
	int time;
	BOOL line;
	BOOL first;
	BOOL erase;
} StarInfo;

typedef struct {
	int num;
	StarInfo *star;
	float range;
	FVector2 disp_pos;
	float star_alpha_tbl[STAR_ALPHA_TBL];
	FVector2 *star_alpha;
	FVector2 *star_time;
	float tail_dist;
	sRGBA *col_tbl;
	int col_tbl_num;
} ObjVar;


static sOBJ *bgObj = 0;
static ObjVar *bgVar = 0;


static void readColTbl(ObjVar *var, sParam *param)
{
	char id_str[ID_MAXLEN];
	int i;

	for(i = 1; ; i += 1)
	{
		sprintf(id_str, "%d.col", i);
		if(!ParamIsExists(param, id_str)) break;
	}
	var->col_tbl_num = i - 1;

	sRGBA *col_tbl = (sRGBA *)appMalloc(sizeof(sRGBA) * i, "bg_col");
	ASSERT(col_tbl);
	var->col_tbl = col_tbl;
	for(i = 0; i < var->col_tbl_num; i += 1)
	{
		sprintf(id_str, "%d.col", i + 1);
		FVector4 *vct = ParamGetFVec4(param, id_str);
		col_tbl->red = vct->x;
		col_tbl->green = vct->y;
		col_tbl->blue = vct->z;
		col_tbl->alpha = vct->w;

		col_tbl += 1;
	}
}

static void createStar(StarInfo *star, ObjVar *var)
{
	star->pos.x = Rndm() * var->range;
	star->pos.y = Rndm() * var->range;
	star->col = *(var->col_tbl + RndI(var->col_tbl_num));
	star->col_tail = star->col;
	star->col_tail.alpha = 0.0f;
	star->alpha = var->star_alpha->x + Rnd() * var->star_alpha->y;
	star->time = var->star_time->x + RndI((int)var->star_time->y);
	star->first = TRUE;
	star->erase = FALSE;
}


static int objProc(sOBJ *obj, sParam *param, int msg, int lParam, int rParam)
{
	int res = 0;

	ObjVar *var = (ObjVar *)ObjGetVar(obj, sizeof(ObjVar));
	switch(msg)
	{
	case MSG_CREATE:
		{
			var->num = ParamGetReal(param, "num");
			var->star = (StarInfo *)appMalloc(sizeof(StarInfo) * var->num, "StarInfo");
			var->range = ParamGetReal(param, "range");
			FVector2 *speed = ParamGetFVec2(param, "speed");
			float line_speed = ParamGetReal(param, "line_speed");
			var->star_alpha = ParamGetFVec2(param, "star_alpha");
			var->star_time = ParamGetFVec2(param, "star_time");
			readColTbl(var, param);

			StarInfo *star = var->star;
			for(int i = 0; i < var->num; i += 1)
			{
				createStar(star, var);

				star->ofs = speed->x + Rnd() * speed->y;
				star->prio = (star->ofs < 1.0f) ? PRIO_STAR_BACK : PRIO_STAR_FRONT;

				float size_tbl[] = { 1.0f, 1.0f, 1.0f, 2.0f };
				star->size = size_tbl[RndI(elemsof(size_tbl))];

				star->line = (star->ofs > line_speed);
				
				star += 1;
			}
			
			FVector2 *alpha = ParamGetFVec2(param, "alpha");
			for(int i = 0; i < STAR_ALPHA_TBL; i += 1)
			{
				var->star_alpha_tbl[i] = alpha->x + alpha->y * (1.0f + sinf(PI * (float)i / (float)STAR_ALPHA_TBL)) / 2.0f;
			}
			var->tail_dist = ParamGetReal(param, "tail_dist");
			bgObj = obj;
			bgVar = var;
		}
		break;

	case MSG_KILL:
		{
			Free(var->star);
			Free(var->col_tbl);
			bgObj = 0;
			bgVar = 0;
		}
		break;

	case MSG_STEP:
		{
		}
		break;

	case MSG_DRAW:
		{
			float tail_dist = var->tail_dist;
			float range = var->range * 2.0f;
			StarInfo *star = var->star;
			for(int i = 0; i < var->num; i += 1)
			{
				float dx, dy;
				float ofs;

				ofs = star->ofs;
				dx = star->pos.x - var->disp_pos.x * ofs;
				dy = star->pos.y - var->disp_pos.y * ofs;
				SetV2d(&star->disp_pos[0], dx, dy);
				if(star->first)
				{
					star->first = FALSE;
					SetV2d(&star->disp_pos[1], dx, dy);
				}
				dx = fmodf(dx, range);
				dy = fmodf(dy, range);
				if(dx < 0.0f) dx += range;
				if(dy < 0.0f) dy += range;

				star->col.alpha += (star->alpha - star->col.alpha) * 0.1f;
				float alpha = var->star_alpha_tbl[star->time % STAR_ALPHA_TBL];
					
				sGRPOBJ *grp;
				if(star->line && !MainIsPause())
				{
					grp = GRPOBJ_LINE(star->prio);
					sRGBA col = star->col;
					col.alpha *= alpha;
					GrpSetRGBA4(grp, &col, &star->col_tail, 0, 0);
					GrpSetDrawSize(grp, star->size);
					FVector2 pos[2];
					SetV2d(&pos[0], dx, dy);
					SetV2d(&pos[1], dx + (star->disp_pos[1].x - star->disp_pos[0].x) * tail_dist, dy + (star->disp_pos[1].y - star->disp_pos[0].y) * tail_dist);
					GrpSetVtx(grp, &pos[0], &pos[1], 0, 0);
				}
				else
				{
					grp = GRPOBJ_POINT(star->prio);
					GrpSetRGBA(grp, star->col.red, star->col.green, star->col.blue, star->col.alpha * alpha);
					GrpSetDrawSize(grp, star->size);
					GrpSetPos(grp, dx, dy);
				}
				GrpSetBlendMode(grp, GRP_BLEND_ADD);

				star->disp_pos[1] = star->disp_pos[0];

				if((star->time > 0) && !--star->time)
				{
					if(!star->erase)
					{
						star->erase = TRUE;
						star->alpha = 0.0f;
						star->time = 10;
					}
					else
					{
						createStar(star, var);
					}
				}
				star += 1;
			}
		}
		break;

		
	case MSG_GAME_CAMERA:
		{
			FVector2 *pos = (FVector2 *)StkRefFrameP(0);
			var->disp_pos = *pos;
		}
		break;
	}

	return res;
}


void BgCreate(void)
{
	ObjCreate("bg", OBJ_BG, objProc, 0, 0);
}

void BgResetTail(void)
{
	/* ƒJƒƒ‰‚ÌÀ•W‚ªXV‚³‚ê‚½‚çŒÄ‚Ô */
	if(bgVar)
	{
		StarInfo *star = bgVar->star;
		for(int i = 0; i < bgVar->num; i += 1)
		{
			star->first = TRUE;
			star += 1;
		}
	}
}
