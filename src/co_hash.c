//
//	ハッシュ法によるデータ検索
//

#include "co_hash.h"
#include "co_memory.h"


#define _MODIFY_NISHI							// _NISHI 修正個所
#define hashMalloc(a)  MemMalloc(MEM_APP, a, "hash")


struct _IHASH {
	int	hashMax, hashMask;
	int	words, strings;
	HASH **hashTop;

	HASH *top, *use, *free;
};


#ifdef _MODIFY_NISHI

//==============================================================
static HASH *l_new(IHASH *hash)
//--------------------------------------------------------------
// オブジェクトを確保
//--------------------------------------------------------------
// in:	hash = ヘッダ
//--------------------------------------------------------------
// out:	データポインタ(NULL = 確保失敗)
//==============================================================
{
	HASH *hp;

	// 未使用領域から１つ取り出す
	//----------------------------
	hp = hash->free;
	if(hp)
	{
		hash->free = hp->next;

		// 使用領域の先頭に追加
		//----------------------
		hp->next = hash->use;
		hash->use = hp;
	}

	return hp;
}

//==============================================================
static void l_delete(IHASH *hash, HASH *hp)
//--------------------------------------------------------------
// オブジェクトを破棄
//--------------------------------------------------------------
// in:	hash = ヘッダ
//		hp   = 破棄するオブジェクト
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	HASH *prev;

	prev = hash->use;
	if(prev == hp)
	{
		// 先頭データの場合
		//------------------
		hash->use = hp->next;
	}
	else
	{
		// 途中のデータの場合
		//--------------------
		while(prev->next != hp)
			prev = prev->next;
		prev->next = hp->next;
	}

	// 未使用領域の先頭に追加
	//------------------------
	hp->next = hash->free;
	hash->free = hp;
}

//==============================================================
static void setString(HASH *hp, const char *s)
//--------------------------------------------------------------
// 文字列をコピー
//--------------------------------------------------------------
// in:	hp = ハッシュハンドル
//		s  = 文字列
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	int j;
	char *str;

	j = (int)strlen(s) + 1;
	if(j <= ID_MAXLEN)
	{
		str = hp->s;
		hp->str = NULL;
	}
	else
	{
		str = (char *)hashMalloc(j);
		hp->str = str;
	}
	strcpy(str, s);
}

//==============================================================
static char *getString(HASH *hp)
//--------------------------------------------------------------
// 文字列を取得
//--------------------------------------------------------------
// in:	hp = ハッシュハンドル
//--------------------------------------------------------------
// out:	文字列
//==============================================================
{
	return hp->str ? hp->str : hp->s;
}

//==============================================================
static void deleteString(HASH *hp)
//--------------------------------------------------------------
// 文字列を破棄
//--------------------------------------------------------------
// in:	hp = ハッシュハンドル
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	FreeWork(hp->str);
}

//==============================================================
static BOOL isString(HASH *hp)
//--------------------------------------------------------------
// 文字列によるハッシュか判別
//--------------------------------------------------------------
// in:	hp = ハッシュハンドル
//--------------------------------------------------------------
// out:	文字列
//==============================================================
{
	return TRUE;
}

#endif

//==============================================================
IHASH *OpenHash(int hashMax)
//--------------------------------------------------------------
// 検索テーブルを開く
//--------------------------------------------------------------
// in:	最大登録数
//--------------------------------------------------------------
// out:	アクセスハンドル
//==============================================================
{
	int	i, j;
	IHASH *info;

	i =  hashMax;
	j = 1;
	while(i)
	{
		j += j;
		i >>= 1;
	}
	j >>= 1;
	if(j==0)
		j = 256;

//	PRINTF("Hash max: %d(%d)\n", j, hashMax);
	info = (IHASH *)hashMalloc(sizeof(IHASH));
	info->hashMax = j;
	info->hashMask = j - 1;

	info->hashTop = (HASH **)hashMalloc(sizeof(HASH *) * info->hashMax);
	ASSERT(info->hashTop);
	info->words = 0;
	for(i=0; i<info->hashMax; ++i)
		info->hashTop[i] = NULL;

#ifdef _MODIFY_NISHI
	{
		int i;
		HASH *hp;

		info->use = NULL;
		info->top = (HASH *)GetWork(sizeof(HASH) * hashMax, "HASH");
		info->free = info->top;

		hp = info->free;
		for(i = 0; i < (hashMax - 1); ++i)
		{
			hp->next = hp + 1;
			++hp;
		}
		hp->next = NULL;
	}
#endif

	return info;
}

