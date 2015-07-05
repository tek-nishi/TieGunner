//
// �������Ǘ�
//

#include "co_memory.h"
#include "co_os.h"
#include "co_debug.h"
#include "co_random.h"
#include "co_strings.h"


//----------------
// �e�̈�̃T�C�Y
//----------------
#define MEM_SIZE     0xa00000						// �S�̂Ŏg���������T�C�Y
#define MEM_SYS_SIZE 0x200000						// �V�X�e��
#define MEM_DEV_SIZE 0x100000						// �f�o�b�O�p�̈�

#define MEM_HEADER  0x12345678						// �f�[�^�j��`�F�b�N�p

#define _MODIFY_FREE								// Free() �y�ʔ�
#ifdef DEBUG
//#define CHECK_MIN_MEMORY
#endif


typedef struct {
	int area;
	char name[ID_MAXLEN];
} sMemInfoStack;


//----------------------------
// enmMEM_AREA �ƕ��т𓯂���
//----------------------------
static char *memName[] = {
	" MEM_SYS",
	" MEM_APP",

	" MEM_DEV",
};
static size_t memSizeTbl[] = {
	MEM_SYS_SIZE,
};

static sMemInfo gMem;							// ��������

#ifdef DEBUG_MEMORY
	int magicNumber = 0;
#endif


/********************************************/
/*                �v���O����                */
/********************************************/
//--------------------------------------------------------------
//  ���[�J���֐�
//--------------------------------------------------------------

//==============================================================
static void setName(Header *hdr, const char *name)
//--------------------------------------------------------------
// �̈於���R�s�[
//--------------------------------------------------------------
// in:	hdr  = �������w�b�_
//		name = �̈於(NULL = ���O����)
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	if(name)
		STRCPY16(hdr->s.name, name);
	else
		hdr->s.name[0] = '\0';
}


//--------------------------------------------------------------
//  �O���[�o���֐�
//--------------------------------------------------------------

//==============================================================
void *MemDummyMalloc(void)
//--------------------------------------------------------------
// gnu malloc ���g�p�����ꍇ�ɃG���[���o��
// �� malloc() �� ���̊֐����ĂԂ悤�Ƀ}�N����`����Ă���B
//--------------------------------------------------------------
// in:	�Ȃ�
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	ASSERT(0);
	return NULL;
}

//==============================================================
void MemDummyFree(void)
//--------------------------------------------------------------
// gnu free ���g�p�����ꍇ�ɃG���[���o��
// �� free() �� ���̊֐����ĂԂ悤�Ƀ}�N����`����Ă���B
//--------------------------------------------------------------
// in:	�Ȃ�
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	ASSERT(0);
}

//==============================================================
static void bsMemoryClean(sMemHeader *header, int flag)
//--------------------------------------------------------------
// �m�ۂ���Ă���̈��S�ĊJ��
//--------------------------------------------------------------
// in:	header = �������n���h��
//		flag   = TRUE : �̈�������_���N���A����
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	Header *p;

	p = (Header *)header->top;

	if(flag)
	{
		/* �K���Ȓl�ŃN���A */
		memset(p, 0xfd, header->size * sizeof(Header));
	}

	p->s.area = MEM_NONE;
	p->s.size = header->size;
	p->s.ptr = NULL;							// NULL = �G���h�R�[�h
	p->s.header = MEM_HEADER;
	strcpy(p->s.name, "free");

#ifdef DEBUG_MEMORY
	p->s.magicNumber = magicNumber;
	magicNumber = (magicNumber + 1) & 0x7;
#endif

	header->freep = p;
	header->last = NULL;

	/* �f�o�b�O��� */
	header->allocate = header->total = (u_int)((header->size - 1) * sizeof(Header));
	header->min_free = header->total;
	header->fragment = 0;

	PRINTF("Memory Init. %x(%x)\n", header->top, header->size * sizeof(Header));
}

