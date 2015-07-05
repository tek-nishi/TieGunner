//
//	汎用パラメータ管理
//

//==============================================================
#ifndef CO_PARAM_H
#define CO_PARAM_H
//==============================================================

#include "co_common.h"
#include "co_texture.h"
#include "co_sound.h"

#ifdef __cplusplus
extern              "C"
{
#endif

/********************************************/
/*             定数・マクロ宣言             */
/********************************************/
// パラメータの型一覧
enum enmPARAM_TYPE {
	PARAM_NONE = -1,							// データ無し

	PARAM_REAL,
	PARAM_V2,
	PARAM_V3,
	PARAM_V4,
	PARAM_STR,

	PARAM_TEX,
	PARAM_SND,
};


/********************************************/
/*                構造体宣言                */
/********************************************/
typedef struct _sParam sParam;


/********************************************/
/*              グローバル変数              */
/********************************************/


/********************************************/
/*              グローバル関数              */
/********************************************/
// 初期化
extern void ParamInit(void);
// 終了
extern void ParamFin(void);

// 読み込み
extern sParam *ParamRead(char *file);
// 書き出し
extern void ParamWrite(char *file, sParam *param);
// データ破棄
extern void ParamDestroy(sParam *param);
// 全データ破棄
extern void ParamDestroyAll(void);

// パラメータが存在するかチェック
extern BOOL ParamIsExists(sParam *param, char *id_str);
// パラメータの型を取得
extern int ParamGetType(sParam *param, char *id_str);
// 文字列パラメータ取得
extern char *ParamGetStr(sParam *param, char *id_str);
// REAL型パラメータ取得
extern REAL ParamGetReal(sParam *param, char *id_str);
// FVector2型パラメータ取得
extern FVector2 *ParamGetFVec2(sParam *param, char *id_str);
// FVector3型パラメータ取得
extern FVector3 *ParamGetFVec3(sParam *param, char *id_str);
// FVector4型パラメータ取得
extern FVector4 *ParamGetFVec4(sParam *param, char *id_str);
// テクスチャパラメータ取得
extern sTexture *ParamGetTex(sParam *param, char *id_str);
extern SndObj *ParamGetSnd(sParam *param, char *id_str);

// 文字列パラメータ変更
extern BOOL ParamSetStr(sParam *param, char *id_str, char *str);
// REAL型パラメータ変更
extern BOOL ParamSetReal(sParam *param, char *id_str, REAL r);
// FVector2型パラメータ変更
extern BOOL ParamSetFVec2(sParam *param, char *id_str, FVector2 *vec);
// FVector3型パラメータ変更
extern BOOL ParamSetFVec3(sParam *param, char *id_str, FVector3 *vec);
// FVector4型パラメータ変更
extern BOOL ParamSetFVec4(sParam *param, char *id_str, FVector4 *vec);

#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================

