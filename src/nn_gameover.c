//
// ゲームオーバー画面
//

#include "nn_gameover.h"
#include "co_task.h"
#include "co_memory.h"
#include "co_input.h"
#include "co_param.h"
#include "co_misc.h"
#include "co_graph.h"
#include "co_obj.h"
#include "nn_main.h"
#include "nn_sndeffect.h"
#include "nn_gamemisc.h"


#define GAMEOVER_TASK_DELAY  5


typedef struct {
	sParam *param;
	sTexture *texture;

	Sprite game;
	Sprite over;

	FVector2 game_end;
	FVector2 game_start;
	FVector2 over_end;
	FVector2 over_start;

	int move_delay;

	int move_count;
	int move_count_max;
	
	int gameover_count;
	int gameover_kill;
	BOOL disp;
} GameOverVar;


static int mainProc(sTaskBody *body, int msg, int lParam, int rParam)
{
	int res = 0;
	GameOverVar *var = (GameOverVar *)TaskGetVar(body, sizeof(GameOverVar), MEM_APP);
	switch(msg)
	{
	case MSG_CREATE:
		{
			sParam *param = ParamRead(PATH_DATA"/gameover.param");
			var->param = param;
			var->texture = ParamGetTex(param, "texture");

			int delay = ParamGetReal(param, "gameover_delay") * FRAME_RATE;
			if(delay > 0) TaskSleep(body, delay);

			var->gameover_count = ParamGetReal(param, "gameover_count") * FRAME_RATE;

			SpriteSetup(&var->game, "game", param);
			SpriteSetup(&var->over, "over", param);

			var->game_start = var->game.pos;
			var->over_start = var->over.pos;
			
			SetV2d(&var->game_end, 0.0f - var->game.size.x, 230.0f);
			SetV2d(&var->over_end, 512.0f, 230.0f);

			var->move_delay = ParamGetReal(param, "move_delay") * FRAME_RATE;
			var->move_count_max = ParamGetReal(param, "move_count") * FRAME_RATE;
			
			SndEffectReq("gameover", ParamGetReal(param, "sound_delay") * FRAME_RATE, 1.0f);
		}
		break;
		
	case MSG_KILL:
		{
			ParamDestroy(var->param);
		}
		break;


	case MSG_STEP:
		{
			if(var->move_delay > 0)
			{
				var->move_delay -= 1;
			}
			else
			if(var->move_count < var->move_count_max)
			{
				var->move_count += 1;
					
				float d = 1.0f - (float)var->move_count / (float)var->move_count_max;
					
				d = -(d * d) + 2.0f * d;		// 二次曲線的な動き
				var->game.pos.x = var->game_end.x + (var->game_start.x - var->game_end.x) * d;
				var->over.pos.x = var->over_end.x + (var->over_start.x - var->over_end.x) * d;
			}

			if(var->gameover_kill > 0)
			{
				if(!(var->gameover_kill -= 1))
				{
					res = 1;
				}
			}
			else
			if(!(var->gameover_count -= 1))
			{
				TaskPostMsgAll(TASK_PRI_NONE, MSG_GAME_RANKING, TRUE, 0);
				var->gameover_kill = ParamGetReal(var->param, "gameover_kill") * FRAME_RATE;
			}
		}
		break;
			
	case MSG_DRAW:
		{
			SpriteDraw(&var->game, var->texture);
			SpriteDraw(&var->over, var->texture);
		}
		break;
	}

	return res;
}

static int subProc(sTaskBody *body, int msg, int lParam, int rParam)
{
	int res = 0;
	switch(msg)
	{
	case MSG_CREATE:
		{
			TaskSleep(body, lParam);
		}
		break;

	case MSG_STEP:
		{
			TaskCreate("GameOver", TASK_PRI_02, mainProc, 0, 0);
			res = 1;
		}
		break;
	}
	return res;
}

void GameOverStart(void)
{
	TaskCreate("GameOver", TASK_PRI_02, subProc, GAMEOVER_TASK_DELAY, 0);
	/* Player死亡時の処理負荷を分散する措置 */
}
