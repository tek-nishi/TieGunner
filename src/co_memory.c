//
// メモリ管理
//

#include "co_memory.h"
#include "co_os.h"
#include "co_debug.h"
#include "co_random.h"
#include "co_strings.h"


//----------------
// 各領域のサイズ
//----------------
#define MEM_SIZE     0xa00000						// 全体で使うメモリサイズ
#define MEM_SYS_SIZE 0x200000						// システム
#define MEM_DEV_SIZE 0x100000						// デバッグ用領域

#define MEM_HEADER  0x12345678						// データ破壊チェック用

#define _MODIFY_FREE								// Free() 軽量版
#ifdef DEBUG
//#define CHECK_MIN_MEMORY
#endif


typedef struct {
	int area;
	char name[ID_MAXLEN];
} sMemInfoStack;


//----------------------------
// enmMEM_AREA と並びを同じに
//----------------------------
static char *memName[] = {
	" MEM_SYS",
	" MEM_APP",

	" MEM_DEV",
};
static size_t memSizeTbl[] = {
	MEM_SYS_SIZE,
};

static sMemInfo gMem;							// メモリ環境

#ifdef DEBUG_MEMORY
	int magicNumber = 0;
#endif


/********************************************/
/*                プログラム                */
/********************************************/
//--------------------------------------------------------------
//  ローカル関数
//--------------------------------------------------------------

//==============================================================
static void setName(Header *hdr, const char *name)
//--------------------------------------------------------------
// 領域名をコピー
//--------------------------------------------------------------
// in:	hdr  = メモリヘッダ
//		name = 領域名(NULL = 名前無し)
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	if(name)
		STRCPY16(hdr->s.name, name);
	else
		hdr->s.name[0] = '\0';
}


//--------------------------------------------------------------
//  グローバル関数
//--------------------------------------------------------------

//==============================================================
void *MemDummyMalloc(void)
//--------------------------------------------------------------
// gnu malloc を使用した場合にエラーを出す
// ※ malloc() は この関数を呼ぶようにマクロ定義されている。
//--------------------------------------------------------------
// in:	なし
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	ASSERT(0);
	return NULL;
}

//==============================================================
void MemDummyFree(void)
//--------------------------------------------------------------
// gnu free を使用した場合にエラーを出す
// ※ free() は この関数を呼ぶようにマクロ定義されている。
//--------------------------------------------------------------
// in:	なし
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	ASSERT(0);
}

//==============================================================
static void bsMemoryClean(sMemHeader *header, int flag)
//--------------------------------------------------------------
// 確保されている領域を全て開放
//--------------------------------------------------------------
// in:	header = メモリハンドル
//		flag   = TRUE : 領域をランダムクリアする
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	Header *p;

	p = (Header *)header->top;

	if(flag)
	{
		/* 適当な値でクリア */
		memset(p, 0xfd, header->size * sizeof(Header));
	}

	p->s.area = MEM_NONE;
	p->s.size = header->size;
	p->s.ptr = NULL;							// NULL = エンドコード
	p->s.header = MEM_HEADER;
	strcpy(p->s.name, "free");

#ifdef DEBUG_MEMORY
	p->s.magicNumber = magicNumber;
	magicNumber = (magicNumber + 1) & 0x7;
#endif

	header->freep = p;
	header->last = NULL;

	/* デバッグ情報 */
	header->allocate = header->total = (u_int)((header->size - 1) * sizeof(Header));
	header->min_free = header->total;
	header->fragment = 0;

	PRINTF("Memory Init. %x(%x)\n", header->top, header->size * sizeof(Header));
}

//==============================================================
static void bsMemoryInit(sMemHeader *header, void *ptr, size_t size, int flag)
//--------------------------------------------------------------
// メモリ管理初期化
//--------------------------------------------------------------
// in:	header = メモリハンドル
//		ptr    = メモリ領域の先頭アドレス(64 バイトアライン)
//		size   = 領域のサイズ(単位 : バイト)
//		flag   = TRUE : 領域をランダムクリアする
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	header->top = ptr;							// 領域の位置とサイズを保持しておく
	header->size = size / (size_t)sizeof(Header);

	bsMemoryClean(header, flag);
}

