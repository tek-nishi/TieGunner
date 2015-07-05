//
// BOSS撃破演出
//

#include "nn_bossclear.h"
#include "co_task.h"
#include "co_stack.h"
#include "co_memory.h"
#include "co_graph.h"
#include "co_misc.h"
#include "nn_main.h"
#include "nn_gamemisc.h"
#include "nn_sndeffect.h"


enum {
	SPRITE_TARGET,
	SPRITE_DEFEAT,
	SPRITE_SUB1,
	SPRITE_SUB2,
	SPRITE_SUB3,
	SPRITE_NUM
};

enum {
	TITLE_SUB1,
	TITLE_SUB2,
	TITLE_SUB3,
	TITLE_SUB_NUM
};

enum {
	DISP_IN,
	DISP_OUT
};


typedef struct {
	sParam *param;
	sTexture *texture;

	Sprite sprite[SPRITE_NUM];
	FVector2 start_pos[SPRITE_NUM];
	FVector2 end_pos[SPRITE_NUM];
	
	int sub_index[TITLE_SUB_NUM];
	int sub_delay[TITLE_SUB_NUM];

	int time;
	int total_time;
	int mode;
	int next_time;
} TaskVar;


/* サブタイトルを一文字づつ表示する */
static void subtitleExec(int id, TaskVar *var, sParam *param)
{
	if((var->sub_delay[id] > 0) && !--var->sub_delay[id])
	{
		char *tbl[] = {
			"sub1",
			"sub2",
			"sub3",
		};
		int idx_tbl[] = {
			SPRITE_SUB1,			
			SPRITE_SUB2,			
			SPRITE_SUB3,			
		};
		
		char id_str[ID_MAXLEN];

		int index = var->sub_index[id];
		sprintf(id_str, "%s.delay.%d", tbl[id], index + 1);
		if(ParamIsExists(param, id_str))
		{
			var->sub_delay[id] = ParamGetReal(param, id_str) * FRAME_RATE;

			sprintf(id_str, "%s.size_x.%d", tbl[id], index + 1);
			Sprite *sprite = &var->sprite[idx_tbl[id]];
			sprite->size.x = ParamGetReal(param, id_str);
			
			var->sub_index[id] = index + 1;
			SndPlay("rank_key", 0.8f);
		}
	}
}


static int mainProc(sTaskBody *body, int msg, int lParam, int rParam)
{
	int res = 0;

	TaskVar *var = (TaskVar *)TaskGetVar(body, sizeof(TaskVar), MEM_APP);
	switch(msg)
	{
	case MSG_CREATE:
		{
			sParam *param = ParamRead(PATH_DATA"/bossclear.param");
			var->param = param;
			var->texture = ParamGetTex(var->param, "texture");

			char *tbl[] = {
				"target",
				"defeat",
				"sub1",
				"sub2",
				"sub3",
			};

			for(int i = 0; i < SPRITE_NUM; i += 1)
			{
				SpriteSetup(&var->sprite[i], tbl[i], param);
			}

			var->end_pos[SPRITE_TARGET] = var->sprite[SPRITE_TARGET].pos;
			var->sprite[SPRITE_TARGET].pos.x = ParamGetReal(param, "target.st_x");
			var->end_pos[SPRITE_DEFEAT] = var->sprite[SPRITE_DEFEAT].pos;
			var->sprite[SPRITE_DEFEAT].pos.x = ParamGetReal(param, "defeat.st_x");

			for(int i = 0; i < TITLE_SUB_NUM; i += 1)
			{
				var->sub_delay[i] = 1;
			}

			var->time = ParamGetReal(param, "disp_time") * FRAME_RATE;
			var->next_time = ParamGetReal(param, "next_time") * FRAME_RATE;
			int delay = ParamGetReal(param, "delay") * FRAME_RATE;
			TaskSleep(body, delay);
			delay = ParamGetReal(param, "bgm_delay") * FRAME_RATE;
			SndEffectReq("boss_clear", delay, 1.0f);
		}
		break;

	case MSG_KILL:
		{
			ParamDestroy(var->param);
		}
		break;

	case MSG_STEP:
		{
			if((var->next_time > 0) && ((--var->next_time) == 0))
			{
				TaskPostMsgAll(TASK_PRI_NONE, MSG_GAME_BOSS_CLEAR_FIN, 0, 0);
				res = TRUE;
			}
			
			switch(var->mode)
			{
			case DISP_IN:
				{
					for(int i = 0; i < 2; i += 1)
					{
						var->sprite[i].pos.x += (var->end_pos[i].x - var->sprite[i].pos.x) * 0.15f;
					}
					for(int i = 0; i < TITLE_SUB_NUM; i += 1)
					{
						subtitleExec(i, var, var->param);
					}
					if((var->time -= 1) == 0)
					{
						var->mode += 1;
						var->time = ParamGetReal(var->param, "out_time") * FRAME_RATE;
						var->total_time = var->time;

						for(int i = 0; i < SPRITE_NUM; i += 1)
						{
							var->start_pos[i] = var->sprite[i].pos;
							var->end_pos[i] = var->sprite[i].pos;
						}
						var->end_pos[SPRITE_TARGET].y = -var->sprite[SPRITE_TARGET].size.y;
						var->end_pos[SPRITE_DEFEAT].y = -var->sprite[SPRITE_DEFEAT].size.y;
						var->end_pos[SPRITE_SUB1].y = 512.0f;
						var->end_pos[SPRITE_SUB2].y = 512.0f;
						var->end_pos[SPRITE_SUB3].y = 512.0f + var->sprite[SPRITE_SUB3].pos.y - var->sprite[SPRITE_SUB1].pos.y;
						SndEffectReq("title_fin", 0.05f * FRAME_RATE, 0.3f);
					}
				}
				break;

			case DISP_OUT:
				{
					var->time -= 1;
					float d = 1.0f - (float)var->time / (float)var->total_time;
					d = d * d;
					for(int i = 0; i < SPRITE_NUM; i += 1)
					{
						var->sprite[i].pos.x = var->start_pos[i].x + (var->end_pos[i].x - var->start_pos[i].x) * d;
						var->sprite[i].pos.y = var->start_pos[i].y + (var->end_pos[i].y - var->start_pos[i].y) * d;
					}

					if(var->time == 0)
					{
						var->mode += 1;
						for(int i = 0; i < SPRITE_NUM; i += 1)
						{
							var->sprite[i].disp = FALSE;
						}
					}
//					res = (var->time == 0);
				}
				break;
			}
		}
		break;

	case MSG_DRAW:
		{
			for(int i = 0; i < SPRITE_NUM; i += 1)
			{
				SpriteDraw(&var->sprite[i], var->texture);
			}
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

void BossClearStart(void)
{
	TaskCreate("BossClear", TASK_PRI_02, mainProc, 0, 0);	
}
