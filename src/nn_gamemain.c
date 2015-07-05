//
// ゲームメイン処理
//

#include "nn_gamemain.h"
#include "co_task.h"
#include "co_memory.h"
#include "co_stack.h"
#include "co_misc.h"
#include "co_random.h"
#include "co_memory.h"
#include "co_font.h"
#include "co_input.h"
#include "co_hash16.h"
#include "nn_main.h"
#include "nn_camera.h"
#include "nn_player.h"
#include "nn_enemy.h"
#include "nn_weapon.h"
#include "nn_boss.h"
#include "nn_sndeffect.h"
#include "nn_bossdir.h"


#define ENEMY_ENTRY_MAXNUM			 10
#define ENEMY_FORMATION_MAXNUM		 4
#define ENEMY_FORMATION_ENTRY_MAXNUM 8
#define WEAPON_ENTRY_MAXNUM			 10
#define EXTRA_ENTRY_MAXNUM			 5

#define ENEMY_MAX_LEVEL		(99 - 1)
#define BOSS_MAX_LEVEL      (99 - 1)
#define WEAPON_TOTAL_NUMMAX (99999 - 1)


typedef struct {
	int type;
	FVector2 ofs;
} Formation;

typedef struct {
	Formation form[ENEMY_FORMATION_MAXNUM];
	int form_num;
} FormationInfo;


typedef struct {
	int type;
	int rate;
	int formation;
} EnemyInfo;

typedef struct {
	int type;
	int level;
	int rand_max;
} WeaponEntry;

typedef struct {
	int level;
	int rand_max;
} ExtraEntry;

typedef struct {
	sParam *param;
	int level;
	int score;
	int destroy_num;
	BOOL ene_entry;

	int enemy_num;									/* 敵の出現数 */
	int interval;									/* 出現間隔 */
	int enemy_maxnum;								/* 最大登場数 */
	int enemy_maxnum_next;
	int ene_timeout;								/* 出現時間 */

	FormationInfo formation[ENEMY_FORMATION_ENTRY_MAXNUM];
	int formation_entrynum;
	
	EnemyInfo enemy_entry[ENEMY_ENTRY_MAXNUM];
	int enemy_entrynum;
	int ene_intarval;
	int boss_entry;
	int boss_entry_num;

	int enemy_entry_randommax;

	WeaponEntry weapon_entry[WEAPON_ENTRY_MAXNUM];
	int weapon_entry_randommax;
	int weapon_entrynum;
	ExtraEntry extra_entry[EXTRA_ENTRY_MAXNUM];
	int extra_entry_randommax;
	int extra_entrynum;
	
	int levelup_num;
	int next_level;

	int enemy_level;
	int total_weapon;
	int lvup_weapon_num;
	int next_weapon_num;

	int boss_level;

	char **jingle_tbl;
	int jingle_num;

	BOOL ene_shotactive;
} MainVar;


static sTaskBody *mainTask = 0;
static MainVar *mainVar = 0;


static void analyzeJingleSound(MainVar *var, sParam *param)
{
	char id_str[ID_MAXLEN];
	int i;
	for(i = 0; ; i += 1)
	{
		sprintf(id_str, "%d.jingle", i + 1);
		if(!ParamIsExists(param, id_str)) break;
	}

	var->jingle_num = i;
	var->jingle_tbl = (char **)appMalloc(sizeof(char **) * i, "jingle");
	ASSERT(var->jingle_tbl);
	for(i = 0; i < var->jingle_num; i += 1)
	{
		sprintf(id_str, "%d.jingle", i + 1);
		var->jingle_tbl[i] = ParamGetStr(param, id_str);
	}
}

static void luvpSound(MainVar *var, int level, int delay)
{
	SndEffectReq(var->jingle_tbl[level % var->jingle_num], delay, 0.6f);
}


static void analyzeEnemyEntryInfo(MainVar *var, sParam *param)
{
	char id_str[ID_MAXLEN];

	sprintf(id_str, "%d.enemy_num", var->level + 1);
	var->enemy_maxnum_next = ParamGetReal(param, id_str);

	sprintf(id_str, "%d.ene_interval", var->level + 1);
	var->ene_intarval = ParamGetReal(param, id_str) * FRAME_RATE;

	sprintf(id_str, "%d.ene_timeout", var->level + 1);
	var->ene_timeout = ParamIsExists(param, id_str) ? ParamGetReal(param, id_str) * FRAME_RATE : -1;
}

