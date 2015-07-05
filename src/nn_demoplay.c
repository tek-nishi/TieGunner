//
// DEMO PLAY
//

#include "nn_demoplay.h"
#include "co_task.h"
#include "co_memory.h"
#include "co_texture.h"
#include "co_input.h"
#include "co_stack.h"
#include "nn_main.h"
#include "nn_gamemisc.h"
#include "nn_sndeffect.h"


typedef struct {
	sTexture *texture;
	Sprite demo;

	int time;
	int skip_delay;
} TaskVar;


static int mainProc(sTaskBody *body, int msg, int lParam, int rParam)
{
	int res = 0;
	TaskVar *var = (TaskVar *)TaskGetVar(body, sizeof(TaskVar), MEM_APP);
	switch(msg)
	{
	case MSG_CREATE:
		{
			sParam *param = (sParam *)StkRefFrameP(0);
			var->texture = ParamGetTex(param, "texture");
			SpriteSetup(&var->demo, "demo", param);
			var->time = ParamGetReal(param, "demo_time") * FRAME_RATE;
			var->skip_delay = ParamGetReal(param, "demo_skip") * FRAME_RATE;
		}
		break;

	case MSG_KILL:
		{
		}
		break;

	case MSG_STEP:
		{
			if(var->skip_delay > 0) var->skip_delay -= 1;
			if(var->time > 0) var->time -= 1;

			BOOL abort = !var->time || (!var->skip_delay && InputGetBtnTD(MOUSE_LEFT | MOUSE_D_LEFT));
			if(abort)
			{
				SndEffectStopAll(TRUE);
				TaskPostMsgAll(TASK_PRI_NONE, MSG_GAME_DEMOPLAY_FIN, 0, 0);
				res = 1;
			}
		}
		break;

	case MSG_DRAW:
		{
			if(g.time & 0x20) SpriteDraw(&var->demo, var->texture);
		}
		break;


	case MSG_GAME_PLAYER_DEAD:
		{
			if(var->time > (2 * FRAME_RATE)) var->time = 2 * FRAME_RATE;
		}
		break;
	}

	return res;
}

void DemoplayStart(sParam *param)
{
	StkMakeFrame();
	StkPushP(param);								// 0
	TaskCreate("Demoplay", TASK_PRI_02, mainProc, 0, 0);	
	StkDelFrame();

	TaskPostMsgAll(TASK_PRI_NONE, MSG_GAME_START, TRUE, 0);
}
