
//==============================================================
#ifndef CO_TEXTURE_H
#define CO_TEXTURE_H
//==============================================================

#include "co_common.h"

#ifdef __cplusplus
extern              "C"
{
#endif

/********************************************/
/*             定数・マクロ宣言             */
/********************************************/


/********************************************/
/*                構造体宣言                */
/********************************************/
typedef struct _sTexture sTexture;


/********************************************/
/*              グローバル変数              */
/********************************************/


/********************************************/
/*              グローバル関数              */
/********************************************/
// 初期化
extern void TexInit(void);
// 終了
extern void TexFin(void);

// テクスチャ読み込み
extern sTexture *TexRead(char *file);
// テクスチャのクローン
extern sTexture *TexClone(sTexture *texture);
// テクスチャ破棄
extern void TexDestroy(sTexture *texture);

// ハンドルを取得
extern sTexture *TexGetFromName(char *id_str);
// 識別子を取得
extern char *TexGetName(sTexture *texture);
// サイズを取得
extern void TexGetSize(FVector2 *res, sTexture *texture);
// UV最大値を取得
extern void TexGetUV(FVector2 *res, sTexture *texture);

// テクスチャを有効にする
extern void TexBind(sTexture *texture);


#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================

