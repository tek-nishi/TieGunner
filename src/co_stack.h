
// 汎用的に使用するフレームスタック
//
// @note
// RwV3dを要素とするスタック構造です。
// 主にメッセージの引数の受け渡しに使用されます。
//
// このスタック構造では通常のPUSH/POP動作の他に，
// スタックフレームの機構が用意されています。
//
// スタックフレームとは..
// スタックの要素をひとまとめに扱うための機構で，
// フレーム単位での開放や，フレーム先頭からのオフセット位置による参照など，
// スタックの先頭位置を意識せずに値を操作することができます。
//
// ex.)
// - スタック生成 -
// sSTACK *stack = StkCreate();
//
// - フレーム作成 -
// StkMakeFrame();
//
// - 値のプッシュ -
// StkPushVec(&tmpvec1);
// StkPushI(30);
// StkPushF(40.0f);
//
// - フレームオフセットで値を取得 -
// PRINTFV3D(StkRefFrameVec(0));
// PRINTF("stack i:%d\n", StkRefFrameI(2));
// PRINTF("stack f:%f\n", StkRefFrameF(3));
//
// - 値のポップ -
// PRINTF("stack f:%f\n", StkPopF());
// PRINTF("stack i:%d\n", StkPopI());
// vec = StkPopVec();
// PRINTFV3D(vec);
//
// - フレーム削除 -
// StkDelFrame();
//
// - スタック破棄 -
// StkKill(stack);

//==============================================================
#ifndef CO_STACK_H
#define CO_STACK_H
//==============================================================

#include "co_common.h"

