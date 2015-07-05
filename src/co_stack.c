
//
//	スタックフレームの提供
//

#include "co_stack.h"
#include "co_debug.h"
#include "co_memory.h"


#ifdef DEBUG
//==============================================================
void StkError(int code)
//--------------------------------------------------------------
// スタックフレーム 内部用エラー関数
//--------------------------------------------------------------
// in:	code = エラーコード
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	switch(code)
	{
		case STK_ERR_MANY_FRAMES:
		ASSERTM(0, "too many frames.");
		break;

		case STK_ERR_MANY_DATA:
		ASSERTM(0, "too many data.");
		break;

		case STK_ERR_NO_DATA:
		ASSERTM(0, "no data.");
		break;

		case STK_ERR_NO_FRAME:
		ASSERTM(0, "no frame in stack.");
		break;

		case STK_ERR_INVALID_REF:
		ASSERTM(0, "invalid reference.");
		break;
	}
	ASSERTM(0, "unknown error code.");
}
#endif

//==============================================================
sSTACK *StkCreate(void)
//--------------------------------------------------------------
// スタックフレーム スタックの作成
//--------------------------------------------------------------
// in:	なし
//--------------------------------------------------------------
// out:	スタックポインタ
//==============================================================
{
	sSTACK *stk = (sSTACK *)sysMalloc(sizeof(sSTACK), "sSTACK");
	ASSERT(stk);

	stk->buf = (sSTACKBUF *)sysMalloc(sizeof(sSTACKBUF) * STK_DEPTH, "sSTACKBUF");
	ASSERT(stk->buf);
	stk->buf_depth = STK_DEPTH;
	stk->buf_ptr = 0;

	stk->frm = (int *)sysMalloc(sizeof(int) * STK_FRAME, "Stack int");
	ASSERT(stk->frm);
	stk->frm_depth = STK_FRAME;
	stk->frm_ptr = 0;

	return stk;
}

//==============================================================
void StkKill(sSTACK *stk)
//--------------------------------------------------------------
// スタックフレーム スタックの削除
//--------------------------------------------------------------
// in:	stk	= スタック構造体
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	Free(stk->buf);
	Free(stk->frm);
	Free(stk);
}

void StkBufRealloc(sSTACK *stk)
{
	if(stk->buf_ptr == stk->buf_depth)
	{
		stk->buf_depth *= 2;
		stk->buf = (sSTACKBUF *)sysRealloc(stk->buf, sizeof(sSTACKBUF) * stk->buf_depth, "sSTACKBUF");
		ASSERT(stk->buf);
	}
}

void StkFrmRealloc(sSTACK *stk)
{
	if(stk->frm_ptr == stk->frm_depth)
	{
		stk->frm_depth *= 2;
		stk->frm = (int *)sysRealloc(stk->frm, sizeof(int) * stk->frm_depth, "Stack Int");
		ASSERT(stk->frm);
	}
}