//==============================================================
void CloseHash(IHASH *info)
//--------------------------------------------------------------
// 検索テーブルを閉じる
//--------------------------------------------------------------
// in:	info = アクセスハンドル
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	int i;
	HASH *h, *m;

	for(i=0; i<info->hashMax; ++i)
	{
		h = info->hashTop[i];
		if(h)
		{
			do
			{
				m = h;
				h = m->n;
#ifdef _MODIFY_NISHI
				deleteString(m);
#else
				if(m->s) Free(m->s);
#endif

#ifdef _MODIFY_NISHI
				l_delete(info, m);
#else
				Free(m);
#endif
			}
			while(h!=NULL);
		}
	}
#ifdef _MODIFY_NISHI
	Free(info->top);
#endif
	Free(info->hashTop);
	Free(info);
}

//==============================================================
void ClearHash(IHASH *info)
//--------------------------------------------------------------
// 検索テーブルを再初期化
//--------------------------------------------------------------
// in:	info = アクセスハンドル
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	int i;
	HASH *h, *m;

	for(i=0; i<info->hashMax; ++i)
	{
		h = info->hashTop[i];
		if(h)
		{
			do
			{
				m = h;
				h = m->n;
#ifdef _MODIFY_NISHI
				deleteString(m);
#else
				if(m->s) Free(m->s);
#endif

#ifdef _MODIFY_NISHI
				l_delete(info, m);
#else
				Free(m);
#endif
			}
			while(h!=NULL);
		}
		info->hashTop[i] = NULL;
	}
}

//==============================================================
static int CreateHash(const char *s)
//--------------------------------------------------------------
// 文字列からハッシュを作成
//--------------------------------------------------------------
// in:	s = 文字列
//--------------------------------------------------------------
// out:	ハッシュ値
//==============================================================
{
#ifdef _MODIFY_NISHI
	int i, j, k;

	k = j = 0;
	while(*s != 0)
	{
		i = *s;
		k += j ^ ((i << 5) + i + k);
		j += 1;
		s += 1;
	}

	return k;
#else
	register int	i, j, k;

	k = j = 0;
	while((i = *s++)!=0) k += j++ ^ ((i << 5) + i + k);
	return k;
#endif
}

//==============================================================
HASH *LookupString(IHASH *info, const char *s)
//--------------------------------------------------------------
// イメージ検索
//--------------------------------------------------------------
// in:	info = アクセスハンドル
//		s    = 文字列
//--------------------------------------------------------------
// out:	登録値(NULL = 該当する値なし)
//==============================================================
{
	HASH *p;

#ifdef _MODIFY_NISHI
	p = info->hashTop[ CreateHash(s) & info->hashMask ];
	while(p != NULL)
	{
		// コリジョンがあるので、ちゃんと一致しているか調べる
		//----------------------------------------------------
		if(strcmp(getString(p), s) == 0)
			break;

		p = (HASH *)p->n;
	}

	return p;
#else
	p = info->hashTop[ CreateHash(s) & info->hashMask ];
	while(p!=NULL)
	{
	if(strcmp(p->s, s)==0) return p;
	else p = (HASH *)p->n;
	}
	return p;
#endif
}

//==============================================================
HASH *InstallString(IHASH *info, const char *s)
//--------------------------------------------------------------
// 文字列の登録
//--------------------------------------------------------------
// in:	info = アクセスハンドル
//		s    = 登録する文字列(ID_MAXLEN - 1)
//--------------------------------------------------------------
// out:	登録値(NULL = 既に同じ文字列が登録されている)
//==============================================================
{
	int i;
	HASH *p;
#ifndef _MODIFY_NISHI
	int j;
	char *str;
#endif

	i = CreateHash(s) & info->hashMask;
	p = info->hashTop[i];
	if(p==NULL)
	{
#ifdef _MODIFY_NISHI
		p = l_new(info);
		ASSERT(p);
#else
		p = (HASH *)hashMalloc(sizeof(HASH));
#endif

#ifdef _MODIFY_NISHI
		setString(p, s);						// 文字列をコピー
#else
		j = strlen(s) + 1;
		p->s = (char *)hashMalloc(j);
		strcpy(p->s, s);
#endif
		p->a = info->words;
		p->n = NULL;
		info->words += 1;

		info->hashTop[i] = p;

#ifdef MSG_TEXT
		PRINTF("'%s' new install.(Words:%d)\n", s, info->words);
#endif
	}
	else
	{
		while(p!=NULL)
		{
#ifdef _MODIFY_NISHI
			if(strcmp(getString(p), s)==0)
#else
			if(strcmp(p->s, s)==0)
#endif
			{
#ifdef MSG_TEXT
				PRINTF("'%s' installed.\n", s);
#endif
				p = NULL;
			}
			else
			if(p->n!=NULL)
			{
				p = (HASH *)p->n;
			}
			else
			{
				// コリジョンが発生した場合の措置
				//--------------------------------
#ifdef _MODIFY_NISHI
				p->n = l_new(info);
				ASSERT(p->n);
#else
				p->n = (HASH *)hashMalloc(sizeof(HASH));
#endif
				p = (HASH *)p->n;

#ifdef _MODIFY_NISHI
				setString(p, s);				// 文字列をコピー
#else
				j = strlen(s) + 1;
				p->s = (char *)hashMalloc(j);
				strcpy(p->s, s);
#endif

				p->a = info->words;
				p->n = NULL;
				info->words += 1;
#ifdef MSG_TEXT
				PRINTF("'%s' lookup and install.(Words:%d)\n", s, info->words);
#endif
				return p;
			}
		}
	}
	return p;
}