//==============================================================
void bsMemoryCheck(sMemHeader *header)
//--------------------------------------------------------------
// メモリの断片化などをチェック
// ※ヘッダに書き込む
//--------------------------------------------------------------
// in:	header = メモリハンドル
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	int num;
	u_int total, allocate;
	Header *p;

	total = allocate = 0;
	num = 0;
	p = header->freep;
	while(p)
	{
		total += (u_int)p->s.size;
		if(allocate < p->s.size)
			allocate = (u_int)p->s.size;

		++num;
		p = p->s.ptr;							/* 次の空き領域 */
	}

	header->total = total * sizeof(Header);
	header->allocate = (allocate - 1) * sizeof(Header);
	header->fragment = num;

	if(header->min_free > header->total)
		header->min_free = header->total;		// 最小値を覚えておく
}

//==============================================================
static size_t bsMemoryGetFree(sMemHeader *header)
//--------------------------------------------------------------
// 確保可能な最大領域を調べる
//--------------------------------------------------------------
// in:	header = メモリハンドル
//--------------------------------------------------------------
// out:	確保可能なサイズ
//==============================================================
{
	size_t allocate;
	Header *p;

	allocate = 0;
	p = header->freep;
	while(p)
	{
		if(allocate < p->s.size)
			allocate = p->s.size;

		p = p->s.ptr;							/* 次の空き領域 */
	}

	return (allocate - 1) * sizeof(Header);
}

//==============================================================
static void *bsMalloc(sMemHeader *header, size_t size, const char *name)
//--------------------------------------------------------------
// メモリ割り当て
// ※メモリの下の方から確保
//--------------------------------------------------------------
// in:	header = メモリハンドル
//		size   = 確保するメモリサイズ
//		name   = 領域名
//--------------------------------------------------------------
// out:	見つかったメモリ領域の先頭アドレス(NULL = 確保失敗)
//==============================================================
{
	Header *p, *prevp;
	size_t nunits;
	Header *resvPtr;
	Header *resvPrevPtr;

	// 確保するブロックサイズを求める
	//--------------------------------
	ASSERT(size);
	nunits = (size + sizeof(Header) - 1) / sizeof(Header) + 1;

	p = header->freep;
	prevp = NULL;
	resvPtr = NULL;								/* 最適な場所を検索する */
	resvPrevPtr = NULL;
	while(p)
	{
		// 一番大きいアドレスで探す
		//--------------------------
		if((resvPtr < p) && ((p->s.size == nunits) || (p->s.size > (nunits + 1))))
		{
			resvPtr = p;
			resvPrevPtr = prevp;
		}
		prevp = p;
		p = p->s.ptr;
	}

	if(!resvPtr)
	{
		bsMemoryCheck(header);
		PRINTF("No memory. [%s]\nname:%s  size:%d\n  allocate:%d  total:%d\n", memName[header->area], name, size, header->allocate, header->total);
		return NULL;							/* 割り当てられる領域が無い */
	}

	p = resvPtr;
	prevp = resvPrevPtr;
	if(p->s.size == nunits)
	{
		// !!! 若干危険なコードかも !!!
		if(prevp)	prevp->s.ptr = p->s.ptr;	// リストの途中
		else		header->freep = p->s.ptr;	// リストの先頭
	}
	else
	{
		p->s.size -= nunits;					// 後尾の部分を割り当てる
		p += p->s.size;
		p->s.size = nunits;
	}

	setName(p, name);							// 領域名をセット
	p->s.header = MEM_HEADER;
	p->s.area = header->area;
	p->s.top = FALSE;

#ifdef DEBUG_MEMORY
	p->s.magicNumber = magicNumber;
	magicNumber = (magicNumber + 1) & 0x7;
#endif

	// 確保した領域をポインタリストに追加
	//------------------------------------
#ifdef _MODIFY_FREE
	if(header->last)	header->last->s.prev = p;
	p->s.prev = NULL;
#endif
	p->s.ptr = header->last;
	header->last = p;

#if 0
	//------------------------------------
	// ROM で作成した時のメモリチェック用
	//------------------------------------
	if(name)
	{
		if(!strcmp(name, "no_name"))	PRINTF("memory malloc 'no_name' %d(%x)\n", size, p);
	}
#endif

#ifdef DEBUG
	memset((void *)(p + 1), 0xfd, size);
#endif

#ifdef CHECK_MIN_MEMORY
	bsMemoryCheck(header);
#endif
	
	return (void *)(p + 1);				/* ヘッダを除いた領域を返す */
}