//==============================================================
static void bsMemoryInit(sMemHeader *header, void *ptr, size_t size, int flag)
//--------------------------------------------------------------
// �������Ǘ�������
//--------------------------------------------------------------
// in:	header = �������n���h��
//		ptr    = �������̈�̐擪�A�h���X(64 �o�C�g�A���C��)
//		size   = �̈�̃T�C�Y(�P�� : �o�C�g)
//		flag   = TRUE : �̈�������_���N���A����
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	header->top = ptr;							// �̈�̈ʒu�ƃT�C�Y��ێ����Ă���
	header->size = size / (size_t)sizeof(Header);

	bsMemoryClean(header, flag);
}

//==============================================================
void bsMemoryCheck(sMemHeader *header)
//--------------------------------------------------------------
// �������̒f�Љ��Ȃǂ��`�F�b�N
// ���w�b�_�ɏ�������
//--------------------------------------------------------------
// in:	header = �������n���h��
//--------------------------------------------------------------
// out:	�Ȃ�
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
		p = p->s.ptr;							/* ���̋󂫗̈� */
	}

	header->total = total * sizeof(Header);
	header->allocate = (allocate - 1) * sizeof(Header);
	header->fragment = num;

	if(header->min_free > header->total)
		header->min_free = header->total;		// �ŏ��l���o���Ă���
}

//==============================================================
static size_t bsMemoryGetFree(sMemHeader *header)
//--------------------------------------------------------------
// �m�ۉ\�ȍő�̈�𒲂ׂ�
//--------------------------------------------------------------
// in:	header = �������n���h��
//--------------------------------------------------------------
// out:	�m�ۉ\�ȃT�C�Y
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

		p = p->s.ptr;							/* ���̋󂫗̈� */
	}

	return (allocate - 1) * sizeof(Header);
}

//==============================================================
static void *bsMalloc(sMemHeader *header, size_t size, const char *name)
//--------------------------------------------------------------
// ���������蓖��
// ���������̉��̕�����m��
//--------------------------------------------------------------
// in:	header = �������n���h��
//		size   = �m�ۂ��郁�����T�C�Y
//		name   = �̈於
//--------------------------------------------------------------
// out:	���������������̈�̐擪�A�h���X(NULL = �m�ێ��s)
//==============================================================
{
	Header *p, *prevp;
	size_t nunits;
	Header *resvPtr;
	Header *resvPrevPtr;

	// �m�ۂ���u���b�N�T�C�Y�����߂�
	//--------------------------------
	ASSERT(size);
	nunits = (size + sizeof(Header) - 1) / sizeof(Header) + 1;

	p = header->freep;
	prevp = NULL;
	resvPtr = NULL;								/* �œK�ȏꏊ���������� */
	resvPrevPtr = NULL;
	while(p)
	{
		// ��ԑ傫���A�h���X�ŒT��
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
		return NULL;							/* ���蓖�Ă���̈悪���� */
	}

	p = resvPtr;
	prevp = resvPrevPtr;
	if(p->s.size == nunits)
	{
		// !!! �኱�댯�ȃR�[�h���� !!!
		if(prevp)	prevp->s.ptr = p->s.ptr;	// ���X�g�̓r��
		else		header->freep = p->s.ptr;	// ���X�g�̐擪
	}
	else
	{
		p->s.size -= nunits;					// ����̕��������蓖�Ă�
		p += p->s.size;
		p->s.size = nunits;
	}

	setName(p, name);							// �̈於���Z�b�g
	p->s.header = MEM_HEADER;
	p->s.area = header->area;
	p->s.top = FALSE;

#ifdef DEBUG_MEMORY
	p->s.magicNumber = magicNumber;
	magicNumber = (magicNumber + 1) & 0x7;
#endif

	// �m�ۂ����̈���|�C���^���X�g�ɒǉ�
	//------------------------------------
#ifdef _MODIFY_FREE
	if(header->last)	header->last->s.prev = p;
	p->s.prev = NULL;
#endif
	p->s.ptr = header->last;
	header->last = p;

#if 0
	//------------------------------------
	// ROM �ō쐬�������̃������`�F�b�N�p
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
	
	return (void *)(p + 1);				/* �w�b�_���������̈��Ԃ� */
}