#ifdef __cplusplus
extern              "C"
{
#endif

/********************************************/
/*             定数・マクロ宣言             */
/********************************************/
#define STK_FRAME  32				// フレームの最大数
#define STK_DEPTH  256				// スタックの深さ

#ifdef DEBUG
	// 内部用エラーコード
	enum {
		STK_ERR_MANY_FRAMES,		// フレームが多すぎ
		STK_ERR_MANY_DATA,			// データが多すぎ
		STK_ERR_NO_DATA,			// データが無い
		STK_ERR_NO_FRAME,			// フレームがない
		STK_ERR_INVALID_REF			// 無効の参照
	};

	// 内部用エラー関数
	extern void StkError(RwInt32 code);
	// 内部用アサーションマクロ
	#define STK_ASSERT(e, c) if(!(e))	StkError(c)
#else
	#define STK_ASSERT(e, c)
#endif

#define StkMakeFrame()		_StkMakeFrame((sSTACK *)g.msgarg_stack)
#define StkDelFrame()		_StkDelFrame((sSTACK *)g.msgarg_stack)
#define StkFrameLen()		_StkFrameLen((sSTACK *)g.msgarg_stack)
#define StkRefFrame(ofs)	_StkRefFrame((sSTACK *)g.msgarg_stack, ofs)
#define StkPush(val)		_StkPush((sSTACK *)g.msgarg_stack, val)
#define StkPop()			_StkPop((sSTACK *)g.msgarg_stack)
#define StkPushF(val)		_StkPushF((sSTACK *)g.msgarg_stack, val)
#define StkPopF()			_StkPopF((sSTACK *)g.msgarg_stack)
#define StkPushI(val)		_StkPushI((sSTACK *)g.msgarg_stack, val)
#define StkPopI()			_StkPopI((sSTACK *)g.msgarg_stack)
#define StkPushVec(val)		_StkPushVec((sSTACK *)g.msgarg_stack, val)
#define StkPopVec()			_StkPopVec((sSTACK *)g.msgarg_stack)
#define StkPushP(val)		_StkPushP((sSTACK *)g.msgarg_stack, val)
#define StkPopP()			_StkPopP((sSTACK *)g.msgarg_stack)
#define StkRefFrameF(ofs)	_StkRefFrameF((sSTACK *)g.msgarg_stack, ofs)
#define StkRefFrameI(ofs)	_StkRefFrameI((sSTACK *)g.msgarg_stack, ofs)
#define StkRefFrameVec(ofs)	_StkRefFrameVec((sSTACK *)g.msgarg_stack, ofs)
#define StkRefFrameP(ofs)	_StkRefFrameP((sSTACK *)g.msgarg_stack, ofs)


/********************************************/
/*                構造体宣言                */
/********************************************/
typedef union _sSTACKBUF sSTACKBUF;
union _sSTACKBUF {
	int i;
	float f;
	IVector2 vec;
	void *p;
};

// スタック構造体
typedef struct {
	sSTACKBUF *buf;									// バッファ
	int buf_depth;
	int	buf_ptr;									// バッファポインタ
	int	*frm;										// フレーム
	int frm_depth;
	int	frm_ptr;									// フレームポインタ
} sSTACK;


/********************************************/
/*              グローバル変数              */
/********************************************/


/********************************************/
/*              グローバル関数              */
/********************************************/
extern sSTACK *StkCreate(void);
extern void StkKill(sSTACK *stk);
extern void StkBufRealloc(sSTACK *stk);
extern void StkFrmRealloc(sSTACK *stk);


//--------------------------------------------
// インライン関数群
//--------------------------------------------

// --------------------------------------------------------------
// フレーム作成、削除
// --------------------------------------------------------------
//===============================================================
static void _StkMakeFrame(sSTACK *stk)
//--------------------------------------------------------------
// スタックフレームの作成
// 	フレームを作成し，先頭に追加します
// --------------------------------------------------------------
// @param	stk	: スタック構造体
//===============================================================
{
//	STK_ASSERT(stk->frm_ptr<STK_FRAME, STK_ERR_MANY_FRAMES);
	StkFrmRealloc(stk);
	stk->frm[stk->frm_ptr] = stk->buf_ptr;
	stk->frm_ptr += 1;
}

//===============================================================
static void _StkDelFrame(sSTACK *stk)
//--------------------------------------------------------------
// スタックフレームの削除
// --------------------------------------------------------------
// @param	stk	: スタック構造体
//===============================================================
{
	STK_ASSERT(stk->frm_ptr > 0, STK_ERR_NO_FRAME);
	stk->buf_ptr = stk->frm[--stk->frm_ptr];
}

//===============================================================
static RwInt32 _StkFrameLen(sSTACK *stk)
//--------------------------------------------------------------
// スタックフレーム フレーム長の取得
// --------------------------------------------------------------
// @param	stk	: スタック構造体
// --------------------------------------------------------------
// @return	フレーム長
//===============================================================
{
	return stk->buf_ptr - stk->frm[stk->frm_ptr - 1];
}


// --------------------------------------------------------------
// 支援系
// --------------------------------------------------------------
//===============================================================
static sSTACKBUF *_StkRefFrame(sSTACK *stk, int ofs)
//--------------------------------------------------------------
// スタックフレーム フレーム内参照
// 	先頭フレーム内のオフセット位置で値を参照します
// --------------------------------------------------------------
// @param	stk	: スタック構造体
// @param	ofs	: オフセット
// --------------------------------------------------------------
// @return	参照データポインタ
//===============================================================
{
	ofs += stk->frm[stk->frm_ptr - 1];
	STK_ASSERT(ofs<stk->buf_ptr, STK_ERR_INVALID_REF);
	return &stk->buf[ofs];
}

//===============================================================
static void _StkPush(sSTACK *stk, sSTACKBUF *val)
//--------------------------------------------------------------
// スタックフレーム 値のPUSH
// --------------------------------------------------------------
// @param	stk	: スタック構造体
//===============================================================
{
//	STK_ASSERT(stk->buf_ptr < STK_DEPTH, STK_ERR_MANY_DATA);
	StkBufRealloc(stk);
	stk->buf[stk->buf_ptr] = *val;
	stk->buf_ptr += 1;
}

//===============================================================
static sSTACKBUF *_StkPop(sSTACK *stk)
//--------------------------------------------------------------
// スタックフレーム 値のPOP
// --------------------------------------------------------------
// @param	stk	: スタック構造体
// --------------------------------------------------------------
// @return	値
//===============================================================
{
	STK_ASSERT(stk->buf_ptr > 0, STK_ERR_NO_DATA);
	return &stk->buf[--stk->buf_ptr];
}


// -------------------------------------------------------------
// PUSH & POP
// -------------------------------------------------------------
// PUSH (float)
static void _StkPushF(sSTACK *stk, float val)
{
	sSTACKBUF buf;

	buf.f = val;
	_StkPush(stk, &buf);
}

// POP (float)
static float _StkPopF(sSTACK *stk)
{
	return _StkPop(stk)->f;
}

// -------------------------------------------------------------
// PUSH (int)
static void _StkPushI(sSTACK *stk, int val)
{
	sSTACKBUF buf;

	buf.i = val;
	_StkPush(stk, &buf);
}

// POP (int)
static int _StkPopI(sSTACK *stk)
{
	return _StkPop(stk)->i;
}

// -------------------------------------------------------------
// PUSH (IVector2)
static void _StkPushVec(sSTACK *stk, IVector2 *val)
{
	sSTACKBUF buf;

	buf.vec = *val;
	_StkPush(stk, &buf);
}

// POP (IVector2)
static IVector2 *_StkPopVec(sSTACK *stk)
{
#if 1
	return &(_StkPop(stk)->vec);
#else
	sSTACKBUF *buf = _StkPop(stk);

	return &buf->vec;
#endif
}

// -------------------------------------------------------------
// PUSH (pointer)
static void _StkPushP(sSTACK *stk, void *val)
{
	sSTACKBUF buf;

	buf.p = val;
	_StkPush(stk, &buf);
}

// POP (pointer)
static void *_StkPopP(sSTACK *stk)
{
	return _StkPop(stk)->p;
}


// -------------------------------------------------------------
// 参照
// -------------------------------------------------------------
// REF (float)
static float _StkRefFrameF(sSTACK *stk, int ofs)
{
	return _StkRefFrame(stk, ofs)->f;
}

// -------------------------------------------------------------
// REF (int)
static int _StkRefFrameI(sSTACK *stk, int ofs)
{
	return _StkRefFrame(stk, ofs)->i;
}

// -------------------------------------------------------------
// REF (IVector2)
static IVector2 *_StkRefFrameVec(sSTACK *stk, int ofs)
{
#if 1
	return &(_StkRefFrame(stk, ofs)->vec);
#else
	sSTACKBUF *buf = _StkRefFrame(stk, ofs);
	return &buf->vec;
#endif
}

// -------------------------------------------------------------
// REF (pointer)
static void *_StkRefFrameP(sSTACK *stk, int ofs)
{
	return _StkRefFrame(stk, ofs)->p;
}


#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================

