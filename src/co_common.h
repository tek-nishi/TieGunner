
//==============================================================
#ifndef CO_COMMON_H
#define CO_COMMON_H
//==============================================================

/*

	DEBUG			デバッグ機能を有効にしてコンパイル(自分で宣言する！)

	__APPLE__       OSXでコンパイル(自動的に宣言されている)
	_MSC_VER		VCでコンパイル(自動的に宣言されている)
	__GNUC__		GCCでコンパイル(自動的に宣言されている)

*/

#ifdef DEBUG
	#define CRTDBG_MAP_ALLOC					// メモリリークを追跡する
#endif


//------------------------------
// ANSIとか
//------------------------------
#include <ctype.h>
#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/stat.h>

#if defined (__GNUC__)
	#include <dirent.h>
#endif

#if defined (_MSC_VER) || defined (WIN32)
#include <windows.h>								/* 結局… ntohl()とhtonl()を使う為 */
#pragma comment (lib, "Ws2_32.lib")
#endif

//------------------------------
// 外部ライブラリ
//------------------------------
#if defined (__APPLE__)
#include <GLUT/glut.h>
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <GL/glut.h>
#include <AL/al.h>
#include <AL/alc.h>
#endif

#include <zlib.h>
#include <png.h>
#ifndef png_infopp_NULL 
#define png_infopp_NULL NULL
#endif

#include "co_types.h"
#include "co_message.h"


#ifdef __cplusplus
extern              "C"
{
#endif

#define PROJECT_NAME    "TieGunner"					// タイトルバーに表示されるプロダクト名
#define VERSION_NUMBER  "1.01"						// 同バージョン番号
#define BUILD_NUMBER    "142"						/* ビルド番号 */

#define WINDOW_WIDTH   512
#define WINDOW_HEIGHT  512

#define FRAME_RATE		60							/* １秒あたりのループ回数 */
#define UPDATE_INTERVAL (1000 / FRAME_RATE)			// 更新間隔(ms)

#define FNAME_MAXLEN   256							// パス名の最大長
#define ID_MAXLEN      16							// ID名の最大長

#define PATH_DEVELOP "devdata"						// 開発ルートパス
#define PATH_DATA	 PATH_DEVELOP					// 開発データ(グラフィック・サウンドなどの元データ)
#define PATH_BACKUP  PATH_DEVELOP"/backup"
#define IMAGE_FILE	 "files.dat"


//------------------------------
//	システムカラー定義(HTML 準拠)
//------------------------------
enum tagSYS_COLOR {
	COL_BLACK,									// 黒
	COL_GLAY,									// 灰色
	COL_SILVER,									// 明るい灰色
	COL_WHITE,									// 白

	COL_RED,									// 赤
	COL_YELLOW,									// 黄色
	COL_LIME,									// 緑
	COL_AQUA,									// 水色
	COL_BLUE,									// 青
	COL_FUCHSIA,								// 紫

	COL_MAROON,									// 暗い赤
	COL_OLIVE,									// 暗い黄色
	COL_GREEN,									// 暗い緑
	COL_TEAL,									// 暗い水色
	COL_NAVY,									// 暗い青
	COL_PURPLE,									// 暗い紫

	// ---------------------
	COL_MAX
};


//------------------------------
//	デバッグフラグ
//------------------------------
enum enumDEBUG_FLAG {
	DBG_FLAG_01 = (1 << 0),
	DBG_FLAG_02 = (1 << 1),
	DBG_FLAG_03 = (1 << 2),
	DBG_FLAG_04 = (1 << 3),
	DBG_FLAG_05 = (1 << 4),
	DBG_FLAG_06 = (1 << 5),
	DBG_FLAG_07 = (1 << 6),
	DBG_FLAG_08 = (1 << 7),
	DBG_FLAG_09 = (1 << 8),
	DBG_FLAG_10 = (1 << 9),
	DBG_FLAG_11 = (1 << 10),
	DBG_FLAG_12 = (1 << 11),
	DBG_FLAG_13 = (1 << 12),
	DBG_FLAG_14 = (1 << 13),
	DBG_FLAG_15 = (1 << 14),
	DBG_FLAG_16 = (1 << 15),
	DBG_FLAG_17 = (1 << 16),
	DBG_FLAG_18 = (1 << 17),
	DBG_FLAG_19 = (1 << 18),
	DBG_FLAG_20 = (1 << 19),
	DBG_FLAG_21 = (1 << 20),
	DBG_FLAG_22 = (1 << 21),
	DBG_FLAG_23 = (1 << 22),
	DBG_FLAG_24 = (1 << 23),

	// ---------------------
	DBG_FLAG_NONE = 0,
	DBG_FLAG_ALL  = (~0)
};


//------------------------------
// マクロで実装
//------------------------------
#ifdef DEBUG
	#define EXIT(val)	while(1)
#else
	#define EXIT(val)
#endif


//------------------------------------------
// グローバル変数
// ※プログラム起動/ソフトウェアリセット時に初期化
//------------------------------------------
typedef struct {
	int width, height;							// フレームバッファのサイズ
	IVector2 disp_size;							// 表示サイズ
	IVector2 disp_ofs;
	sRGBA bg_col;								// 背景色

	GLuint display_list;
	GLuint display_page;

	u_int time;									// ゲームが開始されてからの経過時間 ( 2^32フレ目で 0 に戻る )
	BOOL stop;									// TRUE: 全ての動き処理をストップ中
	int slow;									// ゲームの進行をスローにする間隔(0 = 100%)
	int slow_intvl;								// スローのフレーム間隔
	int step_loop;								// ステップ回数(1～)

	BOOL softreset;								// TRUE: ソフトリセット要求
	BOOL app_exit;								// TRUE: アプリケーション終了
	BOOL window_reset;							/* TRUE: サイズ初期化 */

	void *msgarg_stack;							// メッセージ用スタック(訳有って void 型)

#ifdef DEBUG
	u_int debug_flag;
#endif
} GLOBAL_COMMON;

extern GLOBAL_COMMON g;								/* main.cにて定義 */

#ifdef __cplusplus
}
#endif

//==============================================================
#endif
//==============================================================