//==============================================================
static void *bsMalloc2(sMemHeader *header, size_t size, const char *name)
//--------------------------------------------------------------
// ���������蓖��
// ���������̏�̂ق�����m��
//--------------------------------------------------------------
// in:	header = �������n���h��
//		size   = �m�ۂ��郁�����T�C�Y
//		name   = �̈於
//--------------------------------------------------------------
// out:	���������������̈�̐擪�A�h���X
//==============================================================
{
	Header *p;
	Header *prevp;
	Header *max_p;
	Header *resv_p;
	Header *resv_prev;
	size_t nunits;

	// �m�ۂ���u���b�N�T�C�Y�����߂�
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
		// ��ԏ������A�h���X�ŒT��
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
		return NULL;							/* ���蓖�Ă���̈悪���� */
	}

	p = resv_p;
	prevp = resv_prev;
	if(p->s.size == nunits)
	{
		// ���S��v
		//----------
		// !!! �኱�댯�ȃR�[�h���� !!!
		if(prevp)	prevp->s.ptr = p->s.ptr;	// ���X�g�̓r��
		else		header->freep = p->s.ptr;	// ���X�g�̐擪
	}
	else
	{
		// �O�������蓖�Ă�
		//------------------
		Header *free;

		free = p + nunits;
		*free = *p;
		free->s.size = p->s.size - nunits;
		p->s.size = nunits;

		if(prevp)	prevp->s.ptr = free;
		else		header->freep = free;
	}

	setName(p, name);							// �̈於���Z�b�g
	p->s.header = MEM_HEADER;
	p->s.area = header->area;
	p->s.top = TRUE;

#ifdef DEBUG_MEMORY
	p->s.magicNumber = magicNumber;
	magicNumber = (magicNumber + 1) & 0x7;
#endif

	// �m�ۂ����̈���|�C���^���X�g�ɒǉ�
	//------------------------------------
#ifdef _MODIFY_FREE
	if(header->last)	header->last->s.prev = p;
	p->s.prev = NULL;
#endif
	p->s.ptr = header->last;
	header->last = p;

#if 0
	//------------------------------------
	// ROM �ō쐬�������̃������`�F�b�N�p
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

	return (void *)(p + 1);				/* �w�b�_���������̈��Ԃ� */
}

//==============================================================
static void bsFree(sMemHeader *header, void *ptr)
//--------------------------------------------------------------
// �������J��
//--------------------------------------------------------------
// in:	header = �������n���h��
//		ptr    = �J�����郁�����̈�̐擪�A�h���X
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	Header *bp;
	Header *p;
	Header *prev;

	if(!ptr)	return;

	bp = (Header *)ptr - 1;						/* �u���b�N�w�b�_���w�� */
	strcpy(bp->s.name, "free");					// �̈於�̏�����

#ifdef DEBUG
	// �J�������������������_���N���A
	memset(ptr, 0xfd, (bp->s.size - 1) * sizeof(Header));
#endif

	if(bp->s.area == MEM_NONE)
	{
		// �Q�d�J��
		ASSERT(0);
		return;
	}

	// �g�p���̃��X�g����O��
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

	// �󂫃��X�g�ɉ�����
	//--------------------
	if(!header->freep)
	{
		// �󂫗̈悪�S�����������ꍇ
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
				// �擪
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
				// �Ō��
				//--------
				if((p + p->s.size) == bp)
				{
					// ��ƌ���
					p->s.size += bp->s.size;
				}
				else
				{
					// �Ō���ɒǉ�
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
					// ��ƌ���
					p->s.size += bp->s.size;
					p->s.ptr = bp->s.ptr;
					bp = p;
				}

				if((bp + bp->s.size) == prev)
				{
					// ���ƌ���
					bp->s.size += prev->s.size;
					bp->s.ptr = prev->s.ptr;
				}
				break;
			}
			p = p->s.ptr;
		}
	}

	bp->s.area = MEM_NONE;						// ���������J�����ꂽ���Ƃ��w�b�_�ɏ�������ł���
}

