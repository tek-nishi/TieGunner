//
// アプリケーションメイン処理
//

#include "nn_main.h"
#include "co_task.h"
#include "co_font.h"
#include "co_memory.h"
#include "co_input.h"
#include "co_graph.h"
#include "co_obj.h"
#include "co_random.h"
#include "co_stack.h"
#include "co_misc.h"
#include "co_sound.h"
#include "co_strings.h"
#include "nn_player.h"
#include "nn_enemy.h"
#include "nn_weapon.h"
#include "nn_camera.h"
#include "nn_bg.h"
#include "nn_gamemain.h"
#include "nn_title.h"
#include "nn_gamestart.h"
#include "nn_gameover.h"
#include "nn_ranking.h"
#include "nn_opening.h"
#include "nn_demoplay.h"
#include "nn_bossentry.h"
#include "nn_bossclear.h"
#include "nn_effect.h"
#include "nn_sndeffect.h"
#include "nn_gamemisc.h"


#define GAME_INPUT_RECORD_TIME  (3 * 60 * FRAME_RATE) /* とりあえず３分記録 */

#define DEBUG_INFO_DISPTIME  120
#define DEBUG_INFO_STRLEN    256

#define BOSS_NUM_MAX   (99 - 1)


#ifdef DEBUG
#define MSG_GAME_FIRST_MESSAGE  MSG_GAME_OPENING
//#define MSG_GAME_FIRST_MESSAGE  MSG_GAME_TITLE
//#define MSG_GAME_FIRST_MESSAGE  MSG_GAME_RANKING
//#define MSG_GAME_FIRST_MESSAGE  MSG_GAME_START
#define WRITE_RECORD_DATA
#else
#define MSG_GAME_FIRST_MESSAGE  MSG_GAME_OPENING
#endif


enum enumMODE {
	MODE_TITLE,
	MODE_RANKING,
	MODE_GAME,
};


typedef struct {
	FVector2 player_pos;
	FVector2 player_vct;
	float player_dir;

	FVector2 camera_pos;
	FVector2 camera_vct;

	u_int random_seed;
	u_char *rec_ptr;
} ReplayInfo;


typedef struct {
	sParam *param;
	sParam *gameParam;
	sParam *settings;
	sOBJ *player;
	BOOL first_proc;

	BOOL gameplay;
	int score;
	int weapon_num;
	int boss_num;
	int top_score;
	int top_boss;

	BOOL lvup;
	int lvup_type;
	int lvup_time;
	int lvup_time_max;
	sRGBA lvup_col;

	BOOL title_sound;
	int title_count;

	BOOL setuped;
	BOOL return_title;

	sTexture *texture;
	Sprite pause;

	BOOL can_gamepause;
	BOOL gamepause;

	BOOL replay;
	BOOL demoplay;
	int demoplay_count;
	int demo_loop;

	ReplayInfo demoPlayRec;
	ReplayInfo lastPlayRec;
} MainVar;

typedef struct {
#ifdef DEBUG
	char debug_info_str[DEBUG_INFO_STRLEN];
	int debug_info_counter;
#endif
} RootVar;


MainVar *mainVar = 0;


static void initObjects(sParam *param)
{
	BOOL flag;
	int index = 1;
	do
	{
		char id_str[ID_MAXLEN];

		sprintf(id_str, "gameobj%d", index);
		flag = ParamIsExists(param, id_str);
		if(flag)	ObjSetup(ParamGetStr(param, id_str));
		index += 1;
	}
	while(flag);
}

static void setupObjects(MainVar *var)
{
	u_int type = OBJ_TYPE_ALL & ~(OBJ_CAMERA | OBJ_BG);
	if(var->setuped) type &= ~OBJ_PLAYER;
	ObjDeleteAll(type);
	EffectDeleteAll();

	if(!var->setuped)
	{
		PRINTF("SetUp Objects.\n");
		var->player = PlayerCreate(var->param);
		InputSetAppExec(INP_CH0, TRUE);
		InputSetAppBtnExec(INP_CH0, TRUE);
		var->setuped = TRUE;
	}
}

