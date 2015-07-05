//
//	ハッシュ法によるデータ検索
//

/*

	ハッシュ法を使った高速検索を提供します

	任意の長さの文字列と、任意長の値を扱えます。

	hash = OpenHash(256)				// テーブルオープン
	p = InstallString(hash, "test");	// 文字列を登録

	p->s = "hoge"						// 適当に値を代入
	p->a = hoge;

	p = LookupString(hash, "test");		// 文字列から検索
	hoge = p->a;						// 値を取り出す

	CloseHash(hash)						// 後始末

*/

//==============================================================
#ifndef CO_HASH_H
#define CO_HASH_H
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
typedef struct hash {
	char s[ID_MAXLEN];
	char *str;
	int a, b;
	void *p;
	struct hash *n;								// 次の構造体へのポインタ

	struct hash *next;
} HASH;

typedef struct _IHASH IHASH;


/********************************************/
/*              グローバル変数              */
/********************************************/


/********************************************/
/*              グローバル関数              */
/********************************************/
// 検索テーブルを開く
extern IHASH *OpenHash(int hashMax);
// 検索テーブルを閉じる
extern void CloseHash(IHASH *info);
// 検索テーブルを再初期化
extern void ClearHash(IHASH *info);
// イメージ検索
extern HASH *LookupString(IHASH *info, const char *s);
// 文字列の登録
extern HASH *InstallString(IHASH *info, const char *s);
// 登録文字列を削除
extern void UninstallString(IHASH *info, const char *s);
// ハンドルの文字列を取得
extern char *GetHashString(HASH *hp);

// ハッシュの全リストを作る
extern HASH **CreateHashList(IHASH *info);
// ハッシュのソート
extern HASH **SortHashList(IHASH *info);
// 値Ａに対してのハッシュ値のソート
extern HASH **SortHashValueA(IHASH *info);
// 値Ｂに対してのハッシュ値のソート
extern HASH **SortHashValueB(IHASH *info);
// リストしたハッシュをファイルに書き出す
extern void ListtingHashList(IHASH *info, FILE *fp);
// ハッシュリストに丸ごと加える
extern HASH **AddHashPointer(HASH **hpp, HASH *hp);


#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================

