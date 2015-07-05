
/*

	データタイプ

	PNG_COLOR_TYPE_PALETTE		インデックスカラー
	PNG_COLOR_TYPE_RGB			RGB
	PNG_COLOR_TYPE_RGB_ALPHA	RGBA

*/

//==============================================================
#ifndef CO_PNG_H
#define CO_PNG_H
//==============================================================

#include "co_common.h"
#if defined (_MSC_VER)
#ifdef DEBUG
#pragma comment (lib, "libpngd.lib")
#else
#pragma comment (lib, "libpng.lib")
#endif
#endif

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
typedef struct {
	u_char red, green, blue, alpha;
} sPalette;

typedef struct {
	int type;									// データタイプ
	int width, height;							// データサイズ
	u_char *image;								// イメージ格納ポインタ

	int palnum;									// パレット数
	sPalette *pal;								// パレット格納ポインタ
} sPNG;


/********************************************/
/*              グローバル変数              */
/********************************************/


/********************************************/
/*              グローバル関数              */
/********************************************/
extern sPNG *PngRead(void *ptr, size_t size);
extern void PngDestroy(sPNG *hdr);

extern void PngWrite(char *file, int w, int h, u_char *ptr);


#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================