//==============================================================
static void *bsMalloc2(sMemHeader *header, size_t size, const char *name)
//--------------------------------------------------------------
// メモリ割り当て
// ※メモリの上のほうから確保
//--------------------------------------------------------------
// in:	header = メモリハンドル
//		size   = 確保するメモリサイズ
//		name   = 領域名
//--------------------------------------------------------------
// out:	見つかったメモリ領域の先頭アドレス
//==============================================================
{
	Header *p;
	Header *prevp;
	Header *max_p;
	Header *resv_p;
	Header *resv_prev;
	size_t nunits;

	// 確保するブロックサイズを求める
	//--------------------------------
	ASSERT(size);
	nunits = (size + sizeof(Header) - 1) / sizeof(Header) + 1;

	p = header->freep;
	prevp = NULL;
	resv_p = NULL;
	resv_prev = NULL;
	max_p = (Header *)header->top + header->size;
	while(p)
	{
		// 一番小さいアドレスで探す
		//--------------------------
		if(((p->s.size == nunits) || (p->s.size > (nunits + 1))) && (max_p > p))
		{
			resv_p = p;
			max_p = p;
			resv_prev = prevp;
		}
		prevp = p;
		p = p->s.ptr;
	}

	if(!resv_p)
	{
		bsMemoryCheck(header);
		PRINTF("No memory. [%s]\nname:%s  size:%d\n  allocate:%d  total:%d\n", memName[header->area], name, size, header->allocate, header->total);
		return NULL;							/* 割り当てられる領域が無い */
	}

	p = resv_p;
	prevp = resv_prev;
	if(p->s.size == nunits)
	{
		// 完全一致
		//----------
		// !!! 若干危険なコードかも !!!
		if(prevp)	prevp->s.ptr = p->s.ptr;	// リストの途中
		else		header->freep = p->s.ptr;	// リストの先頭
	}
	else
	{
		// 前半を割り当てる
		//------------------
		Header *free;

		free = p + nunits;
		*free = *p;
		free->s.size = p->s.size - nunits;
		p->s.size = nunits;

		if(prevp)	prevp->s.ptr = free;
		else		header->freep = free;
	}

	setName(p, name);							// 領域名をセット
	p->s.header = MEM_HEADER;
	p->s.area = header->area;
	p->s.top = TRUE;

#ifdef DEBUG_MEMORY
	p->s.magicNumber = magicNumber;
	magicNumber = (magicNumber + 1) & 0x7;
#endif

	// 確保した領域をポインタリストに追加
	//------------------------------------
#ifdef _MODIFY_FREE
	if(header->last)	header->last->s.prev = p;
	p->s.prev = NULL;
#endif
	p->s.ptr = header->last;
	header->last = p;

#if 0
	//------------------------------------
	// ROM で作成した時のメモリチェック用
	//------------------------------------
	if(name)
	{
		if(!strcmp(name, "no_name"))	PRINTF("memory malloc 'no_name' %d(%x)\n", size, p);
	}
#endif

#ifdef DEBUG
	memset((void *)(p + 1), 0xfd, size);
#endif

#ifdef CHECK_MIN_MEMORY
	bsMemoryCheck(header);
#endif

	return (void *)(p + 1);				/* ヘッダを除いた領域を返す */
}

//==============================================================
static void bsFree(sMemHeader *header, void *ptr)
//--------------------------------------------------------------
// メモリ開放
//--------------------------------------------------------------
// in:	header = メモリハンドル
//		ptr    = 開放するメモリ領域の先頭アドレス
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	Header *bp;
	Header *p;
	Header *prev;

	if(!ptr)	return;

	bp = (Header *)ptr - 1;						/* ブロックヘッダを指す */
	strcpy(bp->s.name, "free");					// 領域名の初期化

#ifdef DEBUG
	// 開放したメモリをランダムクリア
	memset(ptr, 0xfd, (bp->s.size - 1) * sizeof(Header));
#endif

	if(bp->s.area == MEM_NONE)
	{
		// ２重開放
		ASSERT(0);
		return;
	}

	// 使用中のリストから外す
	//------------------------
#ifdef _MODIFY_FREE
	if(bp->s.prev)			bp->s.prev->s.ptr = bp->s.ptr;
	if(bp->s.ptr)			bp->s.ptr->s.prev = bp->s.prev;
	if(header->last == bp)	header->last = bp->s.ptr;
