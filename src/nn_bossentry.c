//
// BOSS登場演出
//

#include "nn_bossentry.h"
#include "co_task.h"
#include "co_stack.h"
#include "co_memory.h"
#include "co_objlink.h"
#include "co_graph.h"
#include "co_misc.h"
#include "nn_main.h"
#include "nn_gamemisc.h"
#include "nn_sndeffect.h"


typedef struct {
	Sprite sprite;

	int count;
	int count_end;

	FVector2 scale_start;
	FVector2 scale_end;
} EmargenVar;

typedef struct {
	sParam *param;
	sTexture *texture;
	
	Sprite boss_ent1;
	Sprite boss_ent2;
	Sprite boss_ent3;
	Sprite boss_ent4;

	int mode;

	int disp_count;

	int emargen_delay;
	sLink *emar_link;
	int emar_intr;
	int emar_intr2;
	int emar_intr3;
	int emar_num;
	BOOL se_play;

	int sub_intr;
	int sub_count;

	int move_frame;
	int move_count;

	FVector2 ent1_start;
	FVector2 ent1_end;
	FVector2 ent2_start;
	FVector2 ent2_end;
	FVector2 ent3_start;
	FVector2 ent3_end;
} BossVar;


static BOOL stepEmargency(BossVar *var)
{
	if(var->emargen_delay > 0) var->emargen_delay -= 1;
	if(var->emargen_delay == 0)
	{
		if(var->emar_intr2 > 0) var->emar_intr2 -= 1;
		if(var->emar_intr2 == 0)
		{
			if(var->emar_intr3 > 0) var->emar_intr3 -= 1;
			if(var->emar_intr3 == 0)
			{
				FVector2 *val = ParamGetFVec2(var->param, "emargen.intr2");
				var->emar_intr2 = val->x * FRAME_RATE;
				var->emar_intr3 = val->y * FRAME_RATE;
				var->se_play = FALSE;
				var->emar_num -= 1;
				if(var->emar_num == 0)  var->emargen_delay = -1;
			}
			else
			{
				int emar_intr = ParamGetReal(var->param, "emargen.intr") * FRAME_RATE;
				if(++var->emar_intr >= emar_intr)
				{
					var->emar_intr = 0;

					EmargenVar *emargen = (EmargenVar *)ObjLinkNew(var->emar_link);
					ASSERT(emargen);
					emargen->sprite = var->boss_ent4;
					emargen->count_end = ParamGetReal(var->param, "emargen.count") * FRAME_RATE;
					emargen->scale_start.x = 1.0f;
					emargen->scale_start.y = 1.0f;
					emargen->scale_end = *ParamGetFVec2(var->param, "emargen.scale");

					if(!var->se_play)
					{
						SndEffectReq("boss_enter", 0, 0.5f);
						var->se_play = TRUE;
					}
				}
			}
		}
	}
	
	return var->emargen_delay < 0;
}


