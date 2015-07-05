//
//	汎用オブジェクトリンク
//

//==============================================================
#ifndef CO_OBJLINK_H
#define CO_OBJLINK_H
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
typedef struct _sLink sLink;


/********************************************/
/*              グローバル変数              */
/********************************************/


/********************************************/
/*              グローバル関数              */
/********************************************/
// 生成
extern sLink *ObjLinkCreate(int size, int num, int mem, BOOL new_area);
// 破棄
extern void ObjLinkDestroy(sLink *link);

// オブジェクトを確保
extern void *ObjLinkNew(sLink *link);
// オブジェクトを削除
extern void ObjLinkDel(sLink *link, void *obj);
// 全オブジェクトを削除
extern void ObjLinkDelAll(sLink *link);
// オブジェクトをリストに挿入
extern void ObjLinkInsert(sLink *link, void *obj, void *src, BOOL next);
// 最初のポインタを取得
extern void *ObjLinkGetTop(sLink *link);
// 最後のポインタを取得
extern void *ObjLinkGetLast(sLink *link);
// 次のポインタを取得
extern void *ObjLinkGetNext(void *ptr);
// 前のポインタを取得
extern void *ObjLinkGetPrev(void *ptr);
// データ数を取得
extern int ObjLinkGetNum(sLink *link);


#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================