#else
	p = header->last;
	prev = NULL;
	while(p)
	{
		if(bp == p)
		{
			if(prev)	prev->s.ptr = p->s.ptr;
			else		header->last = p->s.ptr;
			break;
		}
		prev = p;
		p = p->s.ptr;
	}
	ASSERT(p);
#endif

	// 空きリストに加える
	//--------------------
	if(!header->freep)
	{
		// 空き領域が全く無かった場合
		header->freep = bp;
		bp->s.ptr = NULL;
	}
	else
	{
		p = header->freep;
		while(p)
		{
			prev = p->s.ptr;
			if(bp < p)
			{
				// 先頭
				//------
				if((bp + bp->s.size) == p)
				{
					bp->s.size += p->s.size;
					bp->s.ptr = p->s.ptr;
				}
				else
				{
					bp->s.ptr = p;
				}
				header->freep = bp;
				break;
			}
			else
			if(!prev)
			{
				// 最後尾
				//--------
				if((p + p->s.size) == bp)
				{
					// 上と結合
					p->s.size += bp->s.size;
				}
				else
				{
					// 最後尾に追加
					p->s.ptr = bp;
					bp->s.ptr = NULL;
				}
				break;
			}
			else
			if((bp > p) && (bp < prev))
			{
				p->s.ptr = bp;
				bp->s.ptr = prev;

				if((p + p->s.size) == bp)
				{
					// 上と結合
					p->s.size += bp->s.size;
					p->s.ptr = bp->s.ptr;
					bp = p;
				}

				if((bp + bp->s.size) == prev)
				{
					// 下と結合
					bp->s.size += prev->s.size;
					bp->s.ptr = prev->s.ptr;
				}
				break;
			}
			p = p->s.ptr;
		}
	}

	bp->s.area = MEM_NONE;						// メモリが開放されたことをヘッダに書き込んでおく
}

//==============================================================
static void *bsRealloc(sMemHeader *header, void *ptr, size_t size, const char *name)
//--------------------------------------------------------------
// メモリ再割り当て
//--------------------------------------------------------------
// in:	header = メモリハンドル
//		ptr    = データポインタ(NULL は渡されない)
//		size   = 新しく要求するサイズ
//		name   = 領域名
//--------------------------------------------------------------
// out:	見つかったメモリ領域の先頭アドレス
//==============================================================
{
	size_t newunits, units, d;
	Header *p;
	void *newptr;

	if(!size)
	{
		bsFree(header, ptr);					// size == 0 : 領域を開放する
		return NULL;
	}

	p = (Header *)ptr - 1;
	units = p->s.size;
	newunits = (size + sizeof(Header) - 1) / sizeof(Header) + 1;
	d = units - newunits;
	if((d == 0) || (d == 1))
	{
		return ptr;								// ブロック数に変化が無ければ、処理を行わない
	}
	else
	{
		if(p->s.top)	newptr = bsMalloc2(header, size, name);
		else			newptr = bsMalloc(header, size, name);

		if(newptr)
		{
			memcpy(newptr, ptr, size);			// 別の領域が確保された場合、データをコピーする
			bsFree(header, ptr);				// 別の領域が確保された場合のみ、オリジナルを開放
		}
		else
		{
			PRINTF("realloc error.\n");
		}

		return newptr;
	}
}

//==============================================================
static void *bsCalloc(sMemHeader *header, size_t count, size_t size, const char *name)
//--------------------------------------------------------------
// メモリ割り当て
// ※確保したメモリの内容をゼロクリアします
//--------------------------------------------------------------
// in:	header = メモリハンドル
//		count  = 要素数
//		size   = 要素ごとのサイズ
//		name   = 領域名
//--------------------------------------------------------------
// out:	見つかったメモリ領域の先頭アドレス
//==============================================================
{
	void *ptr;

	ptr = bsMalloc(header, count * size, name);
	if(ptr)
	{
		/* メモリの内容をゼロクリア */
		memset(ptr, 0, count * size);
	}

	return ptr;
}

