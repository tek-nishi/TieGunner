//
// ƒTƒEƒ“ƒhŠÇ—
//

#include "nn_sndeffect.h"
#include "co_objlink.h"
#include "co_memory.h"
#include "co_sound.h"
#include "co_task.h"
#include "co_strings.h"
#include "nn_main.h"


#define EFFECT_MAX_NUM  64


typedef struct {
	char id_str[ID_MAXLEN];
	int delay;
	float gain;
} EftObj;

typedef struct {
	sLink *link;
} TaskVar;


static sTaskBody *taskBody;
static TaskVar *taskVar;


static int mainProc(sTaskBody *body, int msg, int lParam, int rParam)
{
	int res = 0;
	TaskVar *var = (TaskVar *)TaskGetVar(body, sizeof(TaskVar), MEM_APP);
	switch(msg)
	{
	case MSG_CREATE:
		{
			taskBody = body;
			taskVar = var;
			var->link = ObjLinkCreate(sizeof(EftObj), EFFECT_MAX_NUM, MEM_APP, FALSE);
		}
		break;

	case MSG_STEP:
		{
			void *p = ObjLinkGetTop(var->link);
			while(p)
			{
				void *next = ObjLinkGetNext(p);
				EftObj *obj = (EftObj *)p;
				if(!(obj->delay -= 1))
				{
					SndPlay(obj->id_str, obj->gain);
					ObjLinkDel(var->link, p);
				}
				p = next;
			}
		}
		break;

	case MSG_KILL:
		{
			taskBody = 0;
			taskVar = 0;
			ObjLinkDelAll(var->link);
		}
		break;


	case MSG_GAME_SNDEFFECT_STOP:
		{
			ObjLinkDelAll(var->link);
			BOOL audio_stop = (BOOL)lParam;
			if(audio_stop) SndStopAll();
		}
		break;
	}

	return res;
}

void SndEffectStart(void)
{
	TaskCreate("SndEft", TASK_PRI_SYS, mainProc, 0, 0);	
}

void SndEffectReq(char *id_str, int delay, float gain)
{
	if(taskVar)
	{
		EftObj *obj = (EftObj *)ObjLinkNew(taskVar->link);
		ASSERT(obj);
		STRCPY16(obj->id_str, id_str);
		obj->delay = delay + 1;
		obj->gain = gain;
	}
}

void SndEffectStopAll(BOOL audio_stop)
{
	if(taskBody)
	{
		TaskPostMsg(taskBody, MSG_GAME_SNDEFFECT_STOP, audio_stop, 0);
	}
}