static u_int setupReplay(ReplayInfo *info)
{
	u_int random_seed = info->random_seed;

	CameraSetPos(&info->camera_pos);
	CameraSetVct(&info->camera_vct);
	PlayerSetPos(&info->player_pos);
	PlayerSetVct(&info->player_vct);
	PlayerSetDir(info->player_dir);
	BgResetTail();
				
	InputSetRecData(info->rec_ptr);

	return random_seed;
}

static void readReplayInfo(ReplayInfo *info, sFILE *fp)
{
	u_int value;

	FsRead(fp, &value, sizeof(u_int));
	*(int *)(&info->player_pos.x) = ntohl(value);
	FsRead(fp, &value, sizeof(u_int));
	*(int *)(&info->player_pos.y) = ntohl(value);

	FsRead(fp, &value, sizeof(u_int));
	*(int *)(&info->player_vct.x) = ntohl(value);
	FsRead(fp, &value, sizeof(u_int));
	*(int *)(&info->player_vct.y) = ntohl(value);

	FsRead(fp, &value, sizeof(u_int));
	*(int *)(&info->player_dir) = ntohl(value);
	
	FsRead(fp, &value, sizeof(u_int));
	*(int *)(&info->camera_pos.x) = ntohl(value);
	FsRead(fp, &value, sizeof(u_int));
	*(int *)(&info->camera_pos.y) = ntohl(value);

	FsRead(fp, &value, sizeof(u_int));
	*(int *)(&info->camera_vct.x) = ntohl(value);
	FsRead(fp, &value, sizeof(u_int));
	*(int *)(&info->camera_vct.y) = ntohl(value);

	FsRead(fp, &value, sizeof(u_int));
	info->random_seed = ntohl(value);
}

static void writeReplayInfo(ReplayInfo *info, sFILE *fp)
{
	u_int value;

	value = htonl(*(u_int *)(&info->player_pos.x));
	FsWrite(fp, &value, sizeof(u_int));
	value = htonl(*(u_int *)(&info->player_pos.y));
	FsWrite(fp, &value, sizeof(u_int));

	value = htonl(*(u_int *)(&info->player_vct.x));
	FsWrite(fp, &value, sizeof(u_int));
	value = htonl(*(u_int *)(&info->player_vct.y));
	FsWrite(fp, &value, sizeof(u_int));

	value = htonl(*(u_int *)(&info->player_dir));
	FsWrite(fp, &value, sizeof(u_int));

	value = htonl(*(u_int *)(&info->camera_pos.x));
	FsWrite(fp, &value, sizeof(u_int));
	value = htonl(*(u_int *)(&info->camera_pos.y));
	FsWrite(fp, &value, sizeof(u_int));

	value = htonl(*(u_int *)(&info->camera_vct.x));
	FsWrite(fp, &value, sizeof(u_int));
	value = htonl(*(u_int *)(&info->camera_vct.y));
	FsWrite(fp, &value, sizeof(u_int));
	
	value = htonl(info->random_seed);
	FsWrite(fp, &value, sizeof(u_int));
}


#ifdef WRITE_RECORD_DATA
static void writeReplay(MainVar *var)
{
	char file[FNAME_MAXLEN];
	sprintf(file, "input_%s.rec", StrMakeUniqueName());

	sFILE *fp;
	fp = FsCreate(file);
	writeReplayInfo(&var->lastPlayRec, fp);
	InputRecordWrite(fp);
	FsClose(fp);
	ParamSetStr(var->param, "replay", file);
}
#endif


