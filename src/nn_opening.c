//
// オープニング
//

#include "nn_opening.h"
#include "co_task.h"
#include "co_memory.h"
#include "co_input.h"
#include "co_param.h"
#include "co_misc.h"
#include "co_graph.h"
#include "co_sound.h"
#include "co_texture.h"
#include "nn_main.h"
#include "nn_camera.h"
#include "nn_player.h"
#include "nn_sndeffect.h"
#include "nn_bg.h"
#include "nn_gamemisc.h"


typedef struct {
	sParam *param;
	sTexture *texture;

	int opening_delay;
	int opening_time;
	
	float decay;
	float camera_decay;
	int text_fade;
	int text_fade_cnt;
	BOOL disp_text;

	Sprite *text_sprite;
	int text_num;
} TaskVar;


static int mainProc(sTaskBody *body, int msg, int lParam, int rParam)
{
	int res = 0;
	TaskVar *var = (TaskVar *)TaskGetVar(body, sizeof(TaskVar), MEM_APP);
	switch(msg)
	{
	case MSG_CREATE:
		{
			sParam *param = ParamRead(PATH_DATA"/opening.param");
			var->param = param;
			var->opening_delay = ParamGetReal(param, "opening_delay") * FRAME_RATE;
			var->opening_time = ParamGetReal(param, "opening_time") * FRAME_RATE;
			var->decay = ParamGetReal(param, "decay");

			var->camera_decay = CameraGetDecay();
			CameraSetPos(ParamGetFVec2(param, "start_ofs"));
			CameraSetDecay(var->decay, 0);
			InputSetAppExec(INP_CH0, FALSE);

			FVector2 pos = {0.0f, 0.0f};
			PlayerSetPos(&pos);
			PlayerSetDir(ANG2RAD(-45.0f));

			BgResetTail();

			var->text_fade = ParamGetReal(param, "text_fade") * FRAME_RATE;
			var->texture = ParamGetTex(param, "texture");
			int i;
			for(i = 0; ; i += 1)
			{
				char id_str[ID_MAXLEN];
				sprintf(id_str, "%d.size", i + 1);
				if(!ParamIsExists(param, id_str)) break;
			}
			var->text_num = i;
			var->text_sprite = (Sprite *)appMalloc(sizeof(Sprite) * i, "opening");
			while(i > 0)
			{
				char id_str[ID_MAXLEN];

				sprintf(id_str, "%d", i);
				Sprite *spr = var->text_sprite + i - 1;
				SpriteSetup(spr, id_str, param);
				spr->col.alpha = 0.0f;
				
				i -= 1;
			}
			SndEffectReq("opening", 60, 1.0f);
		}
		break;
		
	case MSG_KILL:
		{
			Free(var->text_sprite);
			ParamDestroy(var->param);
			CameraSetDecay(var->camera_decay, 3.0f * FRAME_RATE);
		}
		break;


	case MSG_STEP:
		{
			BOOL abort = InputGetBtnTD(MOUSE_LEFT | MOUSE_D_LEFT);
			if(var->opening_delay > 0)
			{
				if(!(var->opening_delay -= 1))
				{
					var->disp_text = TRUE;					
				}
			}
			else
			{
				if(var->decay < 1.0f)
				{
					var->decay *= ParamGetReal(var->param, "decay_add");
					if(var->decay > var->camera_decay) var->decay = var->camera_decay;
				}
				CameraSetDecay(var->decay, 0);

				if(var->text_fade_cnt < var->text_fade)
				{
					var->text_fade_cnt += 1;
					float alpha = (float)var->text_fade_cnt / (float)var->text_fade;
					for(int i = 0; i < var->text_num; i += 1)
					{
						(var->text_sprite + i)->col.alpha = alpha;
					}
				}
				if(!abort) abort = !(var->opening_time -= 1);
			}

			if(abort)
			{
				var->disp_text = FALSE;
				InputSetAppExec(INP_CH0, TRUE);
				TaskPostMsgAll(TASK_PRI_NONE, MSG_GAME_OPENING_FIN, 0, 0);
				res = 1;
			}
		}
		break;
			
	case MSG_DRAW:
		{
			if(var->disp_text)
			{
				for(int i = 0; i < var->text_num; i += 1)
				{
					SpriteDraw(var->text_sprite + i, var->texture);
				}
			}
		}
		break;
	}

	return res;
}

void OpeningStart(void)
{
	TaskCreate("Opening", TASK_PRI_02, mainProc, 0, 0);	
}
