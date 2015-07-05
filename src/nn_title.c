//
// タイトル画面
//

#include "nn_title.h"
#include "co_task.h"
#include "co_memory.h"
#include "co_input.h"
#include "co_param.h"
#include "co_misc.h"
#include "co_graph.h"
#include "co_sound.h"
#include "co_stack.h"
#include "nn_main.h"
#include "nn_player.h"
#include "nn_camera.h"
#include "nn_sndeffect.h"
#include "nn_gamemisc.h"


#define LOGO_CHARANUM  9


typedef struct {
	Sprite *obj;
	int count;
	int frame;
	BOOL slow;

	FVector2 pos_start;
	FVector2 pos_end;
	FVector2 scale_start;
	FVector2 scale_end;
} SpriteVar;

typedef struct {
	Sprite *logo;
	FVector2 logo_pos[LOGO_CHARANUM];
	float logo_eft;
	BOOL inited;
	BOOL kill_req;
} LogoVar;

typedef struct {
	sParam *param;
	sTexture *texture;

	Sprite logo[LOGO_CHARANUM];

	Sprite game;
	Sprite start;
	Sprite copyright;
	Sprite ngs;
	Sprite cursor[2];
	FVector2 cursor_pos[2];

	int start_delay;
	float start_scale;
	float start_fade;
	BOOL start_cur;
	BOOL started;
	int start_count;
	int abort_count;

	int start_eft_mode;

	/* Volume */
	Sprite speker;
	Sprite vol[3];
	Sprite vol_var[2];

	BOOL vol_slider;
	BOOL chg_gain;
	int vol_slider_x;
} TitleVar;


static int spriteProc(sTaskBody *body, int msg, int lParam, int rParam)
{
	int res = 0;
	SpriteVar *var = (SpriteVar *)TaskGetVar(body, sizeof(SpriteVar), MEM_APP);
	switch(msg)
	{
	case MSG_CREATE:
		{
			char *name = (char *)StkRefFrameP(0);
			var->obj = (Sprite *)StkRefFrameP(1);
			sParam *param = (sParam *)StkRefFrameP(2);
			int delay = StkRefFrameI(3);

			char id_str[ID_MAXLEN];

			sprintf(id_str, "%s.delay", name);
			delay += ParamGetReal(param, id_str) * FRAME_RATE;
			if(delay > 0) TaskSleep(body, delay);
			sprintf(id_str, "%s.frame", name);
			var->frame = ParamGetReal(param, id_str) * FRAME_RATE;

			sprintf(id_str, "%s.slow", name);
			var->slow = ParamGetReal(param, id_str);

			sprintf(id_str, "%s.start", name);
			if(ParamIsExists(param, id_str))
			{
				var->pos_start = *ParamGetFVec2(param, id_str);
			}
			else
			{
				var->pos_start = var->obj->pos;
			}

			sprintf(id_str, "%s.end", name);
			if(ParamIsExists(param, id_str))
			{
				var->pos_end = *ParamGetFVec2(param, id_str);
			}
			else
			{
				var->pos_end = var->obj->pos;
			}
			var->obj->pos = var->pos_start;

			sprintf(id_str, "%s.s_sc", name);
			if(ParamIsExists(param, id_str))
			{
				var->scale_start = *ParamGetFVec2(param, id_str);
			}
			else
			{
				var->scale_start = var->obj->scale;
			}

			sprintf(id_str, "%s.e_sc", name);
			if(ParamIsExists(param, id_str))
			{
				var->scale_end = *ParamGetFVec2(param, id_str);
			}
			else
			{
				var->scale_end = var->obj->scale;
			}
			var->obj->scale = var->scale_start;
		}
		break;

	case MSG_KILL:
		{
		}
		break;

	case MSG_STEP:
		{
			var->count += 1;
			float d = (float)var->count / (float)var->frame;

			if(var->slow) d = d * d;
			else          d = (2.0f - d) * d;
			var->obj->pos.x = var->pos_start.x + (var->pos_end.x - var->pos_start.x) * d;
			var->obj->pos.y = var->pos_start.y + (var->pos_end.y - var->pos_start.y) * d;
			var->obj->scale.x = var->scale_start.x + (var->scale_end.x - var->scale_start.x) * d;
			var->obj->scale.y = var->scale_start.y + (var->scale_end.y - var->scale_start.y) * d;

			res = (var->count == var->frame);
		}
		break;

		
	case MSG_GAME_TITLE_SKIP_EFFECT:
		{
			TaskAwake(body);
			var->count = var->frame - 1;
		}
		break;
	}
	return res;
}