//==============================================================
static void *bsCalloc2(sMemHeader *header, size_t count, size_t size, const char *name)
//--------------------------------------------------------------
// メモリ割り当て
// ※確保したメモリの内容をゼロクリアします
//--------------------------------------------------------------
// in:	header = メモリハンドル
//		count  = 要素数
//		size   = 要素ごとのサイズ
//		name   = 領域名
//--------------------------------------------------------------
// out:	見つかったメモリ領域の先頭アドレス
//==============================================================
{
	void *ptr;

	ptr = bsMalloc2(header, count * size, name);
	if(ptr)
	{
		/* メモリの内容をゼロクリア */
		memset(ptr, 0, count * size);
	}

	return ptr;
}

//==============================================================
int bsMemoryHeaderCheck(Header *p)
//--------------------------------------------------------------
// ヘッダチェック
//--------------------------------------------------------------
// in:	p = メモリハンドル
//--------------------------------------------------------------
// out:	FALSE : メモリ破壊の可能性あり
//==============================================================
{
	return (p->s.header == MEM_HEADER);
}

//==============================================================
void MemInit(void)
//--------------------------------------------------------------
// メモリ管理の初期化
//--------------------------------------------------------------
// in:	なし
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	void *ptr;
	size_t size;
	int i;

	ZEROMEMORY(&gMem, sizeof(sMemInfo));		// 管理領域を初期化

	//----------------------------
	// ヒープ領域を作成して初期化
	//----------------------------
	ptr = OsCreateMemory(MEM_SIZE);

	// 空きエリアを全てグローバル領域として確保
	//------------------------------------------
	bsMemoryInit(&gMem.global_mem_hdr, ptr, MEM_SIZE, TRUE);

	//--------------------
	// MEM_APP 以外を確保
	//--------------------
	for(i = MEM_SYS; i < MEM_APP; ++i)
	{
		ptr = bsMalloc(&gMem.global_mem_hdr, memSizeTbl[i], NULL);
		ASSERT(ptr);
		bsMemoryInit(&gMem.mem_hdr[i], ptr, memSizeTbl[i], FALSE);
		gMem.mem_hdr[i].area = i;
	}

	//------------------------------------------------------
	// グローバル領域の残りをアプリケーション領域として確保
	//------------------------------------------------------
	size = bsMemoryGetFree(&gMem.global_mem_hdr);

	ptr = bsMalloc(&gMem.global_mem_hdr, size, NULL);
	ASSERT(ptr);
	bsMemoryInit(&gMem.mem_hdr[MEM_APP], ptr, size, FALSE);
	gMem.mem_hdr[MEM_APP].area = MEM_APP;

#ifdef DEBUG
	//--------------------
	// 開発時の領域の確保
	//--------------------
	ptr = OsCreateMemory(MEM_DEV_SIZE);
	bsMemoryInit(&gMem.mem_hdr[MEM_DEV], ptr, MEM_DEV_SIZE, TRUE);
	gMem.mem_hdr[MEM_DEV].area = MEM_DEV;
#endif

	SYSINFO(".... memory initialize");
}

//==============================================================
void MemFin(void)
//--------------------------------------------------------------
// メモリ管理の終了
//--------------------------------------------------------------
// in:	なし
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	OsDestroyMemory(gMem.global_mem_hdr.top);
#ifdef DEBUG
	OsDestroyMemory(gMem.mem_hdr[MEM_DEV].top);
#endif

	SYSINFO(".... memory finish");
}

//==============================================================
void *MemMalloc(int area, size_t size, const char *name)
//--------------------------------------------------------------
// 指定領域からメモリを確保
//--------------------------------------------------------------
// in:	area = 領域識別子(enmMEM_AREA)
//		size = 確保するメモリサイズ
//		name = 領域名
//--------------------------------------------------------------
// out:	見つかったメモリ領域の先頭アドレス
//==============================================================
{
	Header *p;

#ifndef DEBUG
	ASSERT(area != MEM_DEV);
	if(area == MEM_DEV)		area = MEM_APP;		/* 暫定 */
#endif

	if(area == MEM_FS)
	{
		// FS の時は APP の上から確保
		p = (Header *)bsMalloc2(&gMem.mem_hdr[MEM_APP], size, name);
	}
	else
	{
		/* それ以外は下から確保 */
		p = (Header *)bsMalloc(&gMem.mem_hdr[area], size, name);
	}

	return p;
}