//==============================================================
static void FreeHashString(IHASH *info, HASH *hp)
//--------------------------------------------------------------
// 登録文字列の破棄
//--------------------------------------------------------------
// in:	info = アクセスハンドル
//		hp   = 削除する値
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	info->words--;
#ifdef MSG_TEXT
	#ifdef _MODIFY_NISHI
		PRINTF("Uninstall string: '%s'\n", getString(hp));
	#else
		PRINTF("Uninstall string: '%s'\n", hp->s);
	#endif
#endif

#ifdef _MODIFY_NISHI
	deleteString(hp);
#else
	Free(hp->s);
#endif
}

//==============================================================
void UninstallString(IHASH *info, const char *s)
//--------------------------------------------------------------
// 登録文字列を削除
//--------------------------------------------------------------
// in:	info = アクセスハンドル
//		s    = 登録文字列
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	HASH	*h, **hh;

	hh = &info->hashTop[ CreateHash(s) & info->hashMask ];

	h = *hh;
	if(h==NULL)
		return;

	while(TRUE)
	{
		if(h->n==NULL)
		{
			FreeHashString(info, h);
#ifdef _MODIFY_NISHI
			l_delete(info, h);
#else
			Free(h);
#endif
			*hh = NULL;

			return;
		}
		else
		{
#ifdef _MODIFY_NISHI
			if(strcmp(getString(h), s)==0)
#else
			if(strcmp(h->s, s)==0)
#endif
			{
				*hh = (HASH *)h->n;				/* 次のポインターを繋ぐ */
				FreeHashString(info, h);
#ifdef _MODIFY_NISHI
				l_delete(info, h);
#else
				Free(h);
#endif
				return;
			}
			else
			{
				hh = &h;
				h = (HASH *)h->n;
			}
		}
	}
}

//==============================================================
char *GetHashString(HASH *hp)
//--------------------------------------------------------------
// ハンドルの文字列を取得
//--------------------------------------------------------------
// in:	hp = ハンドル
//--------------------------------------------------------------
// out:	登録文字列
//==============================================================
{
	return getString(hp);
}

//==============================================================
HASH **CreateHashList(IHASH *info)
//--------------------------------------------------------------
// ハッシュの全リストを作る
//--------------------------------------------------------------
// in:	info = アクセスハンドル
//--------------------------------------------------------------
// out:	ポインタリスト(NULL = リストなし)
//==============================================================
{
	int	i, j;
	HASH	**ip, *p;

	if(info->words==0)
		return NULL;

	ip = (HASH **)hashMalloc(sizeof(HASH *) * (info->words + 1));
	j = 0;
	for(i=0; i<info->hashMax; ++i)
	{
		if((p = info->hashTop[i])!=NULL)
		{
			do
			{
#ifdef _MODIFY_NISHI
				if(isString(p))
				{
					ip[j] = p;
					++j;
				}
#else
				if(p->s)
				{
					ip[j] = p;
					++j;
				}
#endif
				p = (HASH *)p->n;
			}
			while(p);
		}
	}
	ip[j] = NULL;

	return ip;
}

//==============================================================
static void SwapHash(HASH **v, int a, int b)
//--------------------------------------------------------------
// ハッシュの交換
//--------------------------------------------------------------
// in:	v    = ハッシュリスト
//		a, b = 入れ替える値へのインデックス
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	HASH	*p;

	p = *(v + a);
	*(v + a) = *(v + b);
	*(v + b) = p;
}

//==============================================================
static void QsortHash(HASH **v, int left, int right)
//--------------------------------------------------------------
// ハッシュのソート
//--------------------------------------------------------------
// in:	v           = ハッシュリスト
//		left, right = ソート領域
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	int	i, last;

	if(left >= right)
		return;

	SwapHash(v, left, (left + right) / 2);
	last = left;
	for(i=left+1; i<=right; ++i)
	{
#ifdef _MODIFY_NISHI
		if(strcmp(getString(v[i]), getString(v[left])) < 0)
			SwapHash(v, ++last, i);
#else
		if(strcmp(v[i]->s, v[left]->s) < 0)
			SwapHash(v, ++last, i);
#endif
	}
	SwapHash(v, left, last);

	// 再帰処理しているので、スタック溢れに注意
	//------------------------------------------
	QsortHash(v, left, last-1);
	QsortHash(v, last+1, right);
}

