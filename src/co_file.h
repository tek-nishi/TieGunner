
/*

	通常のファイルと、パックされたファイルとの違いを意識しないで扱えるファイルアクセス
	を提供します。
	※セパレーターには「/」を使ってください。

	制約:
	・マウントできるファイルは同時に１つだけです
	・FsWrite() で作成したファイルは書き込み専用で開いているので、そのまま読み込む事は出来ません

*/

//==============================================================
#ifndef CO_FILE_H
#define CO_FILE_H
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
typedef struct _sFILE sFILE;


/********************************************/
/*              グローバル変数              */
/********************************************/


/********************************************/
/*              グローバル関数              */
/********************************************/
// 初期化
extern void FsInit(void);
// 終了
extern void FsFin(void);

// イメージファイルのマウント
extern void FsMountImage(const char *image);
// イメージファイルのアンマウント
extern void FsUnMountImage(void);

// ファイルオープン
extern sFILE *FsOpen(const char *name);
// ファイルクローズ
extern void FsClose(sFILE *fp);
// ファイル読み込み
extern size_t FsRead(sFILE *fp, void *ptr, size_t size);
// ファイルシーク
extern size_t FsSeek(sFILE *fp, size_t ofs, int whence);
// ファイルの読み込み位置を取得
extern size_t FsTell(sFILE *fp);

// ファイルサイズの取得(セクタサイズに切り上げた値)
extern size_t FsGetSize(sFILE *fp);
// 実ファイルサイズの取得
extern size_t FsGetSizeOrig(sFILE *fp);
// ファイル名からファイルサイズを取得
extern size_t FsGetFileSize(const char *name);
// ファイル名から実ファイルサイズを取得
extern size_t FsGetFileSizeOrig(const char *name);

// ファイルを新規作成する
extern sFILE *FsCreate(const char *name);
// データをファイルに書き出す
extern void FsWrite(sFILE *fp, void *ptr, size_t size);


#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================