static void analyzeEnemyEntry(MainVar *var, sParam *param)
{
	int entry_randommax = 0;
	int i;
	for(i = 0; i < ENEMY_ENTRY_MAXNUM; i += 1)
	{
		char id_str[ID_MAXLEN];

		sprintf(id_str, "%d.enemy%d", var->level + 1, i + 1);
		if(!ParamIsExists(param, id_str)) break;

		EnemyInfo *info = &var->enemy_entry[i];
		if(ParamGetType(param, id_str) == PARAM_V2)
		{
			/* 単独出現 */
			FVector2 *value = ParamGetFVec2(param, id_str);
			info->type = value->x;
			info->rate = value->y + entry_randommax;
			info->formation = 0;

			entry_randommax += value->y;
		}
		else
		{
			/* 編隊出現 */
			FVector3 *value = ParamGetFVec3(param, id_str);
			info->type = value->x;
			info->formation = value->y;
			info->rate = value->z + entry_randommax;
			
			entry_randommax += value->z;
		}
	}
	var->enemy_entrynum = i;
	var->enemy_entry_randommax = entry_randommax;

	{
		char id_str[ID_MAXLEN];
		sprintf(id_str, "%d.boss", var->level + 1);
		if(ParamIsExists(param, id_str))
		{
			var->boss_entry = ParamGetReal(param, id_str);
			var->levelup_num = -1;					/* BOSSを撃破するまでLevelUpしない */
		}
	}
}

static void analyzeFormation(MainVar *var, sParam *param)
{
	int i;
	for(i = 0; i < ENEMY_FORMATION_ENTRY_MAXNUM; i += 1)
	{
		int h;
		for(h = 0; h < ENEMY_FORMATION_MAXNUM; h += 1)
		{
			char id_str[ID_MAXLEN];
			sprintf(id_str, "formation%d.%d", i + 1, h + 1);
			if(!ParamIsExists(param, id_str)) break;

			FVector3 *value = ParamGetFVec3(param, id_str);
			Formation *form = &var->formation[i].form[h];
			form->type = value->z;
			form->ofs.x = value->x;
			form->ofs.y = value->y;
		}
		var->formation[i].form_num = h;
		if(!h) break;
	}
	var->formation_entrynum = i;
}

static EnemyInfo *getEnemyEntry(MainVar *var)
{
	int random = RndICH(RND_CH1, var->enemy_entry_randommax);

	int i;
	for(i = 0; i < var->enemy_entrynum; i += 1)
	{
		if(random < var->enemy_entry[i].rate) break;
	}
	ASSERT(i < var->enemy_entrynum);

	return &var->enemy_entry[i];
}

static void analyzeWeaponEntry(MainVar *var, sParam *param)
{
	var->weapon_entry_randommax = 0;

	int i;
	for(i = 0; i < WEAPON_ENTRY_MAXNUM; i += 1)
	{
		char id_str[ID_MAXLEN];

		sprintf(id_str, "%d.weapon%d", var->level + 1, i + 1);
		if(!ParamIsExists(param, id_str)) break;
		
		FVector3 *value = ParamGetFVec3(param, id_str);
		var->weapon_entry[i].type = value->x;
		var->weapon_entry[i].level = value->y;
		var->weapon_entry[i].rand_max = value->z + var->weapon_entry_randommax;
		var->weapon_entry_randommax += value->z;
	}
	var->weapon_entrynum = i;
}

static void analyzeExtraEntry(MainVar *var, sParam *param)
{
	var->extra_entry_randommax = 0;

	int i;
	for(i = 0; i < EXTRA_ENTRY_MAXNUM; i += 1)
	{
		char id_str[ID_MAXLEN];

		sprintf(id_str, "%d.extra%d", var->level + 1, i + 1);
		if(!ParamIsExists(param, id_str)) break;

		FVector2 *value = ParamGetFVec2(param, id_str);
		var->extra_entry[i].level = value->x;
		var->extra_entry[i].rand_max = value->y + var->extra_entry_randommax;
		var->extra_entry_randommax += value->y;
	}
	var->extra_entrynum = i;
}


