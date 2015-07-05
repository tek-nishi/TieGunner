//
// BOSS‚ðŒ©Ž¸‚Á‚½Žž‚Ìˆ—
//

#include "nn_bossdir.h"
#include "co_param.h"
#include "co_task.h"
#include "co_memory.h"
#include "co_graph.h"
#include "co_obj.h"
#include "co_stack.h"
#include "co_misc.h"
#include "nn_main.h"
#include "nn_camera.h"
#include "nn_gamemisc.h"


typedef struct {
	sParam *param;
	sTexture *texture;
	Sprite sprite;

	float alpha;

	sOBJ *target;
} TaskVar;


static int mainProc(sTaskBody *body, int msg, int lParam, int rParam)
{
	int res = 0;

	TaskVar *var = (TaskVar *)TaskGetVar(body, sizeof(TaskVar), MEM_APP);
	switch(msg)
	{
	case MSG_CREATE:
		{
			var->target = (sOBJ *)StkRefFrameP(0);
			sParam *param = ParamRead(PATH_DATA"/bossdir.param");
			var->param = param;
			var->texture = ParamGetTex(param, "texture");
			SpriteSetup(&var->sprite, "dir", param);
			FVector4 *col = ParamGetFVec4(param, "dir.col");
			var->sprite.col.red = col->x;
			var->sprite.col.green = col->y;
			var->sprite.col.blue = col->z;
			var->sprite.col.alpha = col->w;
		}
		break;

	case MSG_KILL:
		{
			ParamDestroy(var->param);
		}
		break;

	case MSG_STEP:
		{
			if(var->target)
			{
				FVector2 *cam = CameraGetPos();
				FVector2 *pos = ObjGetPos(var->target);
				FVector2 vct;
				SubV2d(&vct, pos, cam);
				float r = MathVctAngleY(&vct);
				var->sprite.rot = r;

				float dist = MathLength(&vct);
				BOOL disp = dist > ParamGetReal(var->param, "dist");
				//var->sprite.disp = disp;
				if(disp)
				{
					var->alpha += 0.1f;
					if(var->alpha > 1.0f) var->alpha = 1.0f;
				}
				else
				{
					var->alpha -= 0.1f;
					if(var->alpha < 0.0f) var->alpha = 0.0f;
				}
				float alpha = fabsf(sinf(g.time * 0.05f)) * var->alpha;
				var->sprite.col.alpha = alpha;
			}
			res = !var->target;
		}
		break;

	case MSG_DRAW:
		{
			SpriteDraw(&var->sprite, var->texture);
		}
		break;

		
	case MSG_GAME_PLAYER_DEAD:
		{
			var->target = 0;
		}
		break;
		
	case MSG_GAME_BOSS_DESTROY:
		{
			sOBJ *obj = (sOBJ *)StkRefFrameP(0);
			if(var->target == obj)
			{
				var->target = 0;
			}
		}
		break;
	}

	return res;
}

void BossDirStart(sOBJ *target)
{
	StkMakeFrame();
	StkPushP(target);								// 0
	TaskCreate("BossDir", TASK_PRI_03, mainProc, 0, 0);
	StkDelFrame();
}

