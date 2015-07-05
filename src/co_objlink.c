//
//汎用リンクリスト
//

#include "co_objlink.h"
#include "co_memory.h"


typedef struct _sLinkHead sLinkHead;
struct _sLinkHead {
	sLinkHead *prev;							// 前のポインタ
	sLinkHead *next;							// 次のポインタ
	BOOL alloc;									// 個別に malloc した
};

struct _sLink {
	int mem;									// メモリ確保エリア
	BOOL new_area;								// メモリが足りなくなったら追加で確保する
	int obj_size;								// データサイズ(ヘッダ込み)
	int size;									// データサイズ(ヘッダ抜き)

	u_char *top;								// 先頭領域
	u_char *use;								// 使用中の領域
	u_char *free;								// 未使用の領域
};


//==============================================================
sLink *ObjLinkCreate(int size, int num, int mem, BOOL new_area)
//--------------------------------------------------------------
// 生成
//--------------------------------------------------------------
// in:	size     = 領域サイズ
//		num      = 領域数
//		mem      = 確保する領域
//		new_area = メモリが足りなくなったら追加で確保する
//--------------------------------------------------------------
// out:	ハンドル
//==============================================================
{
	sLink *link;
	u_char *top;
	int i;
	int obj_size;

	link = (sLink *)MemMalloc(mem, sizeof(sLink), "sLink");
	ASSERT(link);
	ZEROMEMORY(link, sizeof(sLink));

	obj_size = sizeof(sLinkHead) + size;
	top = (u_char *)MemMalloc(mem, obj_size * num, "sLinkBody");
	ASSERT(top);
	ZEROMEMORY(top, obj_size * num);

	link->mem = mem;
	link->new_area = new_area;
	link->top = link->free = top;
	link->obj_size = obj_size;
	link->size = size;

	for(i = 0; i < (num - 1); ++i)
	{
		sLinkHead *head;

		head = (sLinkHead *)top;
		top += obj_size;
		head->next = (sLinkHead *)top;
	}
	((sLinkHead *)top)->next = NULL;

	return link;
}

//==============================================================
void ObjLinkDestroy(sLink *link)
//--------------------------------------------------------------
// 破棄
//--------------------------------------------------------------
// in:	link = ハンドル
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	Free(link->top);
	Free(link);
}

//==============================================================
void *ObjLinkNew(sLink *link)
//--------------------------------------------------------------
// オブジェクトを確保
//--------------------------------------------------------------
// in:	link = ハンドル
//--------------------------------------------------------------
// out:	データポインタ(NULL = 確保失敗)
//==============================================================
{
	u_char *obj;

	obj = link->free;
	if((obj == NULL) && link->new_area)
	{
		sLinkHead *head;

		// 確保できない場合には、適時メモリを確保して利用
		obj = (u_char *)MemMalloc(link->mem, link->obj_size, "sLinkBody");
		if(obj != NULL)
		{
			head = (sLinkHead *)obj;
			head->alloc = TRUE;
		}
	}

	if(obj != NULL)
	{
		sLinkHead *head;
		sLinkHead *prev;

		head = (sLinkHead *)obj;
		if(link->free) link->free = (u_char *)head->next;

		// 使用領域の先頭に追加
		//----------------------
		prev = (sLinkHead *)link->use;
		if(prev) prev->prev = head;
		head->next = prev;
		head->prev = NULL;
		link->use = (u_char *)head;

		// 初期化
		//--------
		obj += sizeof(sLinkHead);
		ZEROMEMORY(obj, link->size);
	}

	return obj;
}

//==============================================================
void ObjLinkDel(sLink *link, void *obj)
//--------------------------------------------------------------
// オブジェクトを削除
//--------------------------------------------------------------
// in:	link = ハンドル
//		obj  = データポインタ
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	sLinkHead *head;

	head = (sLinkHead *)obj - 1;				// ちょっと無茶な記述

	// リンクを繋ぎ換える
	//--------------------
	if(head->prev)
		head->prev->next = head->next;
	if(head->next)
		head->next->prev = head->prev;

	// 先頭データだった場合
	//----------------------
	if(head == (sLinkHead *)link->use)
		link->use = (u_char *)head->next;

	if(head->alloc)
	{
		// 別途確保した領域の場合は開放する
		Free(head);
	}
	else
	{
		// 未使用領域の先頭に追加
		//------------------------
		head->next = (sLinkHead *)link->free;
		link->free = (u_char *)head;
#ifdef DEBUG
		memset(obj, 0xfd, link->size);
#endif
	}
}