//==============================================================
HASH **SortHashList(IHASH *info)
//--------------------------------------------------------------
// ハッシュのソート
//--------------------------------------------------------------
// in:	info = アクセスハンドル
//--------------------------------------------------------------
// out:	ソートされたリスト
//==============================================================
{
	HASH	**pp;

	pp = CreateHashList(info);
	if(pp==NULL || info->words <= 1)
		return pp;

	QsortHash(pp, 0, info->words - 1);

	return pp;
}


//==============================================================
static void QsortHashValue(HASH **v, int left, int right, int ofs)
//--------------------------------------------------------------
// ハッシュ値のソート
//--------------------------------------------------------------
// in:	v           = ハッシュ値
//		left, right = ソート範囲
//		ofs         = インデックス
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	int	i, last;
	HASH	h;

	if(left >= right)
		return;

	SwapHash(v, left, (left + right) / 2);
	last = left;

	if( ( (char *)&h.a - (char *)&h )==ofs)
	{
		for(i=left+1; i<=right; ++i)
		{
			if(v[i]->a < v[left]->a)
				SwapHash(v, ++last, i);
		}
	}
	else
	{
		for(i=left+1; i<=right; ++i)
		{
			if(v[i]->b < v[left]->b)
				SwapHash(v, ++last, i);
		}
	}
	SwapHash(v, left, last);

	// 再帰処理しているので、スタック溢れに注意
	//------------------------------------------
	QsortHashValue(v, left, last-1, ofs);
	QsortHashValue(v, last+1, right, ofs);
}

//==============================================================
HASH **SortHashValueA(IHASH *info)
//--------------------------------------------------------------
// 値Ａに対してのハッシュ値のソート
//--------------------------------------------------------------
// in:	info = アクセスハンドル
//--------------------------------------------------------------
// out:	ソートされたリスト
//==============================================================
{
	HASH	**pp, h;

	pp = CreateHashList(info);
	if(pp==NULL || info->words <= 1)
		return pp;

	QsortHashValue(pp, 0, info->words - 1, (char *)&h.a - (char *)&h);
	return pp;
}

//==============================================================
HASH **SortHashValueB(IHASH *info)
//--------------------------------------------------------------
// 値Ｂに対してのハッシュ値のソート
//--------------------------------------------------------------
// in:	info = アクセスハンドル
//--------------------------------------------------------------
// out:	ソートされたリスト
//==============================================================
{
	HASH	**pp, h;

	pp = CreateHashList(info);
	if(pp==NULL || info->words <= 1)
		return pp;

	QsortHashValue(pp, 0, info->words - 1, (char *)&h.b - (char *)&h);
	return pp;
}

//==============================================================
void ListtingHashList(IHASH *info, FILE *fp)
//--------------------------------------------------------------
// リストしたハッシュをファイルに書き出す
//--------------------------------------------------------------
// in:	info = アクセスハンドル
//		fp   = ファイルポインタ
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	HASH	**pp, *p;
	int	i;

	pp = CreateHashList(info);
	if(pp==NULL)
		return;

	i = 0;
	while((p = pp[i])!=NULL)
	{
#ifdef _MODIFY_NISHI
		fprintf(fp, "(%5d) $%08X, $%08X : '%s'\n", i, p->a, p->b, getString(p));
#else
		fprintf(fp, "(%5d) $%08X, $%08X : '%s'\n", i, p->a, p->b, p->s);
#endif
		i += 1;
	}
	Free(pp);
}

//==============================================================
HASH **AddHashPointer(HASH **hpp, HASH *hp)
//--------------------------------------------------------------
// ハッシュリストに丸ごと加える
//--------------------------------------------------------------
// in:	hpp = ハッシュリスト
//		hp  = 加えるハッシュリスト
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	int	i;
	HASH	**nhpp, *nhp;

	if(hp==NULL)
		return NULL;

	i = 0;
	if(hpp)
	{
		while((nhp = hpp[i])!=NULL)
		{
			if(nhp==hp)
				return hpp;
			else
				i += 1;
		}
	}

	nhpp = (HASH **)hashMalloc(sizeof(HASH *) * (i + 2));

	if(i)
	{
		i = 0;
		while((nhp = hpp[i])!=NULL)
		{
			nhpp[i] = nhp;
			++i;
		} 
		Free(hpp);
	}
	nhpp[i] = hp;
	nhpp[i + 1] = NULL;

	return nhpp;
}