static int mainProc(sTaskBody *body, int msg, int lParam, int rParam)
{
	int res = 0;
	MainVar *var = (MainVar *)TaskGetVar(body, sizeof(MainVar), MEM_APP);
	switch(msg)
	{
	case MSG_CREATE:
		{
			var->param = ParamRead(PATH_DATA"/game.param");
			initObjects(var->param);
			var->title_sound = TRUE;
			FontSetup(FONT_APP_1, PATH_DATA"/gamefont.png", 11, 24);

			var->gameParam = ParamRead(PATH_DATA"/gamemain.param");
			var->settings = ReadGameSettings();
			float gain = ParamGetReal(var->settings, "gain");
			SndSetMasterGain(gain);

			var->texture = ParamGetTex(var->param, "texture");
			SpriteSetup(&var->pause, "pause", var->param);

			{
				RankingInfo ranking;
				RankingTopScore(&ranking);
				var->top_score = ranking.score;
				var->top_boss = ranking.boss;
			}

			CameraCreate();
			CameraSetDecay(ParamGetReal(var->param, "camera_decay"), 0);
			BgCreate();
			EffectStart();
			SndEffectStart();
			InputSetKeyRepeat(FALSE);

			char *path = ParamGetStr(var->param, "demoplay");
			char file[FNAME_MAXLEN];
			sprintf(file, PATH_DEVELOP"/%s", path);
			sFILE *fp = FsOpen(file);
			if(fp)
			{
//				FsRead(fp, &var->demoPlayRec, sizeof(ReplayInfo));
				readReplayInfo(&var->demoPlayRec, fp);
				InputRecordRead(fp);
				FsClose(fp);
				var->demoPlayRec.rec_ptr = InputCreateRecData(MEM_APP);
			}
			
			mainVar = var;

#ifdef DEBUG
			g.debug_flag |= GAME_DEBUG_SETTINGS;
#endif
		}
		break;
		
	case MSG_KILL:
		{
			ObjDeleteParamAll();
			ParamDestroy(var->param);
			ParamDestroy(var->gameParam);
			ParamDestroy(var->settings);

			FreeWork(var->demoPlayRec.rec_ptr);
			FreeWork(var->lastPlayRec.rec_ptr);

			mainVar = 0;
		}
		break;


	case MSG_STEP:
		{
			if(!var->first_proc)
			{
				/* FIXME:MSG_CREATE内でメッセージを投げられない為の苦肉の策 */
				TaskPostMsgAll(TASK_PRI_NONE, MSG_GAME_FIRST_MESSAGE, 0, 0);
				var->first_proc = TRUE;
			}

			if(var->gameplay && !var->demoplay)
			{
				var->weapon_num = PlayerGetWeaponNum(); /* 最大数はnn_player側で制限済み */
			}
			
			if(var->lvup > 0 && !(var->lvup_time -= 1))
			{
				var->lvup = FALSE;
			}
			if(var->title_count > 0)
			{
				var->title_sound = !(var->title_count -= 1);
			}
		}
		break;

	case MSG_DRAW:
		{
			FontPrintF(8, 8, PRIO_COCKPIT, "$F1SCORE %7d  W:%-2d $C8B$C7-%-2d", var->score, var->weapon_num, var->boss_num);
			FontPrintF(310, 8, PRIO_COCKPIT, "$F1$C2TOP $C7%7d $C8B$C7-%-2d", var->top_score, var->top_boss);
			if(var->lvup)
			{
				sGRPOBJ *grp;
				grp = GRPOBJ_QUAD(PRIO_COCKPIT);
				GrpSetPos(grp, 12.0f, 28.0f);
				float size = 175.0f * (float)var->lvup_time / (float)var->lvup_time_max;
				GrpSetSize(grp, size, 12.0f);
				GrpSetRGBA(grp, var->lvup_col.red, var->lvup_col.green, var->lvup_col.blue, var->lvup_col.alpha);
			}
		}
		break;

		
	case MSG_GAME_OPENING:
		{
			setupObjects(var);
			OpeningStart();
		}
		break;

	case MSG_GAME_OPENING_FIN:
		{
			TaskPostMsgAll(TASK_PRI_NONE, MSG_GAME_TITLE, TRUE, 0);
		}
		break;

		
	case MSG_GAME_TITLE:
		{
#ifdef DEBUG
			MemDisp();
#endif
			setupObjects(var);
			if(var->return_title)
			{
				/* GAMEOVER後は、プレイヤーとカメラの位置を合わせる */
				var->return_title = FALSE;
				sOBJ *player = PlayerGetObj();
				ASSERT(player);
				CameraSetPos(ObjGetPos(player));
				BgResetTail();
			}
			
			TitleStart(var->title_sound, lParam);
			if(var->title_sound)
			{
				var->title_sound = FALSE;
				var->title_count = 13 * FRAME_RATE;
			}
		}
		break;

	case MSG_GAME_TITLE_FIN:
		{
			int msg;

			var->demo_loop += 1;
			if(var->demo_loop < ParamGetReal(var->param, "demo_loop"))
			{
				InputSetAppBtnExec(INP_CH0, TRUE);
				msg = (var->demo_loop & 0x1) ? MSG_GAME_DEMOPLAY : MSG_GAME_RANKING;
			}
			else
			{
				msg = MSG_GAME_OPENING;
				var->demo_loop = 0;
			}
			TaskPostMsgAll(TASK_PRI_NONE, msg, 0, 0);
		}
		break;

		
	case MSG_GAME_RANKING:
		{
			RankingStart(lParam, var->score, var->boss_num, var->weapon_num);
		}
		break;

	case MSG_GAME_RANKING_FIN:
		{
			if(var->score > var->top_score) var->top_score = var->score;
			TaskPostMsgAll(TASK_PRI_NONE, MSG_GAME_TITLE, 0, 0);
		}
		break;
		

	case MSG_GAME_DEMOPLAY:
		{
			var->setuped = FALSE;
			var->demoplay = TRUE;
			setupObjects(var);
			DemoplayStart(var->param);
		}
		break;

	case MSG_GAME_DEMOPLAY_FIN:
		{
			TaskDeleteAllReq(TASK_PRI_02);
			InputPlayBackStop();

			var->demoplay = FALSE;
			var->lvup = FALSE;
			var->gameplay = FALSE;
			var->return_title = TRUE;
			var->can_gamepause = FALSE;
			var->setuped = FALSE;
			TaskPostMsgAll(TASK_PRI_NONE, MSG_GAME_TITLE, 0, 0);
		}
		break;
		

	case MSG_GAME_GAMESTART:
		{
			TaskDeleteAllReq(TASK_PRI_02);
			SndEffectStopAll(FALSE);
//			SndStop("bgm");
			var->score = 0;
			var->weapon_num = 0;
			var->boss_num = 0;
			GameStartStart();
		}
		break;

	case MSG_GAME_GAMESTART_FIN:
		{
			var->can_gamepause = TRUE;
		}
		break;

		
	case MSG_GAME_START:
		{
			setupObjects(var);
			CameraSetDecay(ParamGetReal(var->param, "camera_decay"), 0);
			var->replay = lParam;

			u_int random_seed;
			if(lParam)
			{
				ReplayInfo *info = &var->demoPlayRec;
				if(var->lastPlayRec.rec_ptr && (var->demoplay_count & 1))
				{
					info = &var->lastPlayRec;
				}
				random_seed = setupReplay(info);
				InputPlayBackStart();

				var->demoplay_count += 1;
			}
			else
			{
				random_seed = g.time;
				{
					var->lastPlayRec.random_seed = random_seed;
					var->lastPlayRec.camera_pos = *CameraGetPos();
					var->lastPlayRec.camera_vct = *CameraGetVct();
					var->lastPlayRec.player_pos = *PlayerGetPos();
					var->lastPlayRec.player_vct = *PlayerGetVct();
					var->lastPlayRec.player_dir = PlayerGetDir();

					InputRecordStart(GAME_INPUT_RECORD_TIME);
				}
			}
			init_genrand(RND_CH1, random_seed);
			var->gameplay = TRUE;
			GameMainStart(var->gameParam);
		}
		break;

		
	case MSG_GAME_BOSS_ENTRY:
		{
			BossEntryStart();
		}
		break;

	case MSG_GAME_BOSS_CLEAR:
		{
			BossClearStart();
		}
		break;

	case MSG_GAME_BOSS_CLEAR_FIN:
		{
		}
		break;

	case MSG_GAME_ENEMY_DESTROY:
	case MSG_GAME_BOSS_WEAPON_DEAD:
	case MSG_GAME_BOSS_DESTROY:
		if(var->gameplay && !var->demoplay)
		{
			int score = lParam;
		    var->score += score;
			if((msg == MSG_GAME_BOSS_DESTROY) && (var->boss_num < BOSS_NUM_MAX))
			{
				var->boss_num += 1;
			}
		}
		break;

	case MSG_GAME_PLAYER_DEAD:
		{
			SndEffectStopAll(FALSE);
			SndStop("bgm");
			SndStop("jingle");

			var->lvup = FALSE;
			var->gameplay = FALSE;
			var->return_title = TRUE;
			var->can_gamepause = FALSE;
			var->setuped = FALSE;

			if(InputRecordIsExec())
			{
				InputRecordStop();
				FreeWork(var->lastPlayRec.rec_ptr);
				var->lastPlayRec.rec_ptr = InputCreateRecData(MEM_APP);
			}

			if(!var->replay)
			{
				GameOverStart();
			}
		}
		break;

	case MSG_GAME_PLAYER_LVUP:
		{
			var->lvup = TRUE;
			var->lvup_type = lParam;
			var->lvup_time_max = var->lvup_time = rParam;

			sRGBA col_tbl[] = {
				{ 1.0f, 0.0f, 0.0f, 1.0f },
				{ 1.0f, 1.0f, 0.0f, 1.0f },
				{ 0.0f, 0.0f, 1.0f, 1.0f },
				{ 1.0f, 0.0f, 0.8f, 1.0f },
			};
			var->lvup_col = col_tbl[var->lvup_type];

//			PRINTF("MSG_GAME_PLAYER_LVUP:%d %d\n", lParam, rParam);
		}
		break;
	}

	return res;
}