static sOBJ *weaponEntry(MainVar *var, sParam *param)
{
	sOBJ *res = 0;

	int random = RndICH(RND_CH1, var->weapon_entry_randommax);
	for(int i = 0; i < var->weapon_entrynum; i += 1)
	{
		if(random < var->weapon_entry[i].rand_max)
		{
			res = WeaponCreate(var->weapon_entry[i].type, var->weapon_entry[i].level, param);
			break;
		}
	}
	return res;
}

static sOBJ *extraWeaponEntry(MainVar *var, sParam *param)
{
	sOBJ *res = 0;

	int random = RndICH(RND_CH1, var->extra_entry_randommax);
	for(int i = 0; i < var->extra_entrynum; i += 1)
	{
		if(random < var->extra_entry[i].rand_max)
		{
			if(var->extra_entry[i].level >= 0)
			{
				res = WeaponCreate(WEAPON_KIND_EXTRA, var->extra_entry[i].level, param);
				/* levelがマイナスの場合は武器を生成しない */
			}
			break;
		}
	}
	return res;
}

static void setupEnemyWeapon(sOBJ *obj, MainVar *var, sParam *param)
{
	sOBJ *weapon;

	weapon= weaponEntry(var, param);
	ASSERT(weapon);
	WeaponFixToObj(weapon, obj, PRIO_ENEMY + 1);
	weapon = extraWeaponEntry(var, param);
	if(weapon) WeaponFixToObj(weapon, obj, PRIO_ENEMY + 1);
}

static void bossEntry(MainVar *var, sOBJ *player)
{
	float angle = RndmCH(RND_CH1) * PI;
	for(int i = 0; i < var->boss_entry; i += 1)
	{
		FVector2 pos;
		MathCalcVector(&pos, angle, ParamGetReal(var->param, "boss_dist"));
		FVector2 p_pos = *ObjGetPos(player);
		AddV2d(&pos, &pos, &p_pos);
		float angle_ofs = PI + ANG2RAD(ParamGetReal(var->param, "boss_angle")) * RndmCH(RND_CH1);
		angle = NormalAngle(angle + angle_ofs);
		sOBJ *obj = BossCreate("boss", &pos, angle, var->param);
		BossDirStart(obj);
		angle = NormalAngle(angle + PI * 2.0f / 3.0f);
	}
	var->boss_entry_num = var->boss_entry;

	TaskPostMsgAll(TASK_PRI_NONE, MSG_GAME_BOSS_ENTRY, 0, 0);
}

static int enemyEntry(MainVar *var, sOBJ *player)
{
	int entry_num = 1;
	
	float angle = RndmCH(RND_CH1) * PI;
	FVector2 pos;
	MathCalcVector(&pos, angle, ParamGetReal(var->param, "entry_dist"));
	FVector2 p_pos = *ObjGetPos(player);
	AddV2d(&pos, &pos, &p_pos);
	float angle_ofs = PI + ANG2RAD(ParamGetReal(var->param, "entry_angle")) * RndmCH(RND_CH1);
	angle = NormalAngle(angle + angle_ofs);

	char id_str[ID_MAXLEN];
	EnemyInfo *info = getEnemyEntry(var);
	sprintf(id_str, "enemy%02d", info->type);
	sOBJ *enemy = EnemyCreate(id_str, &pos, angle, var->ene_shotactive, var->param);
	EnemySetTarget(enemy, player);
	setupEnemyWeapon(enemy, var, var->param);

	if(info->formation > 0)
	{
		/* 編隊で出現 */
		sOBJ *target = enemy;
		FormationInfo *formation = &var->formation[info->formation - 1];
		Formation *form = formation->form;
		for(int i = 0; i < formation->form_num; i += 1)
		{
			FVector2 f_pos;
			SetV2d(&f_pos, form->ofs.x, form->ofs.y);
			MathRotateXY(&f_pos, angle);
			AddV2d(&f_pos, &pos, &f_pos);
						
			sprintf(id_str, "enemy%02d", form->type);
			enemy = EnemyCreate(id_str, &f_pos, angle, var->ene_shotactive, var->param);
			EnemySetFormation(enemy, target, &form->ofs);
			setupEnemyWeapon(enemy, var, var->param);

			form += 1;
		}
		entry_num += formation->form_num;
	}
	return entry_num;
}

