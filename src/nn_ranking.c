//
// ƒ‰ƒ“ƒLƒ“ƒO‰æ–Ê
//

#include "nn_ranking.h"
#include "co_task.h"
#include "co_memory.h"
#include "co_font.h"
#include "co_param.h"
#include "co_input.h"
#include "co_random.h"
#include "co_stack.h"
#include "nn_main.h"
#include "nn_sndeffect.h"
#include "nn_gamemisc.h"


#define RANKING_MAX  10


typedef struct {
	int score;
	int boss;
	int weapon;
	char name[4];
} RankingScore;

typedef struct {
	sParam *param;
	sParam *ranking;
	int abort_count;
	int fade_in;
	float alpha;
	int input_delay;

	BOOL playend;
	BOOL rank_in;
	int rank_in_index;

	int score;
	int boss;
	int weapon;

	int cursor;
	RankingScore rankingScore[RANKING_MAX];
} RankingVar;


static void recordRanking(RankingVar *var)
{
	RankingScore *rankingScore = var->rankingScore;
	for(int i = 0; i < RANKING_MAX; i += 1)
	{
		char id_str[ID_MAXLEN];

		sprintf(id_str, "%d.name", i + 1);
		ParamSetStr(var->ranking, id_str, rankingScore->name);

		sprintf(id_str, "%d.score", i + 1);
		ParamSetReal(var->ranking, id_str, rankingScore->score);

		sprintf(id_str, "%d.boss", i + 1);
		ParamSetReal(var->ranking, id_str, rankingScore->boss);

		sprintf(id_str, "%d.weapon", i + 1);
		ParamSetReal(var->ranking, id_str, rankingScore->weapon);

		rankingScore += 1;
	}
	WriteGameSettings(var->ranking);
}

