//
// 汎用オブジェクト処理
//


#include "co_obj.h"
#include "co_objlink.h"
#include "co_hash16.h"
#include "co_memory.h"
#include "co_task.h"
#include "co_strings.h"
#include "co_misc.h"


#define OBJ_PARAM_NUM  64							// パラメータ管理数
#define OBJ_NUM        1024							// データ生成数
#define OBJ_FILE_PATH  PATH_DATA"/object"			// データ読み込みパス


struct _sOBJ {
	u_int type;									// フィルタリング用タイプ
	char id_str[ID_MAXLEN];						// 識別子
	sParam *param;								// パラメータ
	OBJ_PROC func;								// コールバック
	BOOL create;								// TRUE = 生成フレーム
	BOOL kill_req;								// TRUE = 削除予約
	void *var;									// 汎用ワークエリア

	FVector2 pos;								// 位置
	REAL dir;									// 向き
	FVector2 vct;
	REAL radius;
	BOOL death;
};


static sHASH *param_hash;							// パラメータ管理用
static sLink *obj_link;								// データ領域


//==============================================================
static sParam *objGetParam(char *id_str)
//--------------------------------------------------------------
// パラメータを取得
//--------------------------------------------------------------
// in:	id_str = 識別子
//--------------------------------------------------------------
// out:	データポインタ(NULL = データ無し)
//==============================================================
{
	return (sParam *)HashGet(param_hash, id_str);
}

//==============================================================
static void objDelete(sOBJ *obj)
//--------------------------------------------------------------
// オブジェクトを削除
//--------------------------------------------------------------
// in:	obj = ハンドル
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	obj->kill_req = FALSE;
	ObjPostMsg(obj, MSG_KILL, 0, 0);

	FreeWork(obj->var);
	ObjLinkDel(obj_link, obj);
}

//==============================================================
static void objDeleteAll(u_int type, char *id_str)
//--------------------------------------------------------------
// オブジェクトを全て削除
//--------------------------------------------------------------
// in:	type   = フィルタリング
//		id_str = 識別名
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	sOBJ *obj;

	obj = ObjGetNext(NULL, type, id_str);
	while(obj)
	{
		sOBJ *next;

		next = ObjGetNext(obj, type, id_str);
		objDelete(obj);
		obj = next;
	}
}

//==============================================================
static int objTask(sTaskBody *body, int msg, int lParam, int rParam)
//--------------------------------------------------------------
// タスク処理
//--------------------------------------------------------------
// in:	body           = ハンドル
//		msg            = メッセージ
//		lParam, rParam = 汎用パラメータ
//--------------------------------------------------------------
// out:	実行結果
//==============================================================
{
	int res = 0;

	switch(msg)
	{
		case MSG_CREATE:
		{
			param_hash = HashCreate("obj");
			obj_link = ObjLinkCreate(sizeof(sOBJ), OBJ_NUM, MEM_SYS, FALSE);

			SYSINFO(".... object initialize");
		}
		break;

		case MSG_KILL:
		{
			ObjDeleteParamAll();

			HashKill(param_hash);
			ObjLinkDestroy(obj_link);

			SYSINFO(".... object finish");
		}
		break;

		case MSG_PREPROC:
		{
			{
				sOBJ *obj;

				obj = ObjGetNext(NULL, OBJ_TYPE_ALL, NULL);
				while(obj)
				{
					obj->create = FALSE;
					obj = ObjGetNext(obj, OBJ_TYPE_ALL, NULL);
				}
			}
			ObjPostMsgAll(OBJ_TYPE_ALL, msg, FALSE, 0, 0);
		}
		break;

		case MSG_STEP:
		{
			ObjPostMsgAll(OBJ_TYPE_ALL, msg, FALSE, 0, 0);
			{
				sOBJ *obj;

				obj = ObjGetNext(NULL, OBJ_TYPE_ALL, NULL);
				while(obj)
				{
					sOBJ *next;

					next = ObjGetNext(obj, OBJ_TYPE_ALL, NULL);
					if(obj->kill_req)	objDelete(obj);
					obj = next;
				}
			}
		}
		break;

		case MSG_UPDATE:
		{
			ObjPostMsgAll(OBJ_TYPE_ALL, msg, FALSE, 0, 0);
		}
		break;

		case MSG_DRAW:
		{
			ObjPostMsgAll(OBJ_TYPE_ALL, msg, FALSE, 0, 0);
		}
		break;

		default:
		{
			ObjPostMsgAll(OBJ_TYPE_ALL, msg, FALSE, lParam, rParam);
		}
		break;
	}

	return res;
}

