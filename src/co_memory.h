
/*

	メモリ管理処理を独自に行います。
	「K & R プログラミング言語Ｃ」の 「8.7記憶割り当て」を参考にしています。

	・確保される領域は、64bytes 単位でアラインされます(変更可能)。ただし、ヘッダ
	  のサイズの都合上、最低アラインサイズは 64bytes です。

	・領域開放は一括して Free() を使います
	・hogeClean() は、領域を再初期化する関数です。

*/


//==============================================================
#ifndef CO_MEMORY_H
#define CO_MEMORY_H
//==============================================================

#include "co_common.h"
#include "co_debug.h"

#ifdef __cplusplus
extern              "C"
{
#endif


/********************************************/
/*             定数・マクロ宣言             */
/********************************************/
//--------------------------------------------------
// 領域宣言
// ※領域の並び順を変更したら、ソースも修正する事。
//--------------------------------------------------
enum enmMEM_AREA {
	MEM_NONE = -1,
	MEM_SYS = 0,
	MEM_APP,

	MEM_DEV,

	//--------
	MEM_BLOCK_NUM,								// メモリブロックの総数

	MEM_FS										// MEM_FS を廃止したので、ここに列挙(互換用)
};

//--------------------------------
// 基本的に標準 malloc は使用禁止
//--------------------------------
#define malloc( size )				MemDummyMalloc()		// 中身はエラー関数
#define memalign( align, size )		MemDummyMalloc()		// 中身はエラー関数
#define free( x )					MemDummyFree()			// 中身はエラー関数

//------------------------------
// 各関数のラップ
//------------------------------
#define sysMalloc(size, name)			MemMalloc(MEM_SYS, size, name)
#define sysRealloc(ptr, size, name)		MemRealloc(MEM_SYS, ptr, size, name)
#define sysCalloc(count, size, name)	MemCalloc(MEM_SYS, count, size, name)
#define sysClean(flag)  				bsMemoryClean(&gMem.mem_hdr[MEM_SYS], flag)

#define fsMalloc(size, name)			MemMalloc(MEM_FS, size, name)
#define fsRealloc(ptr, size, name)		MemRealloc(MEM_FS, ptr, size, name)
#define fsCalloc(size, nobj, name)		MemCalloc(MEM_FS, size, nobj, name)
#define fsClean(flag)

#define heapMalloc(size, name)			MemMalloc(MEM_FS, size, name)
#define heapRealloc(ptr, size, name)	MemRealloc(MEM_FS, ptr, size, name)
#define heapCalloc(size, nobj, name)	MemCalloc(MEM_FS, size, nobj, name)
#define heapClean(flag)

#define appMalloc(size, name)			MemMalloc(MEM_APP, size, name)
#define appRealloc(ptr, size, name)		MemRealloc(MEM_APP, ptr, size, name)
#define appCalloc(count, size, name)	MemCalloc(MEM_APP, count, size, name)
#define appClean(flag)  				bsMemoryClean(&gMem.mem_hdr[MEM_APP], flag)

#define devMalloc(size, name)			MemMalloc(MEM_DEV, size, name)
#define devRealloc(ptr, size, name)		MemRealloc(MEM_DEV, ptr, size, name)
#define devCalloc(count, size, name)	MemCalloc(MEM_DEV, count, size, name)
#define devClean(flag)  				bsMemoryClean(&gMem.mem_hdr[MEM_DEV], flag)

#define Free(ptr)						MemFree(ptr)

#if defined DEBUG
	#define DEBUG_MEMORY
#endif


/********************************************/
/*                構造体宣言                */
/********************************************/
union uHeader {
	struct {
		int header;								// ヘッダ(データ破壊チェック用)
		int magicNumber;						// メモリ確保時のマジックナンバー
		int area;								// 確保した場所
		size_t size;							// このブロックの大きさ(単位 : sizeof(uHeader) ヘッダも含む)
		union uHeader *ptr;						// 次の空きブロックへのポインタ
		union uHeader *prev;					// 前の空きブロックへのポインタ
		int top;								// TRUE = メモリの上部から確保した
		char name[ID_MAXLEN];					// メモリブロック名
	} s;
	u_int x[16];								// ブロックの整合を強制(64)
};
typedef union uHeader Header;

typedef struct {
	Header *freep;								// 空き領域リスト
	Header *last;								// 使用領域リスト

	void *top;									// 使用している領域
	size_t size;								// 領域サイズ(ブロック数)
	int area;									// 領域番号
	u_int allocate, total;						// 一度で確保可能な最大サイズ、空き領域の合計
	u_int min_free;								// 空き領域の合計が一番少ない時の値
	int fragment;								// 断片化数
} sMemHeader;

typedef struct {
	sMemHeader global_mem_hdr;					// グローバルメモリ領域
	sMemHeader mem_hdr[MEM_BLOCK_NUM];			// システムで使用するメモリ領域
} sMemInfo;


/********************************************/
/*              グローバル変数              */
/********************************************/


/********************************************/
/*              グローバル関数              */
/********************************************/
//	gnu malloc を使用した場合にエラーを出す
extern void *MemDummyMalloc(void);
//	gnu free を使用した場合にエラーを出す
extern void MemDummyFree(void);

//	メモリ管理初期化
extern void MemInit(void);
//	メモリ管理終了
extern void MemFin(void);
//	指定領域からメモリを確保
extern void *MemMalloc(RwInt32 mode, size_t size, const char *name);
//	指定領域を変更
extern void *MemRealloc(RwInt32 area, void *ptr, size_t size, const char *name);
//	指定領域からメモリを確保
extern void *MemCalloc(RwInt32 area, size_t count, size_t size, const char *name);
//	メモリを開放
extern void MemFree(void *ptr);
//	指定領域のメモリをすべて開放
extern void MemClean(RwInt32 mode, RwBool flag);

//	sysMalloc で取得されるメモリ領域の名前を変更する
extern void MemSetSysName(const char *name);
//	sysMalloc で取得されるメモリ領域のエリアを変更する
extern void MemSetSysArea(RwInt32 area);
//	sysMalloc で設定されるメモリ領域の名前を取得する
extern char *MemGetSysName(void);
//	sysMalloc で設定されるメモリ領域のエリアを取得する
extern RwInt32 MemGetSysArea(void);
//	sysMalloc で取得されるメモリ領域のエリアと名前をまとめて設定
extern void MemPushSysArea(RwInt32 area, char *name);
//	sysMalloc で取得されるメモリ領域のエリアと名前を復帰
extern void MemPopSysArea(void);

//	メモリの断片化などをチェック
extern void bsMemoryCheck(sMemHeader *header);
//	ヘッダチェック
extern RwBool bsMemoryHeaderCheck(Header *p);

// 利用状況を表示する
extern void MemDisp(void);
// メモリヘッダの取得
extern sMemHeader *MemGetHeader(int area);
// 領域数を取得
extern int MemGetNumArea(BOOL alloc, sMemHeader *header);
// メモリの使用状況などの取得
extern void MemGetInfo(sMemHeader *header);


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// インライン関数群
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

static void *GetWork(int size, char *name)
{
	void *p;

	p = appMalloc(size, name);
	ASSERT(p);
	ZEROMEMORY(p, size);

	return p;
}

static void FreeWork(void *ptr)
{
	if(ptr)		Free(ptr);
}


#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================

