
/*

  TODO:高速列挙

 */

//==============================================================
#ifndef CO_OBJ_H
#define CO_OBJ_H
//==============================================================

#include "co_common.h"
#include "co_param.h"

#ifdef __cplusplus
extern              "C"
{
#endif

/********************************************/
/*             定数・マクロ宣言             */
/********************************************/
// 専用メッセージ
enum enmMSG_OBJ {
	_MSG_OBJ = MSG_OBJ,							// ダミー
};

// タイプはアプリケーション側で指定
//------------------------------
#define OBJ_TYPE_ALL     (~0)


/********************************************/
/*                構造体宣言                */
/********************************************/
typedef struct _sOBJ sOBJ;
typedef int (*OBJ_PROC)(sOBJ *obj, sParam *param, int msg, int lParam, int rParam);


/********************************************/
/*              グローバル変数              */
/********************************************/


/********************************************/
/*              グローバル関数              */
/********************************************/
// 初期化
extern void ObjInit(void);
// オブジェクトのセットアップ
extern sParam *ObjSetup(char *id_str);
// セットアップ済みのパラメータを取得
extern sParam *ObjGetSetupParam(char *id_str);
// セットアップしたデータの破棄
extern void ObjDeleteParam(char *id_str);
// セットアップした全データの破棄
extern void ObjDeleteParamAll(void);
extern void ObjDeleteAll(u_int type);

// オブジェクト取得
extern sOBJ *ObjGetNext(sOBJ *prev, u_int type, char *id_str);
// メッセージ送信
extern int ObjPostMsg(sOBJ *obj, int msg, int lParam, int rParam);
// 全オブジェクトへメッセージ送信
extern BOOL ObjPostMsgAll(u_int type, int msg, BOOL abort, int lParam, int rParam);

// オブジェクト生成
extern sOBJ *ObjCreate(char *id_str, u_int type, OBJ_PROC func, int lParam, int rParam);
// オブジェクト削除要求
extern void ObjKillReq(sOBJ *obj);

// ワーク取得
extern void *ObjGetVar(sOBJ *obj, int size);
// パラメータ取得
extern sParam *ObjGetParam(sOBJ *obj);
// タイプ取得
extern u_int ObjGetType(sOBJ *obj);
// タイプ変更
extern void ObjSetType(sOBJ *obj, u_int type);
// 位置を取得
extern FVector2 *ObjGetPos(sOBJ *obj);
// 位置を変更
extern void ObjSetPos(sOBJ *obj, REAL x, REAL y);

extern REAL ObjGetDir(sOBJ *obj);
extern void ObjSetDir(sOBJ *obj, REAL dir);
extern FVector2 *ObjGetVct(sOBJ *obj);
extern void ObjSetVct(sOBJ *obj, REAL x, REAL y);
extern REAL ObjGetRadius(sOBJ *obj);
extern void ObjSetRadius(sOBJ *obj, REAL radius);
extern BOOL ObjIsDead(sOBJ *obj);
extern void ObjSetDeath(sOBJ *obj, BOOL death);


#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================

