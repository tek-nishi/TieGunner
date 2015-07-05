//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
//  < fpack >
//   2004 ASTROLL Inc. All Rights Reserved.
//--------------------------------------------------------------
//
//	ハッシュ法による検索
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  $Id: hash.h,v 1.1.1.1 2004/02/09 08:13:36 nishi Exp $
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
/*

	ハッシュ法を使った高速検索を提供します

	任意の長さの文字列と、任意長の値を扱えます。

	hash = OpenHash(256)				// テーブルオープン(テーブルサイズは２のべき乗)
	p = InstallString(hash, "test");	// 文字列を登録

	p->s = "hoge"						// 適当に値を代入
	p->a = hoge;

	p = LookupString(hash, "test");		// 文字列から検索
	hoge = p->a;						// 値を取り出す

	CloseHash(hash)						// 後始末

*/

//==============================================================
#ifndef _HASH_H_
#define _HASH_H_
//==============================================================

/********************************************/
/*             定数・マクロ宣言             */
/********************************************/


/********************************************/
/*                構造体宣言                */
/********************************************/
typedef struct hash {
	char *s;
	int a, b;
	struct hash *n;								// 次の構造体へのポインタ
} HASH;

typedef struct {
	int	hashMax, hashMask;
	int	words, strings;
	HASH **hashTop;
	int	imageSize;
} IHASH;


/********************************************/
/*              グローバル変数              */
/********************************************/


/********************************************/
/*              グローバル関数              */
/********************************************/
extern IHASH *OpenHash(int hashMax);
extern void CloseHash(IHASH *info);
extern HASH *LookupString(IHASH *info, const char *s);
extern HASH *LookupImage(IHASH *info, const char *s);
extern HASH *InstallString(IHASH *info, const char *s);
extern HASH *InstallImage(IHASH *info, const char *s);
extern void UninstallString(IHASH *info, const char *s);
extern void UninstallImage(IHASH *info, const char *s);
extern HASH **CreateHashList(IHASH *info);
extern HASH **SortHashList(IHASH *info);
extern HASH **SortHashValueA(IHASH *info);
extern HASH **SortHashValueB(IHASH *info);
extern void ListtingHashList(IHASH *info, FILE *fp);
extern HASH **AddHashPointer(HASH **hpp, HASH *hp);


//==============================================================
#endif
//==============================================================

