
/*

	�������Ǘ�������Ǝ��ɍs���܂��B
	�uK & R �v���O���~���O����b�v�� �u8.7�L�����蓖�āv���Q�l�ɂ��Ă��܂��B

	�E�m�ۂ����̈�́A64bytes �P�ʂŃA���C������܂�(�ύX�\)�B�������A�w�b�_
	  �̃T�C�Y�̓s����A�Œ�A���C���T�C�Y�� 64bytes �ł��B

	�E�̈�J���͈ꊇ���� Free() ���g���܂�
	�EhogeClean() �́A�̈���ď���������֐��ł��B

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
/*             �萔�E�}�N���錾             */
/********************************************/
//--------------------------------------------------
// �̈�錾
// ���̈�̕��я���ύX������A�\�[�X���C�����鎖�B
//--------------------------------------------------
enum enmMEM_AREA {
	MEM_NONE = -1,
	MEM_SYS = 0,
	MEM_APP,

	MEM_DEV,

	//--------
	MEM_BLOCK_NUM,								// �������u���b�N�̑���

	MEM_FS										// MEM_FS ��p�~�����̂ŁA�����ɗ�(�݊��p)
};

//--------------------------------
// ��{�I�ɕW�� malloc �͎g�p�֎~
//--------------------------------
#define malloc( size )				MemDummyMalloc()		// ���g�̓G���[�֐�
#define memalign( align, size )		MemDummyMalloc()		// ���g�̓G���[�֐�
#define free( x )					MemDummyFree()			// ���g�̓G���[�֐�

//------------------------------
// �e�֐��̃��b�v
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
/*                �\���̐錾                */
/********************************************/
union uHeader {
	struct {
		int header;								// �w�b�_(�f�[�^�j��`�F�b�N�p)
		int magicNumber;						// �������m�ێ��̃}�W�b�N�i���o�[
		int area;								// �m�ۂ����ꏊ
		size_t size;							// ���̃u���b�N�̑傫��(�P�� : sizeof(uHeader) �w�b�_���܂�)
		union uHeader *ptr;						// ���̋󂫃u���b�N�ւ̃|�C���^
		union uHeader *prev;					// �O�̋󂫃u���b�N�ւ̃|�C���^
		int top;								// TRUE = �������̏㕔����m�ۂ���
		char name[ID_MAXLEN];					// �������u���b�N��
	} s;
	u_int x[16];								// �u���b�N�̐���������(64)
};
typedef union uHeader Header;

typedef struct {
	Header *freep;								// �󂫗̈惊�X�g
	Header *last;								// �g�p�̈惊�X�g

	void *top;									// �g�p���Ă���̈�
	size_t size;								// �̈�T�C�Y(�u���b�N��)
	int area;									// �̈�ԍ�
	u_int allocate, total;						// ��x�Ŋm�ۉ\�ȍő�T�C�Y�A�󂫗̈�̍��v
	u_int min_free;								// �󂫗̈�̍��v����ԏ��Ȃ����̒l
	int fragment;								// �f�Љ���
} sMemHeader;

typedef struct {
	sMemHeader global_mem_hdr;					// �O���[�o���������̈�
	sMemHeader mem_hdr[MEM_BLOCK_NUM];			// �V�X�e���Ŏg�p���郁�����̈�
} sMemInfo;


/********************************************/
/*              �O���[�o���ϐ�              */
/********************************************/


/********************************************/
/*              �O���[�o���֐�              */
/********************************************/
//	gnu malloc ���g�p�����ꍇ�ɃG���[���o��
extern void *MemDummyMalloc(void);
//	gnu free ���g�p�����ꍇ�ɃG���[���o��
extern void MemDummyFree(void);

//	�������Ǘ�������
extern void MemInit(void);
//	�������Ǘ��I��
extern void MemFin(void);
//	�w��̈悩�烁�������m��
extern void *MemMalloc(RwInt32 mode, size_t size, const char *name);
//	�w��̈��ύX
extern void *MemRealloc(RwInt32 area, void *ptr, size_t size, const char *name);
//	�w��̈悩�烁�������m��
extern void *MemCalloc(RwInt32 area, size_t count, size_t size, const char *name);
//	���������J��
extern void MemFree(void *ptr);
//	�w��̈�̃����������ׂĊJ��
extern void MemClean(RwInt32 mode, RwBool flag);

//	sysMalloc �Ŏ擾����郁�����̈�̖��O��ύX����
extern void MemSetSysName(const char *name);
//	sysMalloc �Ŏ擾����郁�����̈�̃G���A��ύX����
extern void MemSetSysArea(RwInt32 area);
//	sysMalloc �Őݒ肳��郁�����̈�̖��O���擾����
extern char *MemGetSysName(void);
//	sysMalloc �Őݒ肳��郁�����̈�̃G���A���擾����
extern RwInt32 MemGetSysArea(void);
//	sysMalloc �Ŏ擾����郁�����̈�̃G���A�Ɩ��O���܂Ƃ߂Đݒ�
extern void MemPushSysArea(RwInt32 area, char *name);
//	sysMalloc �Ŏ擾����郁�����̈�̃G���A�Ɩ��O�𕜋A
extern void MemPopSysArea(void);

//	�������̒f�Љ��Ȃǂ��`�F�b�N
extern void bsMemoryCheck(sMemHeader *header);
//	�w�b�_�`�F�b�N
extern RwBool bsMemoryHeaderCheck(Header *p);

// ���p�󋵂�\������
extern void MemDisp(void);
// �������w�b�_�̎擾
extern sMemHeader *MemGetHeader(int area);
// �̈搔���擾
extern int MemGetNumArea(BOOL alloc, sMemHeader *header);
// �������̎g�p�󋵂Ȃǂ̎擾
extern void MemGetInfo(sMemHeader *header);


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// �C�����C���֐��Q
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