static void enemyEntryProc(MainVar *var, sOBJ *player)
{
	if((var->interval == 0) && player && (var->enemy_num < var->enemy_maxnum))
	{
		if(var->boss_entry > 0)
		{
			bossEntry(var, player);
			var->boss_entry = 0;
		}
		else
		if(var->ene_timeout)
		{
			var->enemy_num += enemyEntry(var, player);
		}
		var->interval = var->ene_intarval;

		var->enemy_maxnum = var->enemy_maxnum_next;	/* レベルアップのタイミングが別なので、ここで数を変更する必要がある */
	}
	if(var->interval > 0) var->interval -= 1;
	if(var->ene_timeout > 0) var->ene_timeout -= 1;
}

static BOOL checkLevelup(MainVar *var)
{
	return var->destroy_num == var->levelup_num;
}

static void setupNewLevel(MainVar *var)
{
	var->level = var->next_level;

	char id_str[ID_MAXLEN];
	sprintf(id_str, "%d.lvup", var->level + 1);
	var->levelup_num += ParamIsExists(var->param, id_str) ? ParamGetReal(var->param, id_str) : -1;
	
	sprintf(id_str, "%d.next_lv", var->level + 1);
	var->next_level = ParamIsExists(var->param, id_str) ? ParamGetReal(var->param, id_str) - 1 : var->level + 1;
	PRINTF("next_level:%d\n", var->next_level);
				
	analyzeEnemyEntryInfo(var, var->param);
	analyzeEnemyEntry(var, var->param);
	analyzeWeaponEntry(var, var->param);
	analyzeExtraEntry(var, var->param);
}

static void soundNewLevel(MainVar *var, int delay)
{
	if(var->boss_entry > 0)
	{
		SndEffectReq("boss_bgm", 5 * FRAME_RATE, 0.6f);
	}
	else
	{
		luvpSound(var, var->level, delay);
	}
}

static void setupEnemyLevel(MainVar *var)
{
	if(var->enemy_level < ENEMY_MAX_LEVEL) var->enemy_level += 1;

	char id_str[ID_MAXLEN];
	sprintf(id_str, "%d.ene_level", var->enemy_level + 1);
	
	int lvup_num = ParamIsExists(var->param, id_str) ? ParamGetReal(var->param, id_str) : var->lvup_weapon_num;
	var->next_weapon_num += lvup_num;
	var->lvup_weapon_num = lvup_num;
	PRINTF("enemy_level:%d next_weapon_num:%d\n", var->enemy_level, var->next_weapon_num);
}