//==============================================================
void ObjInit(void)
//--------------------------------------------------------------
// 初期化
//--------------------------------------------------------------
// in:	なし
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	TaskCreate("sys_obj", TASK_PRI_SYS, objTask, 0, 0);
}

//==============================================================
sParam *ObjSetup(char *id_str)
//--------------------------------------------------------------
// オブジェクトのセットアップ
// ※同じ名前が存在する場合、読み込み直す
//--------------------------------------------------------------
// in:	id_str = 識別子
//--------------------------------------------------------------
// out:	読み込んだパラメータ
//==============================================================
{
	char file[FNAME_MAXLEN];
	sParam *param;

	param = (sParam *)HashGet(param_hash, id_str);
	if(param)	ParamDestroy(param);

	sprintf(file, OBJ_FILE_PATH"/%s.param", id_str);
	param = ParamRead(file);
	HashAdd(param_hash, id_str, param);

	return param;
}

//==============================================================
sParam *ObjGetSetupParam(char *id_str)
//--------------------------------------------------------------
// セットアップ済みのパラメータを取得
//--------------------------------------------------------------
// in:	id_str = ハンドル
//--------------------------------------------------------------
// out:	パラメータ
//==============================================================
{
	return objGetParam(id_str);
}

//==============================================================
void ObjDeleteParam(char *id_str)
//--------------------------------------------------------------
// セットアップしたデータの破棄
//--------------------------------------------------------------
// in:	id_str = 識別子
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	sParam *param;

	param = objGetParam(id_str);
	ASSERT(param);

	// 先にオブジェクトを破棄
	objDeleteAll(OBJ_TYPE_ALL, id_str);

	ParamDestroy(param);
	HashDel(param_hash, id_str);
}

//==============================================================
void ObjDeleteParamAll(void)
//--------------------------------------------------------------
// セットアップした全データの破棄
//--------------------------------------------------------------
// in:	なし
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	sHASH_KEY **list;
	int ofs;

	// 先にオブジェクトを破棄
	objDeleteAll(OBJ_TYPE_ALL, NULL);

	// 全パラメータ削除
	list = HashGetKeyList(param_hash);
	ofs = 0;
	while(list[ofs] != NULL)
	{
		ObjDeleteParam(HashGetKeyId(list[ofs]));
		ofs += 1;
	}
	Free(list);

	HashCleanup(param_hash);
}

void ObjDeleteAll(u_int type)
{
	objDeleteAll(type, NULL);
}

//==============================================================
sOBJ *ObjGetNext(sOBJ *prev, u_int type, char *id_str)
//--------------------------------------------------------------
// オブジェクト取得
//--------------------------------------------------------------
// in:	prev   = ハンドル(NULL = 先頭から)
//		type   = フィルタリング
//		id_str = フィルタリング名
//--------------------------------------------------------------
// out:	次のハンドル(NULL = データ無し)
//==============================================================
{
	sOBJ *obj;
	BOOL res;

	obj = prev;
	do
	{
		obj = obj ? (sOBJ *)ObjLinkGetNext(obj) : (sOBJ *)ObjLinkGetTop(obj_link);
		res = obj ? obj->type & type : FALSE;
		if(res && id_str)
		{
			res = !strcmp(id_str, obj->id_str) ? TRUE : FALSE;
		}
	}
	while(obj && !res);

	return obj;
}

//==============================================================
int ObjPostMsg(sOBJ *obj, int msg, int lParam, int rParam)
//--------------------------------------------------------------
// メッセージ送信
//--------------------------------------------------------------
// in:	obj    = ハンドル
//		msg    = メッセージ
//		lParam = パラメータ１
//		rParam = パラメータ２
//--------------------------------------------------------------
// out:	実行結果
//==============================================================
{
	int res = 0;

	if(!obj->kill_req)
	{
		res = obj->func(obj, obj->param, msg, lParam, rParam);
	}

	return res;
}