static int bossEntryProc(sTaskBody *body, int msg, int lParam, int rParam)
{
	int res = 0;

	BossVar *var = (BossVar *)TaskGetVar(body, sizeof(BossVar), MEM_APP);
	switch(msg)
	{
	case MSG_CREATE:
		{
			sParam *param = ParamRead(PATH_DATA"/bossentry.param");
			var->param = param;
			var->texture = ParamGetTex(var->param, "texture");

			SpriteSetup(&var->boss_ent1, "boss_e1", param);
			SpriteSetup(&var->boss_ent2, "boss_e2", param);
			SpriteSetup(&var->boss_ent3, "boss_e3", param);
			SpriteSetup(&var->boss_ent4, "boss_e4", param);
//			var->boss_ent4.blend = GRP_BLEND_ADD;
			var->boss_ent1.size.x = 0.0f;
			var->boss_ent2.size.x = 0.0f;

			var->disp_count = ParamGetReal(param, "disp_count") * FRAME_RATE;
			var->emar_link = ObjLinkCreate(sizeof(EmargenVar), 16, MEM_APP, FALSE);
			var->emar_intr = 1000;					/* 意味なく大きな値 */
			var->emargen_delay = ParamGetReal(param, "emargen.delay") * FRAME_RATE;
			var->emar_num = ParamGetReal(param, "emargen.num");

			FVector2 *val = ParamGetFVec2(var->param, "emargen.intr2");
			var->emar_intr3 = val->y * FRAME_RATE;

			var->sub_intr = ParamGetReal(param, "sub_delay") * FRAME_RATE;

			var->move_frame = ParamGetReal(param, "emargen.frame") * FRAME_RATE;
		}
		break;

	case MSG_KILL:
		{
			ObjLinkDestroy(var->emar_link);
			ParamDestroy(var->param);
		}
		break;

	case MSG_STEP:
		{
			switch(var->mode)
			{
			case 0:
				{
					if(stepEmargency(var))
					{
						var->mode += 1;
						var->emar_intr = ParamGetReal(var->param, "emargen.intr3") * FRAME_RATE;

						var->ent1_start = var->boss_ent1.pos;
						var->ent1_end = var->boss_ent1.pos;
						var->ent1_end.y = 512.0f;

						var->ent2_start = var->boss_ent2.pos;
						var->ent2_end = var->boss_ent2.pos;
						var->ent2_end.y = 512.0f;

						var->ent3_start = var->boss_ent3.pos;
						var->ent3_end = var->boss_ent3.pos;
						var->ent3_end.y = -30.0f;
					}
				}
				break;

			case 1:
				{
					if(var->emar_intr > 0)
					{
						var->emar_intr -= 1;
						if(var->emar_intr == 0) SndEffectReq("title_fin", 0.05f * FRAME_RATE, 0.3f);
					}
					else
					{
						if(var->move_count < var->move_frame)
						{
							var->move_count += 1;

							float d = (float)var->move_count / (float)var->move_frame;
							d = d * d;
/* 						d = (2.0f - d) * d; */
							var->boss_ent1.pos.x = var->ent1_start.x + (var->ent1_end.x - var->ent1_start.x) * d;
							var->boss_ent1.pos.y = var->ent1_start.y + (var->ent1_end.y - var->ent1_start.y) * d;
							var->boss_ent2.pos.x = var->ent2_start.x + (var->ent2_end.x - var->ent2_start.x) * d;
							var->boss_ent2.pos.y = var->ent2_start.y + (var->ent2_end.y - var->ent2_start.y) * d;
							var->boss_ent3.pos.x = var->ent3_start.x + (var->ent3_end.x - var->ent3_start.x) * d;
							var->boss_ent3.pos.y = var->ent3_start.y + (var->ent3_end.y - var->ent3_start.y) * d;
						}
					}
				}
				break;
			}

			void *p = ObjLinkGetTop(var->emar_link);
			while(p)
			{
				void *next = ObjLinkGetNext(p);
				EmargenVar *emargen = (EmargenVar *)p;
				emargen->count += 1;

				float d = (float)emargen->count / (float)emargen->count_end;
				float s = -(d * d) + 2.0f * d;		// 二次曲線的な動き
				float scale_x = emargen->scale_start.x + (emargen->scale_end.x - emargen->scale_start.x) * s;
				float scale_y = emargen->scale_start.y + (emargen->scale_end.y - emargen->scale_start.y) * s;
				SetV2d(&emargen->sprite.scale, scale_x, scale_y);

				float alpha = 1.0f - d;
				emargen->sprite.col.alpha = alpha;
				if(emargen->count >= emargen->count_end)
				{
					ObjLinkDel(var->emar_link, p);
				}
				p = next;
			}

			if((var->sub_intr > 0) && !(--var->sub_intr))
			{
				char id_str[ID_MAXLEN];
				sprintf(id_str, "sub.%d", ++var->sub_count);
				if(ParamIsExists(var->param, id_str))
				{
					FVector3 *val = ParamGetFVec3(var->param, id_str);

					Sprite *spr = (val->z == 0.0f) ? &var->boss_ent1 : &var->boss_ent2;
					spr->size.x = val->x;
					spr->disp = TRUE;
					SndPlay("rank_key", 0.8f);
					
					var->sub_intr = val->y * FRAME_RATE;
				}
#if 0
				else
				{
					var->sub_intr = -1;
					/* 終了 */
				}
#endif
			}
				
			res = (var->disp_count > 0) && !(--var->disp_count);
		}
		break;

	case MSG_DRAW:
		{
			SpriteDraw(&var->boss_ent1, var->texture);
			SpriteDraw(&var->boss_ent2, var->texture);

			void *p = ObjLinkGetTop(var->emar_link);
			while(p)
			{
				EmargenVar *emargen = (EmargenVar *)p;
				SpriteDraw(&emargen->sprite, var->texture);
				p = ObjLinkGetNext(p);
			}
			SpriteDraw(&var->boss_ent3, var->texture);
		}
		break;


	case MSG_GAME_PLAYER_DEAD:
		{
			TaskDeleteReq(body);
		}
		break;
	}

	return res;
}


void BossEntryStart(void)
{
	TaskCreate("BossEnter", TASK_PRI_02, bossEntryProc, 0, 0);	
}
