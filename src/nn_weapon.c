//
// 武器処理
//

#include "nn_weapon.h"
#include "co_graph.h"
#include "co_obj.h"
#include "co_objlink.h"
#include "co_random.h"
#include "co_misc.h"
#include "co_memory.h"
#include "co_font.h"
#include "co_stack.h"
#include "nn_main.h"
#include "nn_player.h"
#include "nn_effect.h"
#include "nn_gamemain.h"
#include "nn_gamemisc.h"


#define WEAPON_FREE_MAXVCT 5.0						/* 解放時の最大移動ベクトル */

#define EXTRA_BASE_ALPHA  0.4f						/* BASE + ADD = 1.0f */
#define EXTRA_ADD_ALPHA	  0.6f
#define EXTRA_CYCLE_ALPHA 0.2f


enum {
	MODE_FREE,
	MODE_FIXED,
};


typedef struct {
	sParam *param;

	int type;										/* enumWEAPON_KIND */
	int level;
	sRGBA col;
	float radius;
	int fix_prio;

	FVector2 pos;
	FVector2 vct;
	float vr;
	float dir;

	int mode;
	float clip;
	
	float disp_x, disp_y;

	sLink *weapons;
	int weapon_max;
	
	int free_time;
	int lost_time;
} ObjVar;


static int weaponLostTime(int type, sParam *param)
{
	char *lost_id[] = {
		// enumWEAPON_LOST
		"lost_none",
		"lost_player",
		"lost_enemy",
		"lost_boss",
	};
	return ParamGetReal(param, lost_id[type]) * FRAME_RATE;
}

