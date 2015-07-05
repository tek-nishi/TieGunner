//
//�ėp�����N���X�g
//

#include "co_objlink.h"
#include "co_memory.h"


typedef struct _sLinkHead sLinkHead;
struct _sLinkHead {
	sLinkHead *prev;							// �O�̃|�C���^
	sLinkHead *next;							// ���̃|�C���^
	BOOL alloc;									// �ʂ� malloc ����
};

struct _sLink {
	int mem;									// �������m�ۃG���A
	BOOL new_area;								// ������������Ȃ��Ȃ�����ǉ��Ŋm�ۂ���
	int obj_size;								// �f�[�^�T�C�Y(�w�b�_����)
	int size;									// �f�[�^�T�C�Y(�w�b�_����)

	u_char *top;								// �擪�̈�
	u_char *use;								// �g�p���̗̈�
	u_char *free;								// ���g�p�̗̈�
};


//==============================================================
sLink *ObjLinkCreate(int size, int num, int mem, BOOL new_area)
//--------------------------------------------------------------
// ����
//--------------------------------------------------------------
// in:	size     = �̈�T�C�Y
//		num      = �̈搔
//		mem      = �m�ۂ���̈�
//		new_area = ������������Ȃ��Ȃ�����ǉ��Ŋm�ۂ���
//--------------------------------------------------------------
// out:	�n���h��
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
// �j��
//--------------------------------------------------------------
// in:	link = �n���h��
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	Free(link->top);
	Free(link);
}

//==============================================================
void *ObjLinkNew(sLink *link)
//--------------------------------------------------------------
// �I�u�W�F�N�g���m��
//--------------------------------------------------------------
// in:	link = �n���h��
//--------------------------------------------------------------
// out:	�f�[�^�|�C���^(NULL = �m�ێ��s)
//==============================================================
{
	u_char *obj;

	obj = link->free;
	if((obj == NULL) && link->new_area)
	{
		sLinkHead *head;

		// �m�ۂł��Ȃ��ꍇ�ɂ́A�K�����������m�ۂ��ė��p
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

		// �g�p�̈�̐擪�ɒǉ�
		//----------------------
		prev = (sLinkHead *)link->use;
		if(prev) prev->prev = head;
		head->next = prev;
		head->prev = NULL;
		link->use = (u_char *)head;

		// ������
		//--------
		obj += sizeof(sLinkHead);
		ZEROMEMORY(obj, link->size);
	}

	return obj;
}

//==============================================================
void ObjLinkDel(sLink *link, void *obj)
//--------------------------------------------------------------
// �I�u�W�F�N�g���폜
//--------------------------------------------------------------
// in:	link = �n���h��
//		obj  = �f�[�^�|�C���^
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	sLinkHead *head;

	head = (sLinkHead *)obj - 1;				// ������Ɩ����ȋL�q

	// �����N���q��������
	//--------------------
	if(head->prev)
		head->prev->next = head->next;
	if(head->next)
		head->next->prev = head->prev;

	// �擪�f�[�^�������ꍇ
	//----------------------
	if(head == (sLinkHead *)link->use)
		link->use = (u_char *)head->next;

	if(head->alloc)
	{
		// �ʓr�m�ۂ����̈�̏ꍇ�͊J������
		Free(head);
	}
	else
	{
		// ���g�p�̈�̐擪�ɒǉ�
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
// �I�u�W�F�N�g�����X�g�ɑ}��
//--------------------------------------------------------------
// in:	link = �n���h��
//		obj  = �}������f�[�^�|�C���^
//		src  = �}����
//		next = TRUE: �O�ɑ}��
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	sLinkHead *head;
	sLinkHead *prev;

	if(obj == src)		return;

	// �����N���q��������
	//--------------------
	head = (sLinkHead *)obj - 1;
	if(head->prev)
		head->prev->next = head->next;
	if(head->next)
		head->next->prev = head->prev;

	// �擪�f�[�^�������ꍇ
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

		// �擪�f�[�^�������ꍇ
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
// �S�I�u�W�F�N�g���폜
//--------------------------------------------------------------
// in:	link = �n���h��
//--------------------------------------------------------------
// out:	�Ȃ�
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
// �ŏ��̃|�C���^���擾
// ���Ō�ɓo�^���ꂽ�I�u�W�F�N�g���擾����
//--------------------------------------------------------------
// in:	link = �n���h��
//--------------------------------------------------------------
// out:	�ŏ��̃|�C���^(NULL = �Ȃ�)
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
// �Ō�̃|�C���^���擾
// ���ŏ��ɓo�^���ꂽ�I�u�W�F�N�g���擾����
//--------------------------------------------------------------
// in:	link = �n���h��
//--------------------------------------------------------------
// out:	�Ō�̃|�C���^(NULL = �Ȃ�)
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
// ���̃|�C���^���擾
//--------------------------------------------------------------
// in:	ptr = �f�[�^�|�C���^
//--------------------------------------------------------------
// out:	���̃|�C���^(NULL = �Ȃ�)
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
// �O�̃|�C���^���擾
//--------------------------------------------------------------
// in:	ptr = �f�[�^�|�C���^
//--------------------------------------------------------------
// out:	�O�̃|�C���^(NULL = �Ȃ�)
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
// �f�[�^�����擾
//--------------------------------------------------------------
// in:	link = �n���h��
//--------------------------------------------------------------
// out:	�f�[�^��
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
