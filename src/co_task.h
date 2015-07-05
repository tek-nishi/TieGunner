
/*

	システム側で必ず実行されるメッセージ

	MSG_CREATE				生成
	MSG_KILL				削除

	MSG_PREPROC				事前処理
	MSG_STEP				ステップ
	MSG_UPDATE				更新
	MSG_DRAW				描画


	タスクを生成する場合、即時に MSG_CREATE を発行しますが、TASK_CREATE_REQ
	が指定されている場合、MSG_PREPROC 直前に MSG_CREATE を発行します。
	スリープ関連処理も MSG_PREPROC 直前に処理されます。
	TASK_DELETE_REQ の処理は MSG_STEP 直後に行います。

*/

//==============================================================
#ifndef CO_TASK_H
#define CO_TASK_H
//==============================================================

#include "co_common.h"

#ifdef __cplusplus
extern              "C"
{
#endif

/********************************************/
/*             定数・マクロ宣言             */
/********************************************/
#define TASK_DELETE_REQ  (1 << 0)				// 削除予約
#define TASK_SLEEP_REQ   (1 << 1)				// スリープ予約
#define TASK_SLEEP       (1 << 2)				// スリープ中
#define TASK_CREATE_REQ  (1 << 3)				// 次のステップで MSG_CREATE を投げる
#define TASK_STEP_REQ    (1 << 4)				// 次回ステップ予約
#define TASK_NOPAUSE     (1 << 5)				// ポーズしない

enum enmTASK_PRIO {
	TASK_PRI_NONE = -1,							// 検索用プライオリティ宣言

	TASK_PRI_SYS = 1,							// 最初に実行されるタスク

	TASK_PRI_01,								// アプリケーションで利用
	TASK_PRI_02,								// アプリケーションの子タスクが利用
	TASK_PRI_03,
	TASK_PRI_04,
	TASK_PRI_05,								// エディタが利用

	TASK_PRI_99,								// 最後に実行されるタスク

	//-------------
	TASK_PRI_NUM,
};

#ifdef DEBUG
	#define TASK_INFO(body, info)  PRINTF("---- %s %s ----\n", TaskGetId(body), info)
#else
	#define TASK_INFO(body, info)  __noop
#endif


/********************************************/
/*                構造体宣言                */
/********************************************/
typedef struct _sTaskBody sTaskBody;

// コールバック関数
typedef int (*TASK_FUNC)(sTaskBody *body, int msg, int lParam, int rParam);


/********************************************/
/*              グローバル変数              */
/********************************************/


/********************************************/
/*              グローバル関数              */
/********************************************/
// 初期化
extern void TaskInit(int num);
// 終了
extern void TaskFin(void);

// タスク更新処理
extern void TaskUpdate(int msg, BOOL pause);

// タスク作成
extern sTaskBody *TaskCreate(char *name, int prio, TASK_FUNC func, int lParam, int rParam);
// タスク作成(MSG_CREATE は TaskUpdate() 内で送信)
extern sTaskBody *TaskEntry(char *name, int prio, TASK_FUNC func, int lParam, int rParam);
// タスク取得
extern sTaskBody *TaskGetBody(sTaskBody *body, int prio);
// タスク取得(名前で調べる)
extern sTaskBody *TaskGetBodyFromName(sTaskBody *body, char *id_str);
// タスク数を調べる
extern int TaskGetNum(int prio);

// 汎用データポインタの取得
extern void *TaskGetVar(sTaskBody *body, int size, int area);

// メッセージ送信
extern int TaskPostMsg(sTaskBody *body, int msg, int lParam, int rParam);
// 全タスクへメッセージ送信
extern void TaskPostMsgAll(int prio, int msg, int lParam, int rParam);
// 子タスクへメッセージ送信
extern void TaskPostMsgChild(sTaskBody *parent, int msg, int lParam, int rParam);

// タスク削除(予約)
extern void TaskDeleteReq(sTaskBody *body);
// タスク削除予約
extern void TaskDeleteAllReq(int prio);
// 全タスク削除(即時実行)
extern void TaskDeleteAll(int prio);
// アプリケーションタスク削除(即時実行)
extern void TaskDeleteAppAll(void);

// タスクのスリープ予約
extern void TaskSleepReq(sTaskBody *body, int time);
// タスクのスリープ(即時実行)
extern void TaskSleep(sTaskBody *body, int time);
// タスクのスリープ解除
extern void TaskAwakeReq(sTaskBody *body, int time);
// タスクのスリープ解除(即時実行)
extern void TaskAwake(sTaskBody *body);
// タスクがスリープ中か判別
extern BOOL TaskIsSleep(sTaskBody *body);

// タスクがポーズに影響されるか設定
extern void TaskSetNoPause(sTaskBody *body, BOOL nopause);
// タスクがポーズに影響されるか判別
extern BOOL TaskIsNoPause(sTaskBody *body);

// 識別子取得
extern char *TaskGetId(sTaskBody *body);

// 親タスク設定
extern void TaskSetParent(sTaskBody *body, sTaskBody *parent);
// 親タスク取得
extern sTaskBody *TaskGetParent(sTaskBody *body);
// 子タスクを列挙
extern sTaskBody *TaskEnumChild(sTaskBody *parent, sTaskBody *prev);


#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================