#ifdef DEBUG
static void dispDebugInfo(RootVar *var, char *str)
{
	strcpy(var->debug_info_str, str);
	var->debug_info_counter = DEBUG_INFO_DISPTIME;
}
#endif


static int rootProc(sTaskBody *body, int msg, int lParam, int rParam)
{
	int res = 0;

	RootVar *var = (RootVar *)TaskGetVar(body, sizeof(RootVar), MEM_APP);
	switch(msg)
	{
	case MSG_CREATE:
		{
			TaskSetNoPause(body, TRUE);
#ifdef DEBUG
			var->debug_info_counter = 0;
#endif
		}
		break;
		
	case MSG_STEP:
		{
			char input_char = InputGetKey();
			if(input_char == ASCII_ESC)
			{
#ifndef DEBUG
				if(mainVar->can_gamepause)
#endif
				{
					mainVar->gamepause = !mainVar->gamepause;
					g.stop = mainVar->gamepause;
					if(mainVar->gamepause) SndPlay("start_touch", 1.0f);
#ifdef DEBUG
					dispDebugInfo(var, "PAUSE");
#endif
				}
			}
			else
			if(InputGetKeyPush(0x11) || InputGetKeyPush(0x17))
			{
				/* Ctrl-Q or Ctrl-W */
				g.app_exit = TRUE;
			}
			else
			if(InputGetKeyPush(INP_KEY_F5))
			{
				g.softreset = TRUE;
			}
			else
			if(InputGetKeyPush(0x12))
			{
				/* Ctrl-R */
				g.window_reset = TRUE;
//				glutReshapeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);
			}
#ifdef DEBUG
			else
			if(input_char == '1')
			{
				g.debug_flag ^= DEBUG_DISP_INFO;

				char str[DEBUG_INFO_STRLEN];
				sprintf(str, "DEBUG DISP INFO:%s", g.debug_flag & DEBUG_DISP_INFO ? "ON" : "OFF");
				dispDebugInfo(var, str);
			}
			else
			if(input_char == '2')
			{
				g.debug_flag ^= DEBUG_PLAYER_INF_POWER;

				char str[DEBUG_INFO_STRLEN];
				sprintf(str, "PLAYER POWER INFINITY:%s", g.debug_flag & DEBUG_PLAYER_INF_POWER ? "ON" : "OFF");
				dispDebugInfo(var, str);
			}
			else
			if(input_char == '3')
			{
				g.debug_flag ^= DEBUG_DISP_SLOW;
				g.slow = g.debug_flag & DEBUG_DISP_SLOW ? 4 : 0;
				g.slow_intvl = g.slow;
				
				char str[DEBUG_INFO_STRLEN];
				sprintf(str, "DEBUG_DISP_SLOW:%s", g.debug_flag & DEBUG_DISP_SLOW ? "ON" : "OFF");
				dispDebugInfo(var, str);
			}
			else
			if(input_char == '4')
			{
				g.debug_flag ^= DEBUG_GAME_NOENTRY;
				
				char str[DEBUG_INFO_STRLEN];
				sprintf(str, "DEBUG_GAME_NOENTRY:%s", g.debug_flag & DEBUG_GAME_NOENTRY ? "ON" : "OFF");
				dispDebugInfo(var, str);
			}
#ifdef WRITE_RECORD_DATA
			else
			if(input_char == 'r')
			{
				if(!InputRecordIsExec() && mainVar->lastPlayRec.rec_ptr)
				{
					writeReplay(mainVar);
					dispDebugInfo(var, "Write Replay");
				}
			}
#endif
			else
			if(input_char == '0')
			{
				sOBJ *obj = PlayerGetObj();
				if(obj)
				{
					FVector2 *pos = ObjGetPos(obj);

					StkMakeFrame();
					StkPushP(obj);
					StkPushP(pos);
					StkPushP((void *)&FVec2Zero);
					StkPushF(0.0f);
					StkPushI(PowerConvertInterValue(1));
					StkPushI(0);
					ObjPostMsg(obj, MSG_GAME_DAMAGE, TRUE, 0);
					StkDelFrame();
				}
			}
#if 1
			else
			if(input_char == 'c')
			{
				BossClearStart();
			}
#endif
			
#endif
		}
		break;
			
	case MSG_DRAW:
		{
			if(mainVar->gameplay && mainVar->gamepause && !mainVar->replay)
			{
				SpriteDraw(&mainVar->pause, mainVar->texture);
			}
#ifdef DEBUG
			if(var->debug_info_counter > 0)
			{
				var->debug_info_counter -= 1;
				FontPrint(0, 0, PRIO_DEBUG_PRINT, var->debug_info_str);
			}
#if 0
			{
				int x = InputGetMouseX();
				int y = InputGetMouseY();
				sGRPOBJ *grp = GRPOBJ_POINT(0);
				GrpSetDrawSize(grp, 4);
				GrpSetPos(grp, x, y);
			}
#endif

#endif
		}
		break;
	}
	
	return res;
}


void MainExec(void)
{
	TaskCreate("Main", TASK_PRI_01, mainProc, 0, 0);	
	TaskCreate("Root", TASK_PRI_SYS, rootProc, 0, 0);	
}

BOOL MainIsPause(void)
{
	return mainVar ? mainVar->gamepause : FALSE;
}

BOOL MainIsDemoPlay(void)
{
	return mainVar ? mainVar->demoplay : FALSE;
}

sParam *MainGetSettings(void)
{
	return mainVar ? mainVar->settings : 0;
}

sParam *MainGetParam(void)
{
	return mainVar ? mainVar->param : 0;
}