//==============================================================
void ObjLinkInsert(sLink *link, void *obj, void *src, BOOL next)
//--------------------------------------------------------------
// オブジェクトをリストに挿入
//--------------------------------------------------------------
// in:	link = ハンドル
//		obj  = 挿入するデータポインタ
//		src  = 挿入元
//		next = TRUE: 前に挿入
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	sLinkHead *head;
	sLinkHead *prev;

	if(obj == src)		return;

	// リンクを繋ぎ換える
	//--------------------
	head = (sLinkHead *)obj - 1;
	if(head->prev)
		head->prev->next = head->next;
	if(head->next)
		head->next->prev = head->prev;

	// 先頭データだった場合
	//----------------------
	if(head == (sLinkHead *)link->use)
		link->use = (u_char *)head->next;

	prev = (sLinkHead *)src - 1;
	if(next)
	{
		head->prev = prev->prev;
		head->next = prev;
		if(head->prev)
			head->prev->next = head;
		prev->prev = head;

		// 先頭データだった場合
		//----------------------
		if(prev == (sLinkHead *)link->use)
			link->use = (u_char *)head;
	}
	else
	{
		head->prev = prev;
		head->next = prev->next;
		if(head->next)
			head->next->prev = head;
		prev->next = head;
	}
}

//==============================================================
void ObjLinkDelAll(sLink *link)
//--------------------------------------------------------------
// 全オブジェクトを削除
//--------------------------------------------------------------
// in:	link = ハンドル
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	void *ptr;

	ptr = ObjLinkGetTop(link);
	while(ptr)
	{
		void *next;

		next = ObjLinkGetNext(ptr);
		ObjLinkDel(link, ptr);
		ptr = next;
	}
}

//==============================================================
void *ObjLinkGetTop(sLink *link)
//--------------------------------------------------------------
// 最初のポインタを取得
// ※最後に登録されたオブジェクトを取得する
//--------------------------------------------------------------
// in:	link = ハンドル
//--------------------------------------------------------------
// out:	最初のポインタ(NULL = なし)
//==============================================================
{
	void *res;

	res = link->use;
	if(res)		res = (void *)(link->use + sizeof(sLinkHead));

	return res;
}

//==============================================================
void *ObjLinkGetLast(sLink *link)
//--------------------------------------------------------------
// 最後のポインタを取得
// ※最初に登録されたオブジェクトを取得する
//--------------------------------------------------------------
// in:	link = ハンドル
//--------------------------------------------------------------
// out:	最後のポインタ(NULL = なし)
//==============================================================
{
	void *p;
	void *res = NULL;

	p = ObjLinkGetTop(link);
	while(p)
	{
		res = p;
		p = ObjLinkGetNext(p);
	}

	return res;
}

//==============================================================
void *ObjLinkGetNext(void *ptr)
//--------------------------------------------------------------
// 次のポインタを取得
//--------------------------------------------------------------
// in:	ptr = データポインタ
//--------------------------------------------------------------
// out:	次のポインタ(NULL = なし)
//==============================================================
{
	sLinkHead *head;
	sLinkHead *p;

	head = (sLinkHead *)ptr - 1;
	p = head->next;
	if(p)
	{
		++p;
	}

	return p;
}

//==============================================================
void *ObjLinkGetPrev(void *ptr)
//--------------------------------------------------------------
// 前のポインタを取得
//--------------------------------------------------------------
// in:	ptr = データポインタ
//--------------------------------------------------------------
// out:	前のポインタ(NULL = なし)
//==============================================================
{
	sLinkHead *head;
	sLinkHead *p;

	head = (sLinkHead *)ptr - 1;
	p = head->prev;
	if(p)
	{
		++p;
	}

	return p;
}

//==============================================================
int ObjLinkGetNum(sLink *link)
//--------------------------------------------------------------
// データ数を取得
//--------------------------------------------------------------
// in:	link = ハンドル
//--------------------------------------------------------------
// out:	データ数
//==============================================================
{
	int num = 0;
	void *p;

	p = ObjLinkGetTop(link);
	while(p)
	{
		++num;
		p = ObjLinkGetNext(p);
	}

	return num;
}