static void createSpriteTask(Sprite *obj, char *id_str, sParam *param, int delay)
{
	StkMakeFrame();
	StkPushP(id_str);
	StkPushP(obj);
	StkPushP(param);
	StkPushI(delay);
	TaskCreate("TitleSprite", TASK_PRI_03, spriteProc, 0, 0);	
	StkDelFrame();
}


static int logoProc(sTaskBody *body, int msg, int lParam, int rParam)
{
	int res = 0;
	LogoVar *var = (LogoVar *)TaskGetVar(body, sizeof(LogoVar), MEM_APP);
	switch(msg)
	{
	case MSG_CREATE:
		{
			var->logo = (Sprite *)StkRefFrameP(0);
		}
		break;

	case MSG_KILL:
		{
		}
		break;

	case MSG_STEP:
		{
			if(!var->inited)
			{
				var->inited = TRUE;
				for(int i = 0; i < LOGO_CHARANUM; i += 1)
				{
					var->logo_pos[i] = var->logo[i].pos;
				}
			}
			
			if(var->kill_req)
			{
				var->logo_eft *= 0.95;
				if(var->logo_eft < 0.01f)
				{
					var->logo_eft = 0.0f;
					res = 1;
				}
			}
			else
			{
				var->logo_eft += 0.01;
				if(var->logo_eft > 1.0f) var->logo_eft = 1.0f;
			}

			for(int i = 0; i < LOGO_CHARANUM; i += 1)
			{
				float x = sinf((float)(g.time + i * 45) * 0.02f) * 5.0f;
				float y = cosf((float)(g.time + i * 30 + 50) * 0.03f) * 4.0f;
				SetV2d(&var->logo[i].pos, var->logo_pos[i].x + x * var->logo_eft, var->logo_pos[i].y + y * var->logo_eft);
			}
		}
		break;

	case MSG_GAME_TITLE_LOGOEFT_FIN:
		{
			var->kill_req = TRUE;
		}
		break;
	}
	return res;
}

static void createLogoTask(Sprite *logo, int delay)
{
	StkMakeFrame();
	StkPushP(logo);
	sTaskBody *body = TaskCreate("TitleLogo", TASK_PRI_03, logoProc, 0, 0);
	if(delay > 0) TaskSleep(body, delay);
	StkDelFrame();
}

static void setupVolume(TitleVar *var)
{
	float gain = SndGetMasterGain();

	var->vol_var[1].size.x = 175.0f * gain;
	var->vol[0].disp = var->vol[1].disp = var->vol[2].disp = FALSE;
	if(gain > 0.7f)
	{
		var->vol[0].disp = TRUE;
	}
	else
	if(gain > 0.4f)
	{
		var->vol[1].disp = TRUE;
	}
	else
	if(gain > 0.1f)
	{
		var->vol[2].disp = TRUE;
	}
}

static BOOL sliderVolume(TitleVar *var)
{
	BOOL exec = FALSE;
	int x = InputGetMouseX();
	int y = InputGetMouseY();

	if(!var->vol_slider)
	{
		sBox box = { { 175, 395 }, { 370, 431 } };
		if(MathBoxCrossPoint(x, y, &box) && InputGetBtnTD(MOUSE_LEFT | MOUSE_D_LEFT))
		{
			/* Slider処理開始 */
			var->vol_slider = TRUE;
			var->vol_slider_x = x;
			var->chg_gain = TRUE;
			exec = TRUE;
		}
	}

	if(InputGetBtnP(MOUSE_LEFT) && var->vol_slider)
	{
		int x_val = (var->vol_slider_x + x) / 2;
		if(x_val < 185) x_val = 185;
		if(x_val > 360) x_val = 360;
		var->vol_slider_x = x_val;
		var->vol_var[1].size.x = x_val - 185;

		float gain = (float)(x_val - 185) / 175.0f;
		SndSetMasterGain(gain);
		setupVolume(var);
		exec = TRUE;
	}
	else
	{
		/* Slider処理終了 */
		var->vol_slider = FALSE;
	}
	return exec;
}