static int objProc(sOBJ *obj, sParam *param, int msg, int lParam, int rParam)
{
	int res = 0;

	ObjVar *var = (ObjVar *)ObjGetVar(obj, sizeof(ObjVar));
	switch(msg)
	{
	case MSG_CREATE:
		{
			int type = lParam;
			var->param = (sParam *)StkRefFrameP(0);
			var->type = (type < 0) ? RndICH(RND_CH1, WEAPON_KIND_MAX) : type;
			var->level = ParamGetReal(param, "level");
			var->weapons = ObjLinkCreate(sizeof(WeaponLink), 4, MEM_APP, TRUE);

			char id_str[ID_MAXLEN];
			sprintf(id_str, "%d.col", var->type);
			FVector4 *col = ParamGetFVec4(param, id_str);
			SetRGBA(&var->col, col->x, col->y, col->z, col->w);
			var->radius = ParamGetReal(param, "radius");
			var->weapon_max = 1;
			var->clip = ParamGetReal(param, "clip");

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
			switch(var->mode)
			{
			case MODE_FREE:
				{
#if 0
					GameFieldRange(&var->pos);
#endif
					AddV2d(&var->pos, &var->pos, &var->vct);
					var->dir = NormalAngle(var->dir + var->vr);
					if(var->free_time > 0) var->free_time -= 1;

#if 0
					if(FieldClip(&var->pos, var->clip))
					{
						ObjKillReq(obj);
					}
					else
#endif
					if(var->free_time == 0)
					{
						u_int obj_type = OBJ_PLAYER | OBJ_ENEMY;
						sOBJ *parent = ObjGetNext(0, obj_type, 0);
						while(parent)
						{
							FVector2 *parent_pos = ObjGetPos(parent);
							float length = MathDistance(parent_pos, &var->pos);
							if((length < var->radius * 2.0f) && !ObjIsDead(parent))
							{
								if(ObjPostMsg(parent, MSG_GAME_WEAPON_IS_FIX, 0, 0))
								{
									var->mode = MODE_FIXED;
									u_int parent_type = ObjGetType(parent);
									var->fix_prio = (parent_type & OBJ_PLAYER) ? PRIO_PLAYER + 1 : PRIO_ENEMY + 1;
									StkMakeFrame();
									StkPushP(obj);
									ObjPostMsg(parent, MSG_GAME_WEAPON_FIXED, var->type, var->level);
									StkDelFrame();
									if(ObjGetType(parent) & OBJ_PLAYER)
									{
										SndPlay("weapon_get", 1.0f);
										GameMainPlayerGetWeapon();
									}
									break;
								}
							}
							parent = ObjGetNext(parent, obj_type, 0);
						}
					}
					if((var->mode == MODE_FREE) && (var->lost_time > 0) && (!--var->lost_time))
					{
						EffectCreate("w_exp", &var->pos, &FVec2Zero, 0.0f, &FVec2One);
						EffectCreate("w_lost", &var->pos, &FVec2Zero, 0.0f, &FVec2One);
						SndPlay("w_lost", 0.5f);
						ObjKillReq(obj);
					}
				}
				break;

			case MODE_FIXED:
				{
				}
				break;
			}

			ObjSetPos(obj, var->pos.x, var->pos.y);
			ObjSetDir(obj, var->dir);
			ObjSetVct(obj, var->vct.x, var->vct.y);
		}
		break;

	case MSG_DRAW:
		{
			int prio = (var->mode == MODE_FREE) ? PRIO_WEAPON : var->fix_prio;
			FVector2 *size = ParamGetFVec2(param, "size");
			FVector2 *center = ParamGetFVec2(param, "center");
			FVector2 uv = *ParamGetFVec2(param, "uv");
			uv.x += size->x * var->type;
			sTexture *tex = ParamGetTex(param, "texture");

			if(var->type == WEAPON_KIND_EXTRA)
			{
				var->col.alpha = EXTRA_BASE_ALPHA + fabs(sinf((float)g.time * EXTRA_CYCLE_ALPHA)) * EXTRA_ADD_ALPHA;
			}

			sGRPOBJ *grp = GRPOBJ_QUAD(prio);
			GrpSetPos(grp, var->disp_x, var->disp_y);
			GrpSetRot(grp, var->dir);
			GrpSetSize(grp, size->x, size->y);
			GrpSetCenter(grp, center->x, center->y);
			GrpSetUV(grp, uv.x, uv.y);
			GrpSetTexture(grp, tex);
			GrpSetFilter(grp, TRUE);
			GrpSetRGBA(grp, var->col.red, var->col.green, var->col.blue, var->col.alpha);
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
			float x = var->pos.x - pos->x;
			float y = var->pos.y - pos->y;
			var->disp_x = x;
			var->disp_y = y;
		}
		break;

	case MSG_GAME_WEAPON_FIXED:
		{
			sOBJ *child = (sOBJ *)StkRefFrameP(0);
			WeaponFixOne(obj, child, var->weapons, var->radius);
		}
		break;

	case MSG_GAME_WEAPON_FREE:
		{
			if(ObjLinkGetNum(var->weapons) == 0)
			{
				var->mode = MODE_FREE;
				FVector3 *vct = ParamGetFVec3(param, "free_vct");
				var->vct.x *= vct->x;
				var->vct.y *= vct->y;
				var->vr *= vct->z;
				if(MathLength(&var->vct) > WEAPON_FREE_MAXVCT)
				{
					MathNormalize(&var->vct, &var->vct);
					MathScalar(&var->vct, WEAPON_FREE_MAXVCT);
				}
				var->free_time = ParamGetReal(param, "free_time") * FRAME_RATE;
				var->lost_time = weaponLostTime(lParam, param);
				res = 1;
			}
			else
			{
				void *p = ObjLinkGetLast(var->weapons);
				while(p)
				{
					WeaponLink *link = (WeaponLink *)p;
					int result = ObjPostMsg(link->obj, MSG_GAME_WEAPON_FREE, lParam, 0);
					void *next = ObjLinkGetPrev(p);
					if(result)
					{
						ObjLinkDel(var->weapons, p);
					}
					p = next;
				}
			}
		}
		break;

	case MSG_GAME_WEAPON_FREE_ALL:
		{
			void *p = ObjLinkGetLast(var->weapons);
			while(p)
			{
				WeaponLink *link = (WeaponLink *)p;
				ObjPostMsg(link->obj, MSG_GAME_WEAPON_FREE_ALL, lParam, 0);
				void *next = ObjLinkGetPrev(p);
				ObjLinkDel(var->weapons, p);
				p = next;
			}
			var->mode = MODE_FREE;
			FVector3 *vct = ParamGetFVec3(param, "free_vct");
			var->vct.x *= vct->x;
			var->vct.y *= vct->y;
			if(MathLength(&var->vct) > WEAPON_FREE_MAXVCT)
			{
				MathNormalize(&var->vct, &var->vct);
				MathScalar(&var->vct, WEAPON_FREE_MAXVCT);
			}
			var->vr *= vct->z;
			var->free_time = ParamGetReal(param, "free_time") * FRAME_RATE;
			var->lost_time = weaponLostTime(lParam, param);
		}
		break;

	case MSG_GAME_WEAPON_UPDATE:
		{
			sOBJ *parent = (sOBJ *)StkRefFrameP(0);
			WeaponLink *link = (WeaponLink *)StkRefFrameP(1);

			float dist = var->radius + link->distance;
			FVector2 *parent_pos = ObjGetPos(parent);
			FVector2 d;
			SetV2d(&d, parent_pos->x - var->pos.x, parent_pos->y - var->pos.y);
			MathNormalize(&d, &d);
			MathScalar(&d, dist);

			float r = NormalAngle(MathVctAngleY(&d) - ObjGetDir(parent));
			float dr = DifAngle(r, link->r);
			if(dr > link->dr)
			{
				MathCalcVector(&d, NormalAngle(ObjGetDir(parent) + link->r + link->dr), dist);
			}
			else
			if(dr < -link->dr)
			{
				MathCalcVector(&d, NormalAngle(ObjGetDir(parent) + link->r - link->dr), dist);
			}

			float x = parent_pos->x - d.x;
			float y = parent_pos->y - d.y;
			var->vct.x = x - var->pos.x;
			var->vct.y = y - var->pos.y;
			var->pos.x = x;
			var->pos.y = y;

			r = MathVctAngleY(&d);
			var->vr = DifAngle(r, var->dir);
			var->dir = r;

			ObjSetPos(obj, var->pos.x, var->pos.y);
			ObjSetDir(obj, var->dir);
			ObjSetVct(obj, var->vct.x, var->vct.y);

			void *p = ObjLinkGetLast(var->weapons);
			while(p)
			{
				WeaponLink *childLink = (WeaponLink *)p;
				childLink->shot = link->shot;

				StkMakeFrame();
				StkPushP(obj);								// 0
				StkPushP(childLink);						// 1
				ObjPostMsg(childLink->obj, MSG_GAME_WEAPON_UPDATE, 0, 0);
				StkDelFrame();

				p = ObjLinkGetPrev(p);
			}
		}
		break;

	case MSG_GAME_WEAPON_IS_FIX:
		{
			res =  ((var->mode == MODE_FIXED) && (var->free_time == 0) && (ObjLinkGetNum(var->weapons) < var->weapon_max)) ? TRUE : FALSE;
		}
		break;

	case MSG_GAME_WEAPON_CHECK_LEVEL:
		{
			int *level = (int *)StkRefFrameP(0);
			*(level + var->type) += 1;

			void *p = ObjLinkGetLast(var->weapons);
			while(p)
			{
				WeaponLink *childLink = (WeaponLink *)p;
				ObjPostMsg(childLink->obj, MSG_GAME_WEAPON_CHECK_LEVEL, 0, 0);
				p = ObjLinkGetPrev(p);
			}
		}
		break;

	case MSG_GAME_WEAPON_FIX:
		{
			sOBJ *parent = (sOBJ *)StkRefFrameP(0);
			var->mode = MODE_FIXED;
			var->fix_prio = lParam;

			StkMakeFrame();
			StkPushP(obj);
			ObjPostMsg(parent, MSG_GAME_WEAPON_FIXED, var->type, var->level);
			StkDelFrame();
		}
		break;

	case MSG_GAME_WEAPON_TYPE:
		{
			res = var->type;
		}
		break;

	case MSG_GAME_WEAPON_LEVEL:
		{
			res = var->level;
		}
		break;

	case MSG_GAME_BOSS_OBJKILL:
		{
			if(var->mode == MODE_FREE)
			{
				sOBJ *parent = (sOBJ *)StkRefFrameP(0);
				FVector2 *pos = ObjGetPos(parent);
				FVector2 d;
				SubV2d(&d, &var->pos, pos);
				float dist = MathLength(&d);
				if(dist > 0.0f)
				{
					MathNormalize(&d, &d);
					float vect = 100.0f + RndCH(RND_CH1) * 100.0f;
					var->vct.x += vect * d.x / dist;
					var->vct.y += vect * d.y / dist;
					var->vr += 0.05f * RndmCH(RND_CH1);
				}
			}
		}
		break;
	}

	return res;
}