static int mainProc(sTaskBody *body, int msg, int lParam, int rParam)
{
	int res = 0;
	RankingVar *var = (RankingVar *)TaskGetVar(body, sizeof(RankingVar), MEM_APP);
	switch(msg)
	{
	case MSG_CREATE:
		{
			var->param = ParamRead(PATH_DATA"/ranking.param");
			var->ranking = ReadGameSettings(); 
			RankingInfo *info = (RankingInfo *)StkRefFrameP(0);
			var->score = info->score;
			var->boss = info->boss;
			var->weapon = info->weapon;

			var->playend = lParam;
			FVector2 *value = ParamGetFVec2(var->param, "ranking_count");
			var->abort_count = ((lParam) ? value->y : value->x) * FRAME_RATE;
			var->fade_in = ParamGetReal(var->param, "fade_in") * FRAME_RATE;
			var->input_delay = ParamGetReal(var->param, "input_delay") * FRAME_RATE;

			RankingScore *rankingScore = var->rankingScore;
			for(int i = 0; i < RANKING_MAX; i++)
			{
				char id_str[ID_MAXLEN];
				sprintf(id_str, "%d.name", i + 1);
				char *name = ParamGetStr(var->ranking, id_str);
				strcpy(rankingScore->name, name);

				sprintf(id_str, "%d.score", i + 1);
				rankingScore->score = ParamGetReal(var->ranking, id_str);

				sprintf(id_str, "%d.boss", i + 1);
				rankingScore->boss = ParamGetReal(var->ranking, id_str);

				sprintf(id_str, "%d.weapon", i + 1);
				rankingScore->weapon = ParamGetReal(var->ranking, id_str);

				rankingScore += 1;
			}

			if(var->playend)
			{
				RankingScore *rankingScore = var->rankingScore;
				int index = RANKING_MAX;
				for(int i = 0; i < RANKING_MAX; i += 1)
				{
					if(var->score >= rankingScore->score)
					{
						index = i;
						break;
					}
					rankingScore += 1;
				}
				for(int i = RANKING_MAX - 1; i > index; i -= 1)
				{
					var->rankingScore[i] = var->rankingScore[i - 1];
				}
				if(index < RANKING_MAX)
				{
					var->rank_in = TRUE;
					var->rank_in_index = index;
					rankingScore->score = var->score;
					rankingScore->boss = var->boss;
					rankingScore->weapon = var->weapon;
					strcpy(rankingScore->name, "   ");
					SndEffectReq("rank_in", ParamGetReal(var->param, "se_delay") * FRAME_RATE, 1.0f);
				}
			}
		}
		break;
		
	case MSG_KILL:
		{
			ParamDestroy(var->ranking);
			ParamDestroy(var->param);
		}
		break;


	case MSG_STEP:
		{
			BOOL abort = InputGetBtnTD(MOUSE_LEFT | MOUSE_D_LEFT);
			BOOL inkey = FALSE;
			BOOL key_input = var->rank_in;

			if(var->fade_in > 0)
			{
				var->fade_in -= 1;
				float fade_in = ParamGetReal(var->param, "fade_in") * FRAME_RATE;
				var->alpha = (fade_in - (float)var->fade_in) / fade_in;
			}

			if(var->input_delay > 0)
			{
				var->input_delay -= 1;
				abort = FALSE;
				key_input = FALSE;
			}

			if(key_input)
			{
				char input = InputGetKey();
				if(input == '"') input = '\'';		/* ‹ê“÷‚Ìô */
				
				RankingScore *rankingScore = &var->rankingScore[var->rank_in_index];
				if(input == 0xd)
				{
					abort = TRUE;
				}
				else
				if((var->cursor > 0) && (input == ASCII_BS || input == ASCII_DEL))
				{
					var->cursor -= 1;
					rankingScore->name[var->cursor] = ' ';
					inkey = TRUE;
				}
				else
				if((var->cursor < 3) && (input >= 0x20) && (input <= 0x7e))
				{
					rankingScore->name[var->cursor] = input;
					var->cursor += 1;
					inkey = TRUE;
				}
			}
			else
			if(var->abort_count > 0)
			{
				var->abort_count -= 1;
			}
			if(inkey || abort) SndEffectReq("rank_key", 0, 1.0f);

			res = ((var->abort_count == 0) || abort);
			if(res)
			{
				if(var->rank_in)
				{
					recordRanking(var);
				}

				TaskPostMsgAll(TASK_PRI_NONE, MSG_GAME_RANKING_FIN, 0, 0);
			}
		}
		break;
			
	case MSG_DRAW:
		{
			FontPrintF(174, 115, PRIO_COCKPIT, "$A%f$$F1$C1R $C7A N K I N $C2G", var->alpha);

			RankingScore *rankingScore = var->rankingScore;
			for(int i = 0; i < RANKING_MAX; i += 1)
			{
				char *name = rankingScore->name;
				int score = rankingScore->score;
				int boss = rankingScore->boss;
				if(var->rank_in && (var->rank_in_index == i))
				{
					char id_str[ID_MAXLEN];
					sprintf(id_str, "%d.rank_str", i + 1);
					char *rank_str = ParamGetStr(var->param, id_str);

					int col = (g.time % 7) + 1;
					FontPrintF(115, 165 + 24 * i, PRIO_COCKPIT, "$A%f$$F1$C%d%s  %s  %7d  B-%-2d", var->alpha, col, rank_str, name, score, boss);
					char cur[5] = "    ";
					cur[var->cursor] = (var->input_delay == 0) ? '_' : ' ';
					FontPrintF(115, 167 + 24 * i, PRIO_COCKPIT, "$A%f$$F1$C%d      %s", var->alpha, col, cur);
				}
				else
				{
					char id_str[ID_MAXLEN];
					sprintf(id_str, "%d.rank_hdr", i + 1);
					char *rank_hdr = ParamGetStr(var->param, id_str);
					sprintf(id_str, "%d.rank_str", i + 1);
					char *rank_str = ParamGetStr(var->param, id_str);
					
					FontPrintF(115, 165 + 24 * i, PRIO_COCKPIT, "$A%f$$F1%s%s  $C7%s  %7d  $C8B$C7-%-2d", var->alpha, rank_hdr, rank_str, name, score, boss);
				}
				rankingScore += 1;
			}
		}
		break;
	}

	return res;
}


void RankingStart(BOOL playend, int score, int boss, int weapon)
{
	RankingInfo info;

	info.score = score;
	info.boss = boss;
	info.weapon = weapon;
	StkMakeFrame();
	StkPushP(&info);
	TaskCreate("Ranking", TASK_PRI_02, mainProc, playend, 0);	
	StkDelFrame();
}

void RankingTopScore(RankingInfo *res)
{
	sParam *ranking = ReadGameSettings();
	res->score = ParamGetReal(ranking, "1.score");
	res->boss = ParamGetReal(ranking, "1.boss");
	ParamDestroy(ranking);
}