//==============================================================
BOOL ObjPostMsgAll(u_int type, int msg, BOOL abort, int lParam, int rParam)
//--------------------------------------------------------------
// 全オブジェクトへメッセージ送信
//--------------------------------------------------------------
// in:	type   = フィルタリング
//		msg    = メッセージ
//		abort  = TRUE: 0 以外が返されたら、途中で中断
//		lParam = パラメータ１
//		rParam = パラメータ２
//--------------------------------------------------------------
// out:	TRUE = 中断された
//==============================================================
{
	BOOL res = FALSE;
	sOBJ *obj;

	obj = ObjGetNext(NULL, type, NULL);
	while(obj)
	{
		if(!obj->create)
		{
			int result;

			result = ObjPostMsg(obj, msg, lParam, rParam);
			if(abort && result)
			{
				res = TRUE;
				break;
			}
		}
		obj = ObjGetNext(obj, type, NULL);
	}

	return res;
}

//==============================================================
sOBJ *ObjCreate(char *id_str, u_int type, OBJ_PROC func, int lParam, int rParam)
//--------------------------------------------------------------
// オブジェクト生成
//--------------------------------------------------------------
// in:	id_str = オブジェクト識別子
//		type   = フィルタリング
//		func   = コールバック
//		lParam = パラメータ１
//		rParam = パラメータ２
//--------------------------------------------------------------
// out:	ハンドル
//==============================================================
{
	sOBJ *obj;

	obj = (sOBJ *)ObjLinkNew(obj_link);
	ASSERT(obj);
	obj->param = objGetParam(id_str);
	ASSERT(obj->param);
	obj->type = type;
	STRCPY16(obj->id_str, id_str);
	obj->func = func;

	ObjPostMsg(obj, MSG_CREATE, lParam, rParam);
	obj->create = TRUE;							// 正しく頭から処理を開始できるようにする

	return obj;
}

//==============================================================
void ObjKillReq(sOBJ *obj)
//--------------------------------------------------------------
// オブジェクト削除要求
//--------------------------------------------------------------
// in:	obj = ハンドル
//--------------------------------------------------------------
// out:	ハンドル
//==============================================================
{
	obj->kill_req = TRUE;
}

//==============================================================
void *ObjGetVar(sOBJ *obj, int size)
//--------------------------------------------------------------
// ワーク取得
//--------------------------------------------------------------
// in:	obj  = ハンドル
//		size = データサイズ
//--------------------------------------------------------------
// out:	取得したポインタ
//==============================================================
{
	if(obj->var == NULL)
	{
		obj->var = GetWork(size, "obj_var");
	}
	return obj->var;
}

//==============================================================
sParam *ObjGetParam(sOBJ *obj)
//--------------------------------------------------------------
// パラメータ取得
//--------------------------------------------------------------
// in:	obj = ハンドル
//--------------------------------------------------------------
// out:	パラメータ
//==============================================================
{
	return obj->param;
}

//==============================================================
u_int ObjGetType(sOBJ *obj)
//--------------------------------------------------------------
// タイプ取得
//--------------------------------------------------------------
// in:	obj = ハンドル
//--------------------------------------------------------------
// out:	タイプ
//==============================================================
{
	return obj->type;
}

//==============================================================
void ObjSetType(sOBJ *obj, u_int type)
//--------------------------------------------------------------
// タイプ変更
//--------------------------------------------------------------
// in:	obj  = ハンドル
//		type = タイプ
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	obj->type = type;
}

//==============================================================
FVector2 *ObjGetPos(sOBJ *obj)
//--------------------------------------------------------------
// 位置を取得
//--------------------------------------------------------------
// in:	obj = ハンドル
//--------------------------------------------------------------
// out:	位置
//==============================================================
{
	return &obj->pos;
}

//==============================================================
void ObjSetPos(sOBJ *obj, REAL x, REAL y)
//--------------------------------------------------------------
// 位置を変更
//--------------------------------------------------------------
// in:	obj  = ハンドル
//		x, y = 位置
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	SetV2d(&obj->pos, x, y);
}

REAL ObjGetDir(sOBJ *obj)
{
	return obj->dir;
}

void ObjSetDir(sOBJ *obj, REAL dir)
{
	obj->dir = dir;
}

FVector2 *ObjGetVct(sOBJ *obj)
{
	return &obj->vct;
}

void ObjSetVct(sOBJ *obj, REAL x, REAL y)
{
	SetV2d(&obj->vct, x, y);
}

REAL ObjGetRadius(sOBJ *obj)
{
	return obj->radius;
}

void ObjSetRadius(sOBJ *obj, REAL radius)
{
	obj->radius = radius;
}

BOOL ObjIsDead(sOBJ *obj)
{
	return obj->death;
}

void ObjSetDeath(sOBJ *obj, BOOL death)
{
	obj->death = death;
}
