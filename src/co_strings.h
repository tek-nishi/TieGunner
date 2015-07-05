
//==============================================================
#ifndef CO_STRINGS_H
#define CO_STRINGS_H
//==============================================================

#include "co_common.h"

#ifdef __cplusplus
extern              "C"
{
#endif

/********************************************/
/*             定数・マクロ宣言             */
/********************************************/
#define L_ID_FLD_MAXLEN	 26						// ビットフィールド文字列長

// ソースファイル名を切り出す
//----------------------------------------
#define MSG_SRC(_f)		l_strstr("src", _f)

// 16 文字コピー(必ずコピー先の終端に '\0' を入れる)
//---------------------------------------------------
#define STRCPY16(dst, src)  StrCopyLength(dst, 16, src)

// ファイルパス長の文字列コピー(必ずコピー先の終端に '\0' を入れる)
//---------------------------------------------------
#define PATHCOPY(dst, src)  StrCopyLength(dst, FNAME_MAXLEN, src)


/********************************************/
/*                構造体宣言                */
/********************************************/


/********************************************/
/*              グローバル変数              */
/********************************************/


/********************************************/
/*              グローバル関数              */
/********************************************/
// 文字列の切り出し
extern const char *l_strstr(const char *str, const char *fname);
// 指定長の文字列コピー
extern void StrCopyLength(char *dst, int len, const char *src);
// ユニークな文字列を生成する
extern const char *StrMakeUniqueName(void);

// ホワイトスペースを飛ばす
extern char *StrSkipSpace(char *ptr);
// 次のトークンへ移動
extern char *StrNextToken(char *ptr);
// 次の行を返す
extern char *StrNextLine(char *ptr);

// コメント行かチェック
extern BOOL StrIsComment(char *ptr);
// テキスト終端かチェック
extern BOOL StrIsTextEnd(char *ptr);
// ブランクかチェック
extern BOOL StrIsBlank(char *ptr);
// 数字かチェック
extern BOOL StrIsValue(char *ptr);
// SJISコードか判別
extern BOOL StrIsSJIS(char *ptr);

// 文字列取得
extern int StrGetTextId(char *dst, char *src);
// 数字を変換
extern int StrGetValue(char *ptr);
// 数字を変換
extern REAL StrGetReal(char *ptr);
// 数字をスキップ
extern char *StrSkipValue(char *ptr);
// １行のサイズを取得(SJISも判別)
extern int StrGetLineLen(char *ptr);

// Bin型文字列をInt型10進数に変換する
extern int StrBinStrToInt(char *src);
// Int型10進数をBin型文字列に変換する
extern void StrIntToBinStr(char *dst, int src);

// 文字列を小文字へ変換
extern void StrToLower(char *dst);
// 文字列を大文字へ変換
extern void StrToUpper(char *dst);


//--------------------------------------------
// インライン関数群
//--------------------------------------------

//===============================================================
static const char *MSG_FILE(const char *_f)
//--------------------------------------------------------------
// ファイル名から PATH_DEVELOP を除いたポインタを返す
//--------------------------------------------------------------
//===============================================================
{
	const char *out = l_strstr(PATH_DEVELOP, _f);
	return l_strstr("/", out);
}

#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================

