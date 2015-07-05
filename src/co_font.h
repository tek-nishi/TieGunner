
/*

	半角のフォント表示を行います。主にデバッグやダイアログ表示に使います。
	テクスチャのサイズ制限は特に無く、文字が16x16で配置されていれば。


	拡張された機能

	色指定
		$C0 : 黒
		  1 : 青
		  2 : 赤
		  3 : マゼンタ
		  4 : 緑
		  5 : シアン
		  6 : 黄
		  7 : 白(default)
		  8 : 拡張色-1
		  9 : 拡張色-2

	ハイライト
		$H0 : 通常(default)
		  1 : グレー
		  2 : 半透明

	点滅
		$B0 : 点滅無し(default)
		  1 : 点滅(速)
		  2 : 点滅(遅)

	フォント切り替え
		$F0 : FONT_SYS(default)
		  1 : FONT_APP_1
		  2 : FONT_APP_2
		  3 : FONT_APP_3
		  4 : FONT_APP_4

	アルファ値
	    $A<falue>$

	拡張フォントコード
		$E0 : 

	'$'を表示したい時は、'$$'と２個並べて書く。

*/

//==============================================================
#ifndef CO_FONT_H
#define CO_FONT_H
//==============================================================

#include "co_common.h"

#ifdef __cplusplus
extern              "C"
{
#endif

/********************************************/
/*             定数・マクロ宣言             */
/********************************************/
enum enmFONT_IDX {
	FONT_SYS,									// システムフォント
	FONT_APP_1,									// アプリケーションフォント
	FONT_APP_2,
	FONT_APP_3,
	FONT_APP_4,

	//--------------
	FONT_NUM
};


/********************************************/
/*                構造体宣言                */
/********************************************/
typedef struct {
	char tex_id[ID_MAXLEN];						// テクスチャ名
	int width;									// 文字幅
	int height;									// 改行幅
} sFontInfo;


/********************************************/
/*              グローバル変数              */
/********************************************/


/********************************************/
/*              グローバル関数              */
/********************************************/
// 初期化
extern void FontInit(void);
// 終了
extern void FontFin(void);

// フォントのセットアップ
extern void FontSetup(int index, char *file, int next_x, int next_y);
// フォント情報の取得
extern void FontGetInfo(sFontInfo *res, int index);

// 表示
extern void FontPrint(int x, int y, int prio, char *str);
// 表示(printf準拠)
extern void FontPrintF(int x, int y, int prio, char *fmt, ...);

// 表示幅を取得
extern int FontGetPrintWidth(char *str);
// 拡張色を設定
extern void FontSetExColor(int index, sRGBA *col);


#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================