//==============================================================
void *MemRealloc(int area, void *ptr, size_t size, const char *name)
//--------------------------------------------------------------
// 指定領域を変更
//--------------------------------------------------------------
// in:	area = 領域識別子(enmMEM_AREA)
//		ptr  = 領域(NULL の時は、area 領域から確保)
//		size = 確保するメモリサイズ
//		name = 領域名
//--------------------------------------------------------------
// out:	見つかったメモリ領域の先頭アドレス
//==============================================================
{
	Header *p;

	if(!ptr)
	{
		return MemMalloc(area, size, name);
	}
	else
	{
		p = (Header *)ptr;
		area = (p - 1)->s.area;
		p = (Header *)bsRealloc(&gMem.mem_hdr[area], ptr, size, name);

		return p;
	}
}

//==============================================================
void *MemCalloc(int area, size_t count, size_t size, const char *name)
//--------------------------------------------------------------
// 指定領域からメモリを確保
//--------------------------------------------------------------
// in:	area  = 領域識別子(enmMEM_AREA)
//		count = 要素数
//		size  = 確保するメモリサイズ
//		name  = 領域名
//--------------------------------------------------------------
// out:	見つかったメモリ領域の先頭アドレス
//==============================================================
{
	Header *p;

#ifndef DEBUG
	ASSERT(area != MEM_DEV);

	if(area == MEM_DEV)
		area = MEM_APP;							/* 暫定 */
#endif

	if(area == MEM_FS)
	{
		// FS の時は APP の上から確保
		p = (Header *)bsCalloc2(&gMem.mem_hdr[MEM_APP], count, size, name);
	}
	else
	{
		p = (Header *)bsCalloc(&gMem.mem_hdr[area], count, size, name);
	}

	return p;
}

//==============================================================
void MemFree(void *ptr)
//--------------------------------------------------------------
// メモリを開放
//--------------------------------------------------------------
// in:	ptr  = 開放する領域
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	Header *p;

	ASSERT(ptr);

	p = (Header *)ptr;
	bsFree(&gMem.mem_hdr[(p - 1)->s.area], ptr);
}

//==============================================================
void MemClean(int area, int flag)
//--------------------------------------------------------------
// 指定領域のメモリをすべて開放
//--------------------------------------------------------------
// in:	area = 領域識別子(enmMEM_AREA)
//		flag = TRUE : 領域をランダムクリアする
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
#ifndef DEBUG
	ASSERT(area != MEM_DEV);
	if(area == MEM_DEV)		return;
#endif

	bsMemoryClean(&gMem.mem_hdr[area], flag);
}

//==============================================================
void MemDisp(void)
//--------------------------------------------------------------
// 利用状況を表示する
//--------------------------------------------------------------
// in:	なし
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	sMemHeader *hdr;
	char *name_tbl[] = {
		" SYS",
		" APP",
		" DEV",
	};
	int i;

	PRINTF("\nMemory Info.\n");

	for(i = 0; i < MEM_BLOCK_NUM; ++i)
	{
		hdr = &gMem.mem_hdr[i];
		bsMemoryCheck(hdr);

		PRINTF("  %s: %d / %d(%d%%) min:%d\n", name_tbl[i], hdr->total, hdr->size * sizeof(Header), (hdr->total * 100) / (hdr->size * sizeof(Header)), hdr->total);
	}
	PRINTF("\n");
}

//==============================================================
sMemHeader *MemGetHeader(int area)
//--------------------------------------------------------------
// ヘッダの取得
//--------------------------------------------------------------
// in:	area = enmMEM_AREA
//--------------------------------------------------------------
// out:	ヘッダ
//==============================================================
{
	return &gMem.mem_hdr[area];
}

//==============================================================
int MemGetNumArea(BOOL alloc, sMemHeader *header)
//--------------------------------------------------------------
// 領域数を取得
//--------------------------------------------------------------
// in:	alloc = TRUE:確保領域
//--------------------------------------------------------------
// out:	領域数
//==============================================================
{
	int num;
	Header *p;

	num = 0;
	p = alloc ? header->last : header->freep;
	while(p)
	{
		++num;
		p = p->s.ptr;
	}
	return num;
}

//==============================================================
void MemGetInfo(sMemHeader *header)
//--------------------------------------------------------------
// メモリの使用状況などの取得
//--------------------------------------------------------------
// in:	header = ヘッダ
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	bsMemoryCheck(header);
}

