//
// カメラ処理
//

#include "nn_camera.h"
#include "co_obj.h"
#include "co_misc.h"
#include "co_stack.h"
#include "co_task.h"
#include "nn_player.h"
#include "nn_main.h"


typedef struct {
	FVector2 pos;
	FVector2 vct;
	float decay;
	float decay_push;
	BOOL no_player;

	float decay_start, decay_end;
	float decay_time, decay_frame;
} ObjVar;


static ObjVar *cameraVar;
static sOBJ *cameraObj;

static int objProc(sOBJ *obj, sParam *param, int msg, int lParam, int rParam)
{
	int res = 0;

	ObjVar *var = (ObjVar *)ObjGetVar(obj, sizeof(ObjVar));
	switch(msg)
	{
	case MSG_CREATE:
		{
			cameraObj = obj;
			cameraVar = var;
			var->vct = FVec2Zero;
			var->decay = 1.0f;
			var->no_player = TRUE;
		}
		break;

	case MSG_KILL:
		{
			cameraVar = 0;
			cameraObj = 0;
		}
		break;

	case MSG_STEP:
		{
			if(var->decay_frame < var->decay_time)
			{
				var->decay_frame += 1;
				float d = (float)var->decay_frame / (float)var->decay_time;
//				d = (2.0f - d) * d;					// 徐々に減速パターン
				d = d * d;							// 徐々に加速パターン
				var->decay = var->decay_start + (var->decay_end - var->decay_start) * d;
			}
		}
		break;

	case MSG_UPDATE:
		{
			if(var->no_player)
			{
				var->pos.x += var->vct.x;
				var->pos.y += var->vct.y;
			}
			else
			{
				sOBJ *target = PlayerGetObj();
				if(target)
				{
					FVector2 *pos = ObjGetPos(target);
					var->vct.x = (pos->x - var->pos.x) * var->decay;
					var->vct.y = (pos->y - var->pos.y) * var->decay;
				}
				var->pos.x += var->vct.x;
				var->pos.y += var->vct.y;
			}
			ObjSetPos(obj, var->pos.x, var->pos.y);

			FVector2 pos;
			SetV2d(&pos, var->pos.x - (float)(WINDOW_WIDTH / 2), var->pos.y - (float)(WINDOW_HEIGHT / 2));
			StkMakeFrame();
			StkPushP(&pos);				// 0
//			ObjPostMsgAll(OBJ_TYPE_ALL, MSG_GAME_CAMERA, FALSE, 0, 0);
			/* FIXME:イマイチ */
			TaskPostMsgAll(TASK_PRI_NONE, MSG_GAME_CAMERA, 0, 0);
			StkDelFrame();
		}
		break;

	case MSG_DRAW:
		{
		}
		break;

		
	case MSG_GAME_PLAYER_DEAD:
		{
			var->no_player = TRUE;
		}
		break;

	case MSG_GAME_OPENING:
	case MSG_GAME_TITLE:
		{
			var->no_player = FALSE;
		}
		break;

	case MSG_GAME_START:
		{
			var->no_player = FALSE;
		}
		break;
	}

	return res;
}


void CameraCreate(void)
{
	ObjCreate("camera", OBJ_CAMERA, objProc, 0, 0);
}

FVector2 *CameraGetPos(void)
{
	return cameraObj ? ObjGetPos(cameraObj) : (FVector2 *)&FVec2Zero;
}

void CameraSetPos(FVector2 *pos)
{
	if(cameraVar)
	{
		cameraVar->pos = *pos;
		ObjSetPos(cameraObj, pos->x, pos->y);
	}
}

FVector2 *CameraGetVct(void)
{
	FVector2 *vct = 0;
	if(cameraVar)
	{
		vct = &cameraVar->vct;
	}
	return vct;
}

void CameraSetVct(FVector2 *vct)
{
	if(cameraVar)
	{
		cameraVar->vct = *vct;
	}
}

void CameraSetDecay(float decay, int time)
{
	if(cameraVar)
	{
		cameraVar->decay_time = time;
		if(time <= 0)
		{
			cameraVar->decay = decay;
		}
		else
		{
			cameraVar->decay_end = decay;
			cameraVar->decay_start = cameraVar->decay;
			cameraVar->decay_frame = 0;
		}
	}
}

float CameraGetDecay(void)
{
	float decay = 0.0f;
	if(cameraVar)
	{
		decay = cameraVar->decay;
	}
	return decay;
}
