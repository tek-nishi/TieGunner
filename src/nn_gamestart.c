//
// ゲーム開始画面
//

#include "nn_gamestart.h"
#include "co_task.h"
#include "co_memory.h"
#include "co_input.h"
#include "co_param.h"
#include "co_misc.h"
#include "co_graph.h"
#include "co_obj.h"
#include "co_font.h"
#include "nn_sndeffect.h"
#include "nn_main.h"
#include "nn_gamemisc.h"


typedef struct {
	sParam *param;
	sTexture *texture;

	Sprite blast;
	Sprite off;
	Sprite sub[3];

	FVector2 blast_start;
	FVector2 blast_end;
	FVector2 off_start;
	FVector2 off_end;

	FVector2 sub_start[3];
	FVector2 sub_end[3];
	int sub_delay[3];

	int gamestart_count;
	int gamestart_count_max;
	int mode;
} GameStartVar;


static int mainProc(sTaskBody *body, int msg, int lParam, int rParam)
{
	int res = 0;
	GameStartVar *var = (GameStartVar *)TaskGetVar(body, sizeof(GameStartVar), MEM_APP);
	switch(msg)
	{
	case MSG_CREATE:
		{
			sParam *param = ParamRead(PATH_DATA"/gamestart.param");
			var->param = param;
			var->texture = ParamGetTex(param, "texture");
			var->gamestart_count = ParamGetReal(param, "text_in") * FRAME_RATE;

			SpriteSetup(&var->blast, "blast", param);
			var->blast_end = var->blast.pos;
			var->blast.pos.x = 0.0f - var->blast.size.x;

			SpriteSetup(&var->off, "off", param);
			var->off_end = var->off.pos;
			var->off.pos.x = 512.0f;

			char *tbl[] = {
				"destroy",
				"all",
				"enemies"
			};
			for(int i = 0; i < 3; i += 1)
			{
				SpriteSetup(&var->sub[i], tbl[i], param);

				var->sub[i].pos.x += var->sub[i].uv.x;
				var->sub_end[i] = var->sub[i].pos;
				var->sub[i].pos.y = 512.0f;
				
				FVector2 *val = ParamGetFVec2(param, "sub_delay");
				var->sub_delay[i] = val->x * FRAME_RATE + i * val->y * FRAME_RATE;
			}

			SndEffectReq("start", (0.25f + ParamGetReal(param, "se_delay")) * FRAME_RATE, 0.9f);
		}
		break;
		
	case MSG_KILL:
		{
			ParamDestroy(var->param);
		}
		break;


	case MSG_STEP:
		{
			switch(var->mode)
			{
			case 0:
				{
					var->blast.pos.x += (var->blast_end.x - var->blast.pos.x) * 0.15f;
					var->off.pos.x += (var->off_end.x - var->off.pos.x) * 0.15f;
					for(int i = 0; i < 3; i += 1)
					{
						if(var->sub_delay[i] > 0)
						{
							var->sub_delay[i] -= 1;
						}
						else
						{
							var->sub[i].pos.y += (var->sub_end[i].y - var->sub[i].pos.y) * 0.15f;
						}
					}

					if(!(var->gamestart_count -= 1))
					{
						var->blast.pos = var->blast_end;
						var->off.pos = var->off_end;
						for(int i = 0; i < 3; i += 1)
						{
							var->sub[i].pos.y = var->sub_end[i].y;
						}
						var->gamestart_count = ParamGetReal(var->param, "text_disp") * FRAME_RATE;
						var->mode += 1;
					}
				}
				break;

			case 1:
				{
					if(!(var->gamestart_count -= 1))
					{
						var->blast_start = var->blast.pos;
						var->blast_end.x = 512.0f;
						var->off_start = var->off.pos;
						var->off_end.x = 512.0f + var->off.pos.x - var->blast.pos.x;
						for(int i = 0; i < 3; i += 1)
						{
							var->sub_start[i] = var->sub[i].pos;
							var->sub_end[i].x = var->sub[i].pos.x - (var->sub[2].pos.x + var->sub[2].size.x);
						}

						var->gamestart_count = ParamGetReal(var->param, "text_out") * FRAME_RATE;
						var->gamestart_count_max = var->gamestart_count;
						TaskPostMsgAll(TASK_PRI_NONE, MSG_GAME_START, 0, 0);
						var->mode += 1;
					}
				}
				break;

			case 2:
				{
					float d = (float)var->gamestart_count / (float)var->gamestart_count_max;
					
					d = -(d * d) + 2.0f * d;		// 二次曲線的な動き
					var->blast.pos.x = var->blast_end.x + (var->blast_start.x - var->blast_end.x) * d;
					var->off.pos.x = var->off_end.x + (var->off_start.x - var->off_end.x) * d;
					for(int i = 0; i < 3; i += 1)
					{
						var->sub[i].pos.x = var->sub_end[i].x + (var->sub_start[i].x - var->sub_end[i].x) * d;
					}

					if(!(var->gamestart_count -= 1))
					{
						TaskPostMsgAll(TASK_PRI_NONE, MSG_GAME_GAMESTART_FIN, 0, 0);
						res = 1;
					}
				}
				break;
			}
		}
		break;
			
	case MSG_DRAW:
		{
			SpriteDraw(&var->blast, var->texture);
			SpriteDraw(&var->off, var->texture);
			for(int i = 0; i < 3; i += 1)
			{
				SpriteDraw(&var->sub[i], var->texture);
			}
		}
		break;
	}

	return res;
}

void GameStartStart(void)
{
	sTaskBody *body = TaskCreate("GameStart", TASK_PRI_02, mainProc, 0, 0);
	TaskSleep(body, 0.25f * FRAME_RATE);
}