//==============================================================
static void *bsRealloc(sMemHeader *header, void *ptr, size_t size, const char *name)
//--------------------------------------------------------------
// �������Ċ��蓖��
//--------------------------------------------------------------
// in:	header = �������n���h��
//		ptr    = �f�[�^�|�C���^(NULL �͓n����Ȃ�)
//		size   = �V�����v������T�C�Y
//		name   = �̈於
//--------------------------------------------------------------
// out:	���������������̈�̐擪�A�h���X
//==============================================================
{
	size_t newunits, units, d;
	Header *p;
	void *newptr;

	if(!size)
	{
		bsFree(header, ptr);					// size == 0 : �̈���J������
		return NULL;
	}

	p = (Header *)ptr - 1;
	units = p->s.size;
	newunits = (size + sizeof(Header) - 1) / sizeof(Header) + 1;
	d = units - newunits;
	if((d == 0) || (d == 1))
	{
		return ptr;								// �u���b�N���ɕω���������΁A�������s��Ȃ�
	}
	else
	{
		if(p->s.top)	newptr = bsMalloc2(header, size, name);
		else			newptr = bsMalloc(header, size, name);

		if(newptr)
		{
			memcpy(newptr, ptr, size);			// �ʂ̗̈悪�m�ۂ��ꂽ�ꍇ�A�f�[�^���R�s�[����
			bsFree(header, ptr);				// �ʂ̗̈悪�m�ۂ��ꂽ�ꍇ�̂݁A�I���W�i�����J��
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
// ���������蓖��
// ���m�ۂ����������̓��e���[���N���A���܂�
//--------------------------------------------------------------
// in:	header = �������n���h��
//		count  = �v�f��
//		size   = �v�f���Ƃ̃T�C�Y
//		name   = �̈於
//--------------------------------------------------------------
// out:	���������������̈�̐擪�A�h���X
//==============================================================
{
	void *ptr;

	ptr = bsMalloc(header, count * size, name);
	if(ptr)
	{
		/* �������̓��e���[���N���A */
		memset(ptr, 0, count * size);
	}

	return ptr;
}

//==============================================================
static void *bsCalloc2(sMemHeader *header, size_t count, size_t size, const char *name)
//--------------------------------------------------------------
// ���������蓖��
// ���m�ۂ����������̓��e���[���N���A���܂�
//--------------------------------------------------------------
// in:	header = �������n���h��
//		count  = �v�f��
//		size   = �v�f���Ƃ̃T�C�Y
//		name   = �̈於
//--------------------------------------------------------------
// out:	���������������̈�̐擪�A�h���X
//==============================================================
{
	void *ptr;

	ptr = bsMalloc2(header, count * size, name);
	if(ptr)
	{
		/* �������̓��e���[���N���A */
		memset(ptr, 0, count * size);
	}

	return ptr;
}

//==============================================================
int bsMemoryHeaderCheck(Header *p)
//--------------------------------------------------------------
// �w�b�_�`�F�b�N
//--------------------------------------------------------------
// in:	p = �������n���h��
//--------------------------------------------------------------
// out:	FALSE : �������j��̉\������
//==============================================================
{
	return (p->s.header == MEM_HEADER);
}

//==============================================================
void MemInit(void)
//--------------------------------------------------------------
// �������Ǘ��̏�����
//--------------------------------------------------------------
// in:	�Ȃ�
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	void *ptr;
	size_t size;
	int i;

	ZEROMEMORY(&gMem, sizeof(sMemInfo));		// �Ǘ��̈��������

	//----------------------------
	// �q�[�v�̈���쐬���ď�����
	//----------------------------
	ptr = OsCreateMemory(MEM_SIZE);

	// �󂫃G���A��S�ăO���[�o���̈�Ƃ��Ċm��
	//------------------------------------------
	bsMemoryInit(&gMem.global_mem_hdr, ptr, MEM_SIZE, TRUE);

	//--------------------
	// MEM_APP �ȊO���m��
	//--------------------
	for(i = MEM_SYS; i < MEM_APP; ++i)
	{
		ptr = bsMalloc(&gMem.global_mem_hdr, memSizeTbl[i], NULL);
		ASSERT(ptr);
		bsMemoryInit(&gMem.mem_hdr[i], ptr, memSizeTbl[i], FALSE);
		gMem.mem_hdr[i].area = i;
	}

	//------------------------------------------------------
	// �O���[�o���̈�̎c����A�v���P�[�V�����̈�Ƃ��Ċm��
	//------------------------------------------------------
	size = bsMemoryGetFree(&gMem.global_mem_hdr);

	ptr = bsMalloc(&gMem.global_mem_hdr, size, NULL);
	ASSERT(ptr);
	bsMemoryInit(&gMem.mem_hdr[MEM_APP], ptr, size, FALSE);
	gMem.mem_hdr[MEM_APP].area = MEM_APP;

#ifdef DEBUG
	//--------------------
	// �J�����̗̈�̊m��
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
// �������Ǘ��̏I��
//--------------------------------------------------------------
// in:	�Ȃ�
//--------------------------------------------------------------
// out:	�Ȃ�
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
// �w��̈悩�烁�������m��
//--------------------------------------------------------------
// in:	area = �̈掯�ʎq(enmMEM_AREA)
//		size = �m�ۂ��郁�����T�C�Y
//		name = �̈於
//--------------------------------------------------------------
// out:	���������������̈�̐擪�A�h���X
//==============================================================
{
	Header *p;

#ifndef DEBUG
	ASSERT(area != MEM_DEV);
	if(area == MEM_DEV)		area = MEM_APP;		/* �b�� */
#endif

	if(area == MEM_FS)
	{
		// FS �̎��� APP �̏ォ��m��
		p = (Header *)bsMalloc2(&gMem.mem_hdr[MEM_APP], size, name);
	}
	else
	{
		/* ����ȊO�͉�����m�� */
		p = (Header *)bsMalloc(&gMem.mem_hdr[area], size, name);
	}

	return p;
}

//==============================================================
void *MemRealloc(int area, void *ptr, size_t size, const char *name)
//--------------------------------------------------------------
// �w��̈��ύX
//--------------------------------------------------------------
// in:	area = �̈掯�ʎq(enmMEM_AREA)
//		ptr  = �̈�(NULL �̎��́Aarea �̈悩��m��)
//		size = �m�ۂ��郁�����T�C�Y
//		name = �̈於
//--------------------------------------------------------------
// out:	���������������̈�̐擪�A�h���X
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
// �w��̈悩�烁�������m��
//--------------------------------------------------------------
// in:	area  = �̈掯�ʎq(enmMEM_AREA)
//		count = �v�f��
//		size  = �m�ۂ��郁�����T�C�Y
//		name  = �̈於
//--------------------------------------------------------------
// out:	���������������̈�̐擪�A�h���X
//==============================================================
{
	Header *p;

#ifndef DEBUG
	ASSERT(area != MEM_DEV);

	if(area == MEM_DEV)
		area = MEM_APP;							/* �b�� */
#endif

	if(area == MEM_FS)
	{
		// FS �̎��� APP �̏ォ��m��
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
// ���������J��
//--------------------------------------------------------------
// in:	ptr  = �J������̈�
//--------------------------------------------------------------
// out:	�Ȃ�
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
// �w��̈�̃����������ׂĊJ��
//--------------------------------------------------------------
// in:	area = �̈掯�ʎq(enmMEM_AREA)
//		flag = TRUE : �̈�������_���N���A����
//--------------------------------------------------------------
// out:	�Ȃ�
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
// ���p�󋵂�\������
//--------------------------------------------------------------
// in:	�Ȃ�
//--------------------------------------------------------------
// out:	�Ȃ�
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
// �w�b�_�̎擾
//--------------------------------------------------------------
// in:	area = enmMEM_AREA
//--------------------------------------------------------------
// out:	�w�b�_
//==============================================================
{
	return &gMem.mem_hdr[area];
}

//==============================================================
int MemGetNumArea(BOOL alloc, sMemHeader *header)
//--------------------------------------------------------------
// �̈搔���擾
//--------------------------------------------------------------
// in:	alloc = TRUE:�m�ۗ̈�
//--------------------------------------------------------------
// out:	�̈搔
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
// �������̎g�p�󋵂Ȃǂ̎擾
//--------------------------------------------------------------
// in:	header = �w�b�_
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	bsMemoryCheck(header);
}