static int mainProc(sTaskBody *body, int msg, int lParam, int rParam)
{
	int res = 0;
	MainVar *var = (MainVar *)TaskGetVar(body, sizeof(MainVar), MEM_APP);
	switch(msg)
	{
	case MSG_CREATE:
		{
			mainTask = body;
			mainVar = var;
			var->param = (sParam *)StkRefFrameP(0);
			analyzeJingleSound(var, var->param);
			analyzeFormation(var, var->param);

			var->enemy_level = -1;
			setupEnemyLevel(var);
#ifdef DEBUG
			{
				int level = ParamGetReal(var->param, "start_ene_lv") - 1;
				for(int i = 0; i < level; i += 1)
				{
					var->total_weapon = var->next_weapon_num;
					setupEnemyLevel(var);
				}
			}
#endif

#ifdef DEBUG
			var->boss_level = ParamGetReal(var->param, "start_boss_lv") - 1;
#endif
				
			var->level = -1;
			setupNewLevel(var);
#ifdef DEBUG
			{
				int skip_level = ParamGetReal(var->param, "start_level") - 1;
				for(int i = 0; i < skip_level; i += 1)
				{
					var->boss_entry = 0;
					var->levelup_num = 0;
					setupNewLevel(var);
				}
			}
#endif
			soundNewLevel(var, 1.5f * FRAME_RATE);
			var->enemy_maxnum = var->enemy_maxnum_next; /* 苦肉の策 */
			var->ene_entry = TRUE;
		}
		break;
		
	case MSG_KILL:
		{
			Free(var->jingle_tbl);
			mainVar = 0;
			mainTask = 0;
		}
		break;


	case MSG_STEP:
		{
			sOBJ *player = PlayerGetObj();

#ifdef DEBUG
			if(!(g.debug_flag & DEBUG_GAME_NOENTRY))
#endif
			{
				if(var->ene_entry)
				{
					enemyEntryProc(var, player);
					/* 敵出現処理 */
				}
			}

#ifdef DEBUG
			/* 強制BOSS出現 */
			if(InputGetKey() == 'b')
			{
				var->boss_entry = 1;
				bossEntry(var, player);
				var->boss_entry = 0;
			}
#endif
		}
		break;
			
	case MSG_DRAW:
		{
#ifdef DEBUG
			FontPrintF(16, 512-16, 0, "P:%2d L:%2d/%2d/%2d D:%2d/%2d, E:%2d/%2d W:%d/%d", PlayerGetPower(), var->level, var->enemy_level, var->boss_level, var->destroy_num, var->levelup_num, var->enemy_num, var->enemy_maxnum, var->total_weapon, var->next_weapon_num);
#endif
		}
		break;


	case MSG_GAME_ENEMY_DESTROY:
		{
			int score = lParam;
			var->score += score;

			var->destroy_num += 1;
			var->enemy_num -= 1;
			if(checkLevelup(var))
			{
				PRINTF("Level Up!\n");

				setupNewLevel(var);
				soundNewLevel(var, 0.5f * FRAME_RATE);
			}
			var->ene_shotactive = TRUE;
		}
		break;
		
	case MSG_GAME_BOSS_WEAPON_DEAD:
		{
			int score = lParam;
			var->score += score;
		}
		break;
		
	case MSG_GAME_BOSS_DESTROY:
		{
			int score = lParam;
			var->score += score;

			if((var->boss_entry_num -= 1) == 0)
			{
				if(var->boss_level < BOSS_MAX_LEVEL) var->boss_level += 1;
				var->ene_entry = FALSE;
				TaskPostMsgAll(TASK_PRI_NONE, MSG_GAME_BOSS_CLEAR, 0, 0);
			}
		}
		break;

	case MSG_GAME_BOSS_CLEAR_FIN:
		{
			/* FIXME:演出待ちでのレベルアップ処理は微妙
			   ⇒ 演出待ち無しの時間差レベルアップ処理がベター */
			PRINTF("Level Up!\n");
			var->levelup_num = var->destroy_num;
			setupNewLevel(var);
			var->ene_entry = TRUE;
			soundNewLevel(var, 0.5 * FRAME_RATE);
		}
		break;

	case MSG_GAME_PLAYER_DEAD:
		{
			PRINTF("Player Dead\n");
			TaskDeleteReq(body);
		}
		break;

	case MSG_GAME_PLAYER_WEAPON_GET:
		if(mainVar->total_weapon < WEAPON_TOTAL_NUMMAX)
		{
			var->total_weapon += 1;
			if(var->total_weapon == var->next_weapon_num)
			{
				setupEnemyLevel(var);
			}
		}
		break;
	}

	return res;
}

void GameMainStart(sParam *param)
{
	StkMakeFrame();
	StkPushP(param);								// 0
	TaskCreate("GameMain", TASK_PRI_02, mainProc, 0, 0);
	StkDelFrame();
}

int GameMainGetEnemyLevel(void)
{
	return mainVar ? mainVar->enemy_level : 0;
}

int GameMainGetBossLevel(void)
{
	return mainVar ? mainVar->boss_level : 0;
}

void GameMainPlayerGetWeapon(void)
{
	GameMainPostMessage(MSG_GAME_PLAYER_WEAPON_GET, 0, 0);
}

int GameMainPostMessage(int msg, int lParam, int rParam)
{
	return mainTask ? TaskPostMsg(mainTask, msg, lParam, rParam) : 0;
}

sOBJ *GameMainWeaponEntry(void)
{
	return mainVar ? weaponEntry(mainVar, mainVar->param) : 0;
}
