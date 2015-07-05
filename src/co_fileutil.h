
//==============================================================
#ifndef CO_FILEUTIL_H
#define CO_FILEUTIL_H
//==============================================================

#include "co_common.h"
#include "co_file.h"

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


/********************************************/
/*              グローバル変数              */
/********************************************/


/********************************************/
/*              グローバル関数              */
/********************************************/
// フルパスからファイル名 + 拡張子のみを検出
extern const char *GetFileBaseName(const char *fname);
// ファイル名から拡張子を取得
extern const char *GetFileExt(const char *file);
// ファイル名をパスと実ファイル名に分離
extern void GetPathName(const char *fname, char *path, char *file, BOOL flag);
// カレントパスを取り除いたパスを返却する
extern char *GetCurPath(char *path, char *cur);

// malloc して読み込む
extern void *_MmFileLoadB(const char *file, int area);
// MEM_FS 領域に malloc して読み込む
#define MmFileLoadB(file)  _MmFileLoadB(file, MEM_FS)
// MmFileLoadB() で読み込んだサイズを得る
extern size_t MmFileGetSize(void);
// ファイルが存在するかチェック
extern BOOL MmFileCheck(const char *file);

// テキストファイルを読み込む
extern void *MmTextFileLoad(const char *file);

// ファイルを書き出す
extern void MmFileWriteB(const char *file, void *ptr, size_t len);
// PATH_BACKUP にバックアップをとる
extern void MakeBackupFile(const char *fname);


#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================