sOBJ *WeaponCreate(int type, int level, sParam *param)
{
	char id_str[ID_MAXLEN];
	sprintf(id_str, "weapon%d", level + 1);
	
	StkMakeFrame();
	StkPushP(param);								// 0
	sOBJ *obj = ObjCreate(id_str, OBJ_WEAPON, objProc, type, 0);
	StkDelFrame();

	return obj;
}

void WeaponFix(sOBJ *obj, sOBJ *child, sLink *weapons, int weapon_max, float dist)
{
	float r1_tbl[] = {
		0.0f,
	};
	float r2_tbl[] = {
		PI / 4.0f + PI / 8.0f,
		-PI / 4.0f - PI / 8.0f,
	};
	float r3_tbl[] = {
		0.0f,
		PI / 2.0f,
		-PI / 2.0f,
	};
	float r4_tbl[] = {
		PI / 4.0f + PI / 8.0f,
		-PI / 4.0f - PI / 8.0f,
		PI / 2.0f,
		-PI / 2.0f,
	};
	float *r_tbl[] = {
		0,
		r1_tbl,
		r2_tbl,
		r3_tbl,
		r4_tbl,
	};

	u_int mask = 0;
	void *p = ObjLinkGetLast(weapons);
	while(p)
	{
		WeaponLink *link = (WeaponLink *)p;
		mask |= (0x1 << link->index);
		p = ObjLinkGetPrev(p);
	}

	int index = getLowBitValue(~mask);
	WeaponLink *link = (WeaponLink *)ObjLinkNew(weapons);
	ASSERT(link);
	link->obj = child;
	link->r = *(r_tbl[weapon_max] + index);
	link->dr = PI / 4.0f;
	link->index = index;
	link->distance = dist;
}

