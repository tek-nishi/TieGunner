
// �ėp�I�Ɏg�p����t���[���X�^�b�N
//
// @note
// RwV3d��v�f�Ƃ���X�^�b�N�\���ł��B
// ��Ƀ��b�Z�[�W�̈����̎󂯓n���Ɏg�p����܂��B
//
// ���̃X�^�b�N�\���ł͒ʏ��PUSH/POP����̑��ɁC
// �X�^�b�N�t���[���̋@�\���p�ӂ���Ă��܂��B
//
// �X�^�b�N�t���[���Ƃ�..
// �X�^�b�N�̗v�f���ЂƂ܂Ƃ߂Ɉ������߂̋@�\�ŁC
// �t���[���P�ʂł̊J����C�t���[���擪����̃I�t�Z�b�g�ʒu�ɂ��Q�ƂȂǁC
// �X�^�b�N�̐擪�ʒu���ӎ������ɒl�𑀍삷�邱�Ƃ��ł��܂��B
//
// ex.)
// - �X�^�b�N���� -
// sSTACK *stack = StkCreate();
//
// - �t���[���쐬 -
// StkMakeFrame();
//
// - �l�̃v�b�V�� -
// StkPushVec(&tmpvec1);
// StkPushI(30);
// StkPushF(40.0f);
//
// - �t���[���I�t�Z�b�g�Œl���擾 -
// PRINTFV3D(StkRefFrameVec(0));
// PRINTF("stack i:%d\n", StkRefFrameI(2));
// PRINTF("stack f:%f\n", StkRefFrameF(3));
//
// - �l�̃|�b�v -
// PRINTF("stack f:%f\n", StkPopF());
// PRINTF("stack i:%d\n", StkPopI());
// vec = StkPopVec();
// PRINTFV3D(vec);
//
// - �t���[���폜 -
// StkDelFrame();
//
// - �X�^�b�N�j�� -
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
/*             �萔�E�}�N���錾             */
/********************************************/
#define STK_FRAME  32				// �t���[���̍ő吔
#define STK_DEPTH  256				// �X�^�b�N�̐[��

#ifdef DEBUG
	// �����p�G���[�R�[�h
	enum {
		STK_ERR_MANY_FRAMES,		// �t���[����������
		STK_ERR_MANY_DATA,			// �f�[�^��������
		STK_ERR_NO_DATA,			// �f�[�^������
		STK_ERR_NO_FRAME,			// �t���[�����Ȃ�
		STK_ERR_INVALID_REF			// �����̎Q��
	};

	// �����p�G���[�֐�
	extern void StkError(RwInt32 code);
	// �����p�A�T�[�V�����}�N��
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
/*                �\���̐錾                */
/********************************************/
typedef union _sSTACKBUF sSTACKBUF;
union _sSTACKBUF {
	int i;
	float f;
	IVector2 vec;
	void *p;
};

// �X�^�b�N�\����
typedef struct {
	sSTACKBUF *buf;									// �o�b�t�@
	int buf_depth;
	int	buf_ptr;									// �o�b�t�@�|�C���^
	int	*frm;										// �t���[��
	int frm_depth;
	int	frm_ptr;									// �t���[���|�C���^
} sSTACK;


/********************************************/
/*              �O���[�o���ϐ�              */
/********************************************/


/********************************************/
/*              �O���[�o���֐�              */
/********************************************/
extern sSTACK *StkCreate(void);
extern void StkKill(sSTACK *stk);
extern void StkBufRealloc(sSTACK *stk);
extern void StkFrmRealloc(sSTACK *stk);


//--------------------------------------------
// �C�����C���֐��Q
//--------------------------------------------

// --------------------------------------------------------------
// �t���[���쐬�A�폜
// --------------------------------------------------------------
//===============================================================
static void _StkMakeFrame(sSTACK *stk)
//--------------------------------------------------------------
// �X�^�b�N�t���[���̍쐬
// 	�t���[�����쐬���C�擪�ɒǉ����܂�
// --------------------------------------------------------------
// @param	stk	: �X�^�b�N�\����
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
// �X�^�b�N�t���[���̍폜
// --------------------------------------------------------------
// @param	stk	: �X�^�b�N�\����
//===============================================================
{
	STK_ASSERT(stk->frm_ptr > 0, STK_ERR_NO_FRAME);
	stk->buf_ptr = stk->frm[--stk->frm_ptr];
}

//===============================================================
static RwInt32 _StkFrameLen(sSTACK *stk)
//--------------------------------------------------------------
// �X�^�b�N�t���[�� �t���[�����̎擾
// --------------------------------------------------------------
// @param	stk	: �X�^�b�N�\����
// --------------------------------------------------------------
// @return	�t���[����
//===============================================================
{
	return stk->buf_ptr - stk->frm[stk->frm_ptr - 1];
}


// --------------------------------------------------------------
// �x���n
// --------------------------------------------------------------
//===============================================================
static sSTACKBUF *_StkRefFrame(sSTACK *stk, int ofs)
//--------------------------------------------------------------
// �X�^�b�N�t���[�� �t���[�����Q��
// 	�擪�t���[�����̃I�t�Z�b�g�ʒu�Œl���Q�Ƃ��܂�
// --------------------------------------------------------------
// @param	stk	: �X�^�b�N�\����
// @param	ofs	: �I�t�Z�b�g
// --------------------------------------------------------------
// @return	�Q�ƃf�[�^�|�C���^
//===============================================================
{
	ofs += stk->frm[stk->frm_ptr - 1];
	STK_ASSERT(ofs<stk->buf_ptr, STK_ERR_INVALID_REF);
	return &stk->buf[ofs];
}

//===============================================================
static void _StkPush(sSTACK *stk, sSTACKBUF *val)
//--------------------------------------------------------------
// �X�^�b�N�t���[�� �l��PUSH
// --------------------------------------------------------------
// @param	stk	: �X�^�b�N�\����
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
// �X�^�b�N�t���[�� �l��POP
// --------------------------------------------------------------
// @param	stk	: �X�^�b�N�\����
// --------------------------------------------------------------
// @return	�l
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
// �Q��
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