static int mainProc(sTaskBody *body, int msg, int lParam, int rParam)
{
	int res = 0;
	TitleVar *var = (TitleVar *)TaskGetVar(body, sizeof(TitleVar), MEM_APP);
	switch(msg)
	{
	case MSG_CREATE:
		{
			sParam *param = ParamRead(PATH_DATA"/title.param");
			var->param = param;
			var->texture = ParamGetTex(param, "texture");

			for(int i = 0; i < LOGO_CHARANUM; i += 1)
			{
				char id_str[ID_MAXLEN];
				sprintf(id_str, "%d", i + 1);
				SpriteSetup(&var->logo[i], id_str, param);
			}
			SpriteSetup(&var->game, "game", param);
			SpriteSetup(&var->start, "start", param);
			SpriteSetup(&var->copyright, "copy", param);
			SpriteSetup(&var->ngs, "ngs", param);
			SpriteSetup(&var->cursor[0], ">", param);
			SpriteSetup(&var->cursor[1], "<", param);
			var->cursor_pos[0] = var->cursor[0].pos;
			var->cursor_pos[1] = var->cursor[1].pos;

			var->start_fade = ParamGetReal(param, "start_fade");

			FVector2 *scale = ParamGetFVec2(param, "start_scale");
			var->start_scale = scale->x;
			var->game.scale.x = var->game.scale.y = var->start_scale;
			var->start.scale.x = var->start.scale.y = var->start_scale;

			var->start_cur = FALSE;
			var->started = FALSE;

			var->abort_count = ParamGetReal(param, "title_count") * FRAME_RATE;

			SpriteSetup(&var->speker, "vol", param);
			SpriteSetup(&var->vol[0], "vol1", param);
			SpriteSetup(&var->vol[1], "vol2", param);
			SpriteSetup(&var->vol[2], "vol3", param);
			SpriteSetup(&var->vol_var[0], "v_var1", param);
			SpriteSetup(&var->vol_var[1], "v_var2", param);

			if(lParam) SndEffectReq("title", 15, 0.8f);

			if(!rParam)
			{
				int delay = ParamGetReal(param, "title_delay") * FRAME_RATE;
				if(delay > 0) TaskSleep(body, delay);

				for(int i = 0; i < LOGO_CHARANUM; i += 1)
				{
					char id_str[ID_MAXLEN];
					sprintf(id_str, "%ds", i + 1);
					createSpriteTask(&var->logo[i], id_str, var->param, delay);
				}

				var->start_delay = ParamGetReal(param, "start_delay") * FRAME_RATE;
				var->game.disp = FALSE;
				var->start.disp = FALSE;
				var->copyright.disp = FALSE;
				var->ngs.disp = FALSE;
				var->cursor[0].disp = FALSE;
				var->cursor[1].disp = FALSE;

				var->speker.disp = FALSE;
				for(int i = 0; i < 3; i += 1)
				{
					var->vol[i].disp = FALSE;
				}
				for(int i = 0; i < 2; i += 1)
				{
					var->vol_var[i].disp = FALSE;
				}
			}
			else
			{
				createLogoTask(var->logo, 1 * FRAME_RATE);
				var->start_delay = 0;
				setupVolume(var);
			}

			BOOL exec = rParam;
			if(exec) exec = !InputGetBtnP(MOUSE_LEFT);
			InputSetAppExec(INP_CH0, exec);
			/* SHOTの暴発を避けるため、若干回りくどい */
		}
		break;
		
	case MSG_KILL:
		{
			if(var->chg_gain)
			{
				sParam *settings = MainGetSettings();
				ParamSetReal(settings, "gain", SndGetMasterGain());
				WriteGameSettings(settings);
			}
			ParamDestroy(var->param);
		}
		break;

	case MSG_PREPROC:
		{
			if(!var->started && (var->start_delay == 0))
			{
				BOOL slider = sliderVolume(var);
				if(slider && var->abort_count < (1 * FRAME_RATE)) var->abort_count = 1 * FRAME_RATE;
				if(!slider && var->abort_count > 0) var->abort_count -= 1;
			}
		}
		break;

	case MSG_STEP:
		{
			if(var->started)
			{
				switch(var->start_eft_mode)
				{
				case 0:
					{
						var->start_count -= 1;

						float alpha = (var->start_count & 0x2) ? 1.0f : 0.5f;
						if(var->start_count == 0)
						{
							var->start_eft_mode += 1;
							var->start_count = ParamGetReal(var->param, "end_delay") * FRAME_RATE;

							for(int i = 0; i < LOGO_CHARANUM; i += 1)
							{
								char id_str[ID_MAXLEN];
								sprintf(id_str, "%de", i + 1);
								createSpriteTask(&var->logo[i], id_str, var->param, 0);
							}
							createSpriteTask(&var->game, "game", var->param, 0);
							createSpriteTask(&var->start, "start", var->param, 0);
							createSpriteTask(&var->copyright, "copy", var->param, 0);
							createSpriteTask(&var->ngs, "ngs", var->param, 0);

							createSpriteTask(&var->speker, "vol", var->param, 0);
							createSpriteTask(&var->vol[0], "vol1", var->param, 0);
							createSpriteTask(&var->vol[1], "vol2", var->param, 0);
							createSpriteTask(&var->vol[2], "vol3", var->param, 0);
							createSpriteTask(&var->vol_var[0], "v_var1", var->param, 0);
							createSpriteTask(&var->vol_var[1], "v_var2", var->param, 0);

							SndEffectReq("title_fin", 0.4f * FRAME_RATE, 0.3f);
							
							alpha = 1.0f;
						}
						var->game.col.alpha = alpha;
						var->start.col.alpha = alpha;
					}
					break;

				case 1:
					{
						var->start_count -= 1;
						if(var->start_count <= 0)
						{
							InputSetAppBtnExec(INP_CH0, TRUE);
							TaskPostMsgAll(TASK_PRI_NONE, MSG_GAME_GAMESTART, 0, 0);
							res = 1;
						}
					}
					break;
				}
			}
			else
			if(var->start_delay > 0)
			{
				BOOL disp = var->start_delay -= 1;
				BOOL abort = InputGetBtnTD(MOUSE_LEFT | MOUSE_D_LEFT);
				if(abort)
				{
					disp = FALSE;
					TaskPostMsgAll(TASK_PRI_03, MSG_GAME_TITLE_SKIP_EFFECT, 0, 0);
					var->start_delay = 0;
				}

				if(!disp)
				{
					createLogoTask(var->logo, 1 * FRAME_RATE);
					var->game.disp = TRUE;
					var->start.disp = TRUE;
					var->copyright.disp = TRUE;
					var->ngs.disp = TRUE;
					var->cursor[0].disp = TRUE;
					var->cursor[1].disp = TRUE;
					var->speker.disp = TRUE;
					for(int i = 0; i < 2; i += 1)
					{
						var->vol_var[i].disp = TRUE;
					}
					setupVolume(var);

					if(!abort) InputSetAppExec(INP_CH0, TRUE);
				}
			}
			else
			{
				if(InputGetBtnTU(MOUSE_LEFT | MOUSE_D_LEFT))
				{
					InputSetAppExec(INP_CH0, TRUE);
					/* ボタンが離されるまで入力禁止 */
				}
				
				int x = InputGetMouseX();
				int y = InputGetMouseY();

				/* かなり強引な方法 */
				BOOL start = FALSE;
				sBox box = {{ 180 -10, 330 - 10 }, {328 + 10, 362 + 10} };
/* 				box.inf.x = 180 - 10; */
/* 				box.inf.y = 330 - 10; */
/* 				box.sup.x = 328 + 10; */
/* 				box.sup.y = 362 + 10; */

				FVector2 *scale = ParamGetFVec2(var->param, "start_scale");
				if(MathBoxCrossPoint(x, y, &box))
				{
					start = TRUE;
					var->start_scale += (scale->y - var->start_scale) * 0.4f;
					var->cursor[0].col.alpha -= 0.08f;
					var->cursor[1].col.alpha -= 0.08f;
					var->start_count = 0;
				}
				else
				{
					var->start_scale += (scale->x - var->start_scale) * 0.3f;
					var->cursor[0].col.alpha += 0.025f;
					var->cursor[1].col.alpha += 0.025f;
					var->start_count += 1;
				}
				var->game.scale.x = var->game.scale.y = var->start_scale;
				var->start.scale.x = var->start.scale.y = var->start_scale;

				if(var->cursor[0].col.alpha < 0.0f)
				{
					var->cursor[0].col.alpha = 0.0f;
					var->cursor[1].col.alpha = 0.0f;
				}
				else
				if(var->cursor[0].col.alpha > 1.0f)
				{
					var->cursor[0].col.alpha = 1.0f;
					var->cursor[1].col.alpha = 1.0f;
				}

				float alpha = 0.5f + cosf((float)var->start_count * var->start_fade) * 0.5f;
				var->game.col.alpha = alpha;
				var->start.col.alpha = alpha;

				{
					float ofs = fabsf(sinf((float)g.time * 0.08f)) * 8.0f;
					var->cursor[0].pos.x = var->cursor_pos[0].x - ofs;
					var->cursor[1].pos.x = var->cursor_pos[1].x + ofs;
				}
				
				if(var->start_cur != start)
				{
					if(start) SndEffectReq("start_touch", 0, 1.0f);
					InputSetAppBtnExec(INP_CH0, !start);
					var->start_cur = start;
				}
				
				if(start && InputGetBtnTD(MOUSE_LEFT | MOUSE_D_LEFT))
				{
					SndStop("bgm");
					SndEffectReq("start_push", 0, 1.0f);
					var->started = TRUE;
					var->start_eft_mode = 0;
					var->start_scale = 1.5f;
					var->start_count = ParamGetReal(var->param, "start_count") * FRAME_RATE;
					var->cursor[0].disp = FALSE;
					var->cursor[1].disp = FALSE;
					TaskPostMsgAll(TASK_PRI_03, MSG_GAME_TITLE_LOGOEFT_FIN, 0, 0);
				}
				else
				{
					if(var->abort_count == 0)
					{
						TaskDeleteAll(TASK_PRI_03);
						TaskPostMsgAll(TASK_PRI_NONE, MSG_GAME_TITLE_FIN, 0, 0);
						res = 1;
					}
				}
			}

#ifdef DEBUG
			if((!res) && (InputGetKey() == 'd'))
			{
				TaskDeleteAll(TASK_PRI_03);
				TaskPostMsgAll(TASK_PRI_NONE, MSG_GAME_START, TRUE, 0);
				res = 1;
			}
#endif
		}
		break;
			
	case MSG_DRAW:
		{
			for(int i = 0; i < LOGO_CHARANUM; i += 1)
			{
				SpriteDraw(&var->logo[i], var->texture);
			}
			SpriteDraw(&var->game, var->texture);
			SpriteDraw(&var->start, var->texture);
			SpriteDraw(&var->copyright, var->texture);
			SpriteDraw(&var->ngs, var->texture);
			SpriteDraw(&var->cursor[0], var->texture);
			SpriteDraw(&var->cursor[1], var->texture);

			SpriteDraw(&var->speker, var->texture);
			for(int i = 0; i < 3; i += 1)
			{
				SpriteDraw(&var->vol[i], var->texture);
			}
			SpriteDraw(&var->vol_var[0], var->texture);
			SpriteDraw(&var->vol_var[1], var->texture);
		}
		break;
	}

	return res;
}

void TitleStart(BOOL sound, BOOL skiped)
{
	TaskCreate("Title", TASK_PRI_02, mainProc, sound, !skiped);
}