/* 一個専用 */
void WeaponFixOne(sOBJ *obj, sOBJ *child, sLink *weapons, float dist)
{
	void *p = ObjLinkGetLast(weapons);
	if(p)
	{
		WeaponLink *link = (WeaponLink *)p;
		StkMakeFrame();
		StkPushP(child);
		ObjPostMsg(link->obj, MSG_GAME_WEAPON_FIXED, 0, 0);
		StkDelFrame();
	}
	else
	{
		WeaponLink *link = (WeaponLink *)ObjLinkNew(weapons);
		ASSERT(link);
		link->obj = child;
		link->dr = PI / 4.0f;
		link->distance = dist;
	}
}

void WeaponUptate(sOBJ *obj, sLink *weapons, BOOL shot, u_int target_flag)
{
	void *p = ObjLinkGetLast(weapons);
	while(p)
	{
		WeaponLink *link = (WeaponLink *)p;
		link->shot = shot;
		link->target_flag = target_flag;
		sOBJ *child = link->obj;

		StkMakeFrame();
		StkPushP(obj);								// 0
		StkPushP(link);								// 1
		ObjPostMsg(child, MSG_GAME_WEAPON_UPDATE, 0, 0);
		StkDelFrame();
		
		p = ObjLinkGetPrev(p);
	}
}

void WeaponFreeOne(sOBJ *obj, sLink *weapons, int prio, int lost_time)
{
#if 1
	/* 根元からLOST */
	void *p = ObjLinkGetLast(weapons);
	if(p)
	{
		WeaponLink *link = (WeaponLink *)p;

		sOBJ *child = link->obj;
		ObjLinkDel(weapons, p);

		ObjVar *var = (ObjVar *)ObjGetVar(child, sizeof(ObjVar));
		p = ObjLinkGetLast(var->weapons);
		if(p)
		{
			link = (WeaponLink *)p;
			WeaponFixToObj(link->obj, obj, prio);
			ObjLinkDel(var->weapons, p);
		}
		
		ObjPostMsg(child, MSG_GAME_WEAPON_FREE, lost_time, 0);
	}
#else
	/* 最後尾からLOST */
	void *p = ObjLinkGetLast(weapons);
	while(p)
	{
		WeaponLink *link = (WeaponLink *)p;
		sOBJ *child = link->obj;
		int res = ObjPostMsg(child, MSG_GAME_WEAPON_FREE, lost_time, 0);
		void *next = ObjLinkGetPrev(p);
		if(res)
		{
			ObjLinkDel(weapons, p);
		}
		p = next;
	}
#endif
}

void WeaponFreeAll(sLink *weapons, int lost_time)
{
	void *p = ObjLinkGetLast(weapons);
	while(p)
	{
		WeaponLink *link = (WeaponLink *)p;
		sOBJ *child = link->obj;
		ObjPostMsg(child, MSG_GAME_WEAPON_FREE_ALL, lost_time, 0);
		void *next = ObjLinkGetPrev(p);
		ObjLinkDel(weapons, p);
		p = next;
	}
}

void WeaponFixToObj(sOBJ *obj, sOBJ *target, int prio)
{
	StkMakeFrame();
	StkPushP(target);
	ObjPostMsg(obj, MSG_GAME_WEAPON_FIX, prio, 0);
	StkDelFrame();
}

int WeaponUseOne(sOBJ *obj, sLink *weapons, int prio)
{
	int res = -1;
	void *p = ObjLinkGetLast(weapons);
	if(p)
	{
		WeaponLink *link = (WeaponLink *)p;

		sOBJ *child = link->obj;
		res = ObjPostMsg(child, MSG_GAME_WEAPON_TYPE, 0, 0);
		ObjLinkDel(weapons, p);

		ObjVar *var = (ObjVar *)ObjGetVar(child, sizeof(ObjVar));
		p = ObjLinkGetLast(var->weapons);
		if(p)
		{
			link = (WeaponLink *)p;
			WeaponFixToObj(link->obj, obj, prio);
		}
		ObjKillReq(child);
	}
	return res;
}
