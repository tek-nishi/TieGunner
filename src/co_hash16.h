//
//	キー文字列 15bytes 限定のハッシュ関数
//

//==============================================================
#ifndef CO_HASH16_H
#define CO_HASH16_H
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
typedef struct _sHASH_KEY sHASH_KEY;
typedef struct _sHASH	  sHASH;


/********************************************/
/*              グローバル変数              */
/********************************************/


/********************************************/
/*              グローバル関数              */
/********************************************/
// 生成
extern sHASH *HashCreate(char *id_str);
// 破棄
extern void HashKill(sHASH *hash);
// 消去
extern void HashCleanup(sHASH *hash);

// 登録
extern void HashAdd(sHASH *hash, char *id_str, void *ptr);
// 取得
extern void *HashGet(sHASH *hash, char *id_str);
// 削除
extern void HashDel(sHASH *hash, char *id_str);

// 登録数を取得
extern int HashGetKeyNum(sHASH *hash);
// 衝突数を取得
extern int HashGetCollision(sHASH *hash);

// 登録リストを作成
extern sHASH_KEY **HashGetKeyList(sHASH *hash);
// キーのIDを取得
extern char *HashGetKeyId(sHASH_KEY *key);
// キーの登録内容を取得
extern void *HashGetKeyValue(sHASH_KEY *key);


#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================

