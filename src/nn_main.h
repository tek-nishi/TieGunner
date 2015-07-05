//
// アプリケーションメイン処理
//

//==============================================================
#ifndef NN_MAIN_H
#define NN_MAIN_H
//==============================================================

#include "co_common.h"
#include "co_param.h"
#include "co_misc.h"
#include "co_obj.h"

#ifdef __cplusplus
extern              "C"
{
#endif

enum enumOBJ_TYPE {
	OBJ_PLAYER		= (1 << 0),
	OBJ_ENEMY		= (1 << 1),
	OBJ_BOSS		= (1 << 2),
	OBJ_WEAPON		= (1 << 3),
	OBJ_BOSS_WEAPON = (1 << 4),
	OBJ_BOSS_TAIL	= (1 << 5),
	OBJ_SHOT		= (1 << 6),
	OBJ_CAMERA		= (1 << 7),
	OBJ_BG			= (1 << 8),
};

enum enumMSG_GAME {
	MSG_GAME_CAMERA = MSG_GAME,

	MSG_GAME_PLAYER_LVUP,

	MSG_GAME_ENEMY_SETUP,
	MSG_GAME_ENEMY_SET_TARGET,
	MSG_GAME_ENEMY_SET_FORMATION,
	MSG_GAME_ENEMY_IS_SHOT,
	MSG_GAME_ENEMY_IS_WRAP,

	MSG_GAME_WEAPON_FIXED,
	MSG_GAME_WEAPON_FREE,
	MSG_GAME_WEAPON_FREE_ALL,
	MSG_GAME_WEAPON_UPDATE,
	MSG_GAME_WEAPON_IS_FIX,
	MSG_GAME_WEAPON_CHECK_LEVEL,
	MSG_GAME_WEAPON_FIX,
	MSG_GAME_WEAPON_TYPE,
	MSG_GAME_WEAPON_LEVEL,

	MSG_GAME_BOSS_CHILD_UPDATE,
	MSG_GAME_BOSS_CHILD_KILL,
	MSG_GAME_BOSS_WEAPON_DEAD,
	MSG_GAME_BOSS_OBJKILL,
	
	MSG_GAME_SHOT_SETUP,

	MSG_GAME_TOUCH,
	MSG_GAME_DAMAGE,

	MSG_GAME_ENEMY_DESTROY,
	MSG_GAME_BOSS_DESTROY,
	MSG_GAME_PLAYER_DEAD,
	MSG_GAME_PLAYER_WEAPON_GET,

	
	MSG_GAME_OPENING,
	MSG_GAME_OPENING_FIN,

	MSG_GAME_TITLE,
	MSG_GAME_TITLE_SKIP_EFFECT,
	MSG_GAME_TITLE_LOGOEFT_FIN,
	MSG_GAME_TITLE_FIN,

	MSG_GAME_RANKING,
	MSG_GAME_RANKING_FIN,
	
	MSG_GAME_GAMESTART,
	MSG_GAME_GAMESTART_FIN,

	MSG_GAME_DEMOPLAY,
	MSG_GAME_DEMOPLAY_FIN,

	MSG_GAME_START,

	MSG_GAME_BOSS_ENTRY,
	MSG_GAME_BOSS_CLEAR,
	MSG_GAME_BOSS_CLEAR_FIN,

	MSG_GAME_EFFECT_CREATE,
	MSG_GAME_EFFECT_DELETE,

	MSG_GAME_SNDEFFECT_STOP,
};


enum enumGAME_PRIO {
	PRIO_DEBUG_PRINT = -5,

	PRIO_COCKPIT = -4,

//	PRIO_STAR_FRONT = -3,

	PRIO_EFFECT		 = -2,
	PRIO_SHOT		 = -1,
	PRIO_PLAYER		 = 0,							/* Player */
	PRIO_ENEMY		 = 2,							/* Enemy */
	PRIO_WEAPON		 = 4,							/* Weapon */
	PRIO_BOSS_WEAPON = 5,							/* Boss砲台 */
	PRIO_BOSS		 = 6,							/* Boss */
	PRIO_BOMB		 = 8,							/* BOMB破裂中 */

	PRIO_STAR_FRONT = 10,
	PRIO_STAR_BACK	= 11,
};


#ifdef DEBUG
enum enumGAMEDEBUG_FLAG {
	DEBUG_DISP_INFO		   = DBG_FLAG_01,
	DEBUG_PLAYER_INF_POWER = DBG_FLAG_02,
	DEBUG_DISP_SLOW		   = DBG_FLAG_03,
	DEBUG_GAME_NOENTRY	   = DBG_FLAG_04,
};

#define GAME_DEBUG_SETTINGS  DBG_FLAG_NONE
/* #define GAME_DEBUG_SETTINGS  (DEBUG_PLAYER_INF_POWER | DEBUG_GAME_NOENTRY) */
#endif

#define SETTINGS_FILE  "settings.sav"

extern void MainExec(void);
extern BOOL MainIsPause(void);
extern BOOL MainIsDemoPlay(void);
extern sParam *MainGetSettings(void);
extern sParam *MainGetParam(void);


#ifdef DEBUG
static BOOL isDispDebugInfo(void)
{
	return g.debug_flag & DEBUG_DISP_INFO;
}

static BOOL isPlayerPowerInf(void)
{
	return g.debug_flag & DEBUG_PLAYER_INF_POWER;
}
#endif

#ifdef __cplusplus
}
#endif

//==============================================================
#endif
//==============================================================
