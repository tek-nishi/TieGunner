//
//	�n�b�V���@�ɂ��f�[�^����
//

#include "co_hash.h"
#include "co_memory.h"


#define _MODIFY_NISHI							// _NISHI �C����
#define hashMalloc(a)  MemMalloc(MEM_APP, a, "hash")


struct _IHASH {
	int	hashMax, hashMask;
	int	words, strings;
	HASH **hashTop;

	HASH *top, *use, *free;
};


#ifdef _MODIFY_NISHI

//==============================================================
static HASH *l_new(IHASH *hash)
//--------------------------------------------------------------
// �I�u�W�F�N�g���m��
//--------------------------------------------------------------
// in:	hash = �w�b�_
//--------------------------------------------------------------
// out:	�f�[�^�|�C���^(NULL = �m�ێ��s)
//==============================================================
{
	HASH *hp;

	// ���g�p�̈悩��P���o��
	//----------------------------
	hp = hash->free;
	if(hp)
	{
		hash->free = hp->next;

		// �g�p�̈�̐擪�ɒǉ�
		//----------------------
		hp->next = hash->use;
		hash->use = hp;
	}

	return hp;
}

//==============================================================
static void l_delete(IHASH *hash, HASH *hp)
//--------------------------------------------------------------
// �I�u�W�F�N�g��j��
//--------------------------------------------------------------
// in:	hash = �w�b�_
//		hp   = �j������I�u�W�F�N�g
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	HASH *prev;

	prev = hash->use;
	if(prev == hp)
	{
		// �擪�f�[�^�̏ꍇ
		//------------------
		hash->use = hp->next;
	}
	else
	{
		// �r���̃f�[�^�̏ꍇ
		//--------------------
		while(prev->next != hp)
			prev = prev->next;
		prev->next = hp->next;
	}

	// ���g�p�̈�̐擪�ɒǉ�
	//------------------------
	hp->next = hash->free;
	hash->free = hp;
}

//==============================================================
static void setString(HASH *hp, const char *s)
//--------------------------------------------------------------
// ��������R�s�[
//--------------------------------------------------------------
// in:	hp = �n�b�V���n���h��
//		s  = ������
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	int j;
	char *str;

	j = (int)strlen(s) + 1;
	if(j <= ID_MAXLEN)
	{
		str = hp->s;
		hp->str = NULL;
	}
	else
	{
		str = (char *)hashMalloc(j);
		hp->str = str;
	}
	strcpy(str, s);
}

//==============================================================
static char *getString(HASH *hp)
//--------------------------------------------------------------
// ��������擾
//--------------------------------------------------------------
// in:	hp = �n�b�V���n���h��
//--------------------------------------------------------------
// out:	������
//==============================================================
{
	return hp->str ? hp->str : hp->s;
}

//==============================================================
static void deleteString(HASH *hp)
//--------------------------------------------------------------
// �������j��
//--------------------------------------------------------------
// in:	hp = �n�b�V���n���h��
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	FreeWork(hp->str);
}

//==============================================================
static BOOL isString(HASH *hp)
//--------------------------------------------------------------
// ������ɂ��n�b�V��������
//--------------------------------------------------------------
// in:	hp = �n�b�V���n���h��
//--------------------------------------------------------------
// out:	������
//==============================================================
{
	return TRUE;
}

#endif

//==============================================================
IHASH *OpenHash(int hashMax)
//--------------------------------------------------------------
// �����e�[�u�����J��
//--------------------------------------------------------------
// in:	�ő�o�^��
//--------------------------------------------------------------
// out:	�A�N�Z�X�n���h��
//==============================================================
{
	int	i, j;
	IHASH *info;

	i =  hashMax;
	j = 1;
	while(i)
	{
		j += j;
		i >>= 1;
	}
	j >>= 1;
	if(j==0)
		j = 256;

//	PRINTF("Hash max: %d(%d)\n", j, hashMax);
	info = (IHASH *)hashMalloc(sizeof(IHASH));
	info->hashMax = j;
	info->hashMask = j - 1;

	info->hashTop = (HASH **)hashMalloc(sizeof(HASH *) * info->hashMax);
	ASSERT(info->hashTop);
	info->words = 0;
	for(i=0; i<info->hashMax; ++i)
		info->hashTop[i] = NULL;

#ifdef _MODIFY_NISHI
	{
		int i;
		HASH *hp;

		info->use = NULL;
		info->top = (HASH *)GetWork(sizeof(HASH) * hashMax, "HASH");
		info->free = info->top;

		hp = info->free;
		for(i = 0; i < (hashMax - 1); ++i)
		{
			hp->next = hp + 1;
			++hp;
		}
		hp->next = NULL;
	}
#endif

	return info;
}

//==============================================================
void CloseHash(IHASH *info)
//--------------------------------------------------------------
// �����e�[�u�������
//--------------------------------------------------------------
// in:	info = �A�N�Z�X�n���h��
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	int i;
	HASH *h, *m;

	for(i=0; i<info->hashMax; ++i)
	{
		h = info->hashTop[i];
		if(h)
		{
			do
			{
				m = h;
				h = m->n;
#ifdef _MODIFY_NISHI
				deleteString(m);
#else
				if(m->s) Free(m->s);
#endif

#ifdef _MODIFY_NISHI
				l_delete(info, m);
#else
				Free(m);
#endif
			}
			while(h!=NULL);
		}
	}
#ifdef _MODIFY_NISHI
	Free(info->top);
#endif
	Free(info->hashTop);
	Free(info);
}

//==============================================================
void ClearHash(IHASH *info)
//--------------------------------------------------------------
// �����e�[�u�����ď�����
//--------------------------------------------------------------
// in:	info = �A�N�Z�X�n���h��
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	int i;
	HASH *h, *m;

	for(i=0; i<info->hashMax; ++i)
	{
		h = info->hashTop[i];
		if(h)
		{
			do
			{
				m = h;
				h = m->n;
#ifdef _MODIFY_NISHI
				deleteString(m);
#else
				if(m->s) Free(m->s);
#endif

#ifdef _MODIFY_NISHI
				l_delete(info, m);
#else
				Free(m);
#endif
			}
			while(h!=NULL);
		}
		info->hashTop[i] = NULL;
	}
}

//==============================================================
static int CreateHash(const char *s)
//--------------------------------------------------------------
// �����񂩂�n�b�V�����쐬
//--------------------------------------------------------------
// in:	s = ������
//--------------------------------------------------------------
// out:	�n�b�V���l
//==============================================================
{
#ifdef _MODIFY_NISHI
	int i, j, k;

	k = j = 0;
	while(*s != 0)
	{
		i = *s;
		k += j ^ ((i << 5) + i + k);
		j += 1;
		s += 1;
	}

	return k;
#else
	register int	i, j, k;

	k = j = 0;
	while((i = *s++)!=0) k += j++ ^ ((i << 5) + i + k);
	return k;
#endif
}

//==============================================================
HASH *LookupString(IHASH *info, const char *s)
//--------------------------------------------------------------
// �C���[�W����
//--------------------------------------------------------------
// in:	info = �A�N�Z�X�n���h��
//		s    = ������
//--------------------------------------------------------------
// out:	�o�^�l(NULL = �Y������l�Ȃ�)
//==============================================================
{
	HASH *p;

#ifdef _MODIFY_NISHI
	p = info->hashTop[ CreateHash(s) & info->hashMask ];
	while(p != NULL)
	{
		// �R���W����������̂ŁA�����ƈ�v���Ă��邩���ׂ�
		//----------------------------------------------------
		if(strcmp(getString(p), s) == 0)
			break;

		p = (HASH *)p->n;
	}

	return p;
#else
	p = info->hashTop[ CreateHash(s) & info->hashMask ];
	while(p!=NULL)
	{
	if(strcmp(p->s, s)==0) return p;
	else p = (HASH *)p->n;
	}
	return p;
#endif
}

//==============================================================
HASH *InstallString(IHASH *info, const char *s)
//--------------------------------------------------------------
// ������̓o�^
//--------------------------------------------------------------
// in:	info = �A�N�Z�X�n���h��
//		s    = �o�^���镶����(ID_MAXLEN - 1)
//--------------------------------------------------------------
// out:	�o�^�l(NULL = ���ɓ��������񂪓o�^����Ă���)
//==============================================================
{
	int i;
	HASH *p;
#ifndef _MODIFY_NISHI
	int j;
	char *str;
#endif

	i = CreateHash(s) & info->hashMask;
	p = info->hashTop[i];
	if(p==NULL)
	{
#ifdef _MODIFY_NISHI
		p = l_new(info);
		ASSERT(p);
#else
		p = (HASH *)hashMalloc(sizeof(HASH));
#endif

#ifdef _MODIFY_NISHI
		setString(p, s);						// ��������R�s�[
#else
		j = strlen(s) + 1;
		p->s = (char *)hashMalloc(j);
		strcpy(p->s, s);
#endif
		p->a = info->words;
		p->n = NULL;
		info->words += 1;

		info->hashTop[i] = p;

#ifdef MSG_TEXT
		PRINTF("'%s' new install.(Words:%d)\n", s, info->words);
#endif
	}
	else
	{
		while(p!=NULL)
		{
#ifdef _MODIFY_NISHI
			if(strcmp(getString(p), s)==0)
#else
			if(strcmp(p->s, s)==0)
#endif
			{
#ifdef MSG_TEXT
				PRINTF("'%s' installed.\n", s);
#endif
				p = NULL;
			}
			else
			if(p->n!=NULL)
			{
				p = (HASH *)p->n;
			}
			else
			{
				// �R���W���������������ꍇ�̑[�u
				//--------------------------------
#ifdef _MODIFY_NISHI
				p->n = l_new(info);
				ASSERT(p->n);
#else
				p->n = (HASH *)hashMalloc(sizeof(HASH));
#endif
				p = (HASH *)p->n;

#ifdef _MODIFY_NISHI
				setString(p, s);				// ��������R�s�[
#else
				j = strlen(s) + 1;
				p->s = (char *)hashMalloc(j);
				strcpy(p->s, s);
#endif

				p->a = info->words;
				p->n = NULL;
				info->words += 1;
#ifdef MSG_TEXT
				PRINTF("'%s' lookup and install.(Words:%d)\n", s, info->words);
#endif
				return p;
			}
		}
	}
	return p;
}

//==============================================================
static void FreeHashString(IHASH *info, HASH *hp)
//--------------------------------------------------------------
// �o�^������̔j��
//--------------------------------------------------------------
// in:	info = �A�N�Z�X�n���h��
//		hp   = �폜����l
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	info->words--;
#ifdef MSG_TEXT
	#ifdef _MODIFY_NISHI
		PRINTF("Uninstall string: '%s'\n", getString(hp));
	#else
		PRINTF("Uninstall string: '%s'\n", hp->s);
	#endif
#endif

#ifdef _MODIFY_NISHI
	deleteString(hp);
#else
	Free(hp->s);
#endif
}

//==============================================================
void UninstallString(IHASH *info, const char *s)
//--------------------------------------------------------------
// �o�^��������폜
//--------------------------------------------------------------
// in:	info = �A�N�Z�X�n���h��
//		s    = �o�^������
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	HASH	*h, **hh;

	hh = &info->hashTop[ CreateHash(s) & info->hashMask ];

	h = *hh;
	if(h==NULL)
		return;

	while(TRUE)
	{
		if(h->n==NULL)
		{
			FreeHashString(info, h);
#ifdef _MODIFY_NISHI
			l_delete(info, h);
#else
			Free(h);
#endif
			*hh = NULL;

			return;
		}
		else
		{
#ifdef _MODIFY_NISHI
			if(strcmp(getString(h), s)==0)
#else
			if(strcmp(h->s, s)==0)
#endif
			{
				*hh = (HASH *)h->n;				/* ���̃|�C���^�[���q�� */
				FreeHashString(info, h);
#ifdef _MODIFY_NISHI
				l_delete(info, h);
#else
				Free(h);
#endif
				return;
			}
			else
			{
				hh = &h;
				h = (HASH *)h->n;
			}
		}
	}
}

//==============================================================
char *GetHashString(HASH *hp)
//--------------------------------------------------------------
// �n���h���̕�������擾
//--------------------------------------------------------------
// in:	hp = �n���h��
//--------------------------------------------------------------
// out:	�o�^������
//==============================================================
{
	return getString(hp);
}

//==============================================================
HASH **CreateHashList(IHASH *info)
//--------------------------------------------------------------
// �n�b�V���̑S���X�g�����
//--------------------------------------------------------------
// in:	info = �A�N�Z�X�n���h��
//--------------------------------------------------------------
// out:	�|�C���^���X�g(NULL = ���X�g�Ȃ�)
//==============================================================
{
	int	i, j;
	HASH	**ip, *p;

	if(info->words==0)
		return NULL;

	ip = (HASH **)hashMalloc(sizeof(HASH *) * (info->words + 1));
	j = 0;
	for(i=0; i<info->hashMax; ++i)
	{
		if((p = info->hashTop[i])!=NULL)
		{
			do
			{
#ifdef _MODIFY_NISHI
				if(isString(p))
				{
					ip[j] = p;
					++j;
				}
#else
				if(p->s)
				{
					ip[j] = p;
					++j;
				}
#endif
				p = (HASH *)p->n;
			}
			while(p);
		}
	}
	ip[j] = NULL;

	return ip;
}

//==============================================================
static void SwapHash(HASH **v, int a, int b)
//--------------------------------------------------------------
// �n�b�V���̌���
//--------------------------------------------------------------
// in:	v    = �n�b�V�����X�g
//		a, b = ����ւ���l�ւ̃C���f�b�N�X
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	HASH	*p;

	p = *(v + a);
	*(v + a) = *(v + b);
	*(v + b) = p;
}

//==============================================================
static void QsortHash(HASH **v, int left, int right)
//--------------------------------------------------------------
// �n�b�V���̃\�[�g
//--------------------------------------------------------------
// in:	v           = �n�b�V�����X�g
//		left, right = �\�[�g�̈�
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	int	i, last;

	if(left >= right)
		return;

	SwapHash(v, left, (left + right) / 2);
	last = left;
	for(i=left+1; i<=right; ++i)
	{
#ifdef _MODIFY_NISHI
		if(strcmp(getString(v[i]), getString(v[left])) < 0)
			SwapHash(v, ++last, i);
#else
		if(strcmp(v[i]->s, v[left]->s) < 0)
			SwapHash(v, ++last, i);
#endif
	}
	SwapHash(v, left, last);

	// �ċA�������Ă���̂ŁA�X�^�b�N���ɒ���
	//------------------------------------------
	QsortHash(v, left, last-1);
	QsortHash(v, last+1, right);
}

//==============================================================
HASH **SortHashList(IHASH *info)
//--------------------------------------------------------------
// �n�b�V���̃\�[�g
//--------------------------------------------------------------
// in:	info = �A�N�Z�X�n���h��
//--------------------------------------------------------------
// out:	�\�[�g���ꂽ���X�g
//==============================================================
{
	HASH	**pp;

	pp = CreateHashList(info);
	if(pp==NULL || info->words <= 1)
		return pp;

	QsortHash(pp, 0, info->words - 1);

	return pp;
}


//==============================================================
static void QsortHashValue(HASH **v, int left, int right, int ofs)
//--------------------------------------------------------------
// �n�b�V���l�̃\�[�g
//--------------------------------------------------------------
// in:	v           = �n�b�V���l
//		left, right = �\�[�g�͈�
//		ofs         = �C���f�b�N�X
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	int	i, last;
	HASH	h;

	if(left >= right)
		return;

	SwapHash(v, left, (left + right) / 2);
	last = left;

	if( ( (char *)&h.a - (char *)&h )==ofs)
	{
		for(i=left+1; i<=right; ++i)
		{
			if(v[i]->a < v[left]->a)
				SwapHash(v, ++last, i);
		}
	}
	else
	{
		for(i=left+1; i<=right; ++i)
		{
			if(v[i]->b < v[left]->b)
				SwapHash(v, ++last, i);
		}
	}
	SwapHash(v, left, last);

	// �ċA�������Ă���̂ŁA�X�^�b�N���ɒ���
	//------------------------------------------
	QsortHashValue(v, left, last-1, ofs);
	QsortHashValue(v, last+1, right, ofs);
}

//==============================================================
HASH **SortHashValueA(IHASH *info)
//--------------------------------------------------------------
// �l�`�ɑ΂��Ẵn�b�V���l�̃\�[�g
//--------------------------------------------------------------
// in:	info = �A�N�Z�X�n���h��
//--------------------------------------------------------------
// out:	�\�[�g���ꂽ���X�g
//==============================================================
{
	HASH	**pp, h;

	pp = CreateHashList(info);
	if(pp==NULL || info->words <= 1)
		return pp;

	QsortHashValue(pp, 0, info->words - 1, (char *)&h.a - (char *)&h);
	return pp;
}

//==============================================================
HASH **SortHashValueB(IHASH *info)
//--------------------------------------------------------------
// �l�a�ɑ΂��Ẵn�b�V���l�̃\�[�g
//--------------------------------------------------------------
// in:	info = �A�N�Z�X�n���h��
//--------------------------------------------------------------
// out:	�\�[�g���ꂽ���X�g
//==============================================================
{
	HASH	**pp, h;

	pp = CreateHashList(info);
	if(pp==NULL || info->words <= 1)
		return pp;

	QsortHashValue(pp, 0, info->words - 1, (char *)&h.b - (char *)&h);
	return pp;
}

//==============================================================
void ListtingHashList(IHASH *info, FILE *fp)
//--------------------------------------------------------------
// ���X�g�����n�b�V�����t�@�C���ɏ����o��
//--------------------------------------------------------------
// in:	info = �A�N�Z�X�n���h��
//		fp   = �t�@�C���|�C���^
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	HASH	**pp, *p;
	int	i;

	pp = CreateHashList(info);
	if(pp==NULL)
		return;

	i = 0;
	while((p = pp[i])!=NULL)
	{
#ifdef _MODIFY_NISHI
		fprintf(fp, "(%5d) $%08X, $%08X : '%s'\n", i, p->a, p->b, getString(p));
#else
		fprintf(fp, "(%5d) $%08X, $%08X : '%s'\n", i, p->a, p->b, p->s);
#endif
		i += 1;
	}
	Free(pp);
}

//==============================================================
HASH **AddHashPointer(HASH **hpp, HASH *hp)
//--------------------------------------------------------------
// �n�b�V�����X�g�Ɋۂ��Ɖ�����
//--------------------------------------------------------------
// in:	hpp = �n�b�V�����X�g
//		hp  = ������n�b�V�����X�g
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	int	i;
	HASH	**nhpp, *nhp;

	if(hp==NULL)
		return NULL;

	i = 0;
	if(hpp)
	{
		while((nhp = hpp[i])!=NULL)
		{
			if(nhp==hp)
				return hpp;
			else
				i += 1;
		}
	}

	nhpp = (HASH **)hashMalloc(sizeof(HASH *) * (i + 2));

	if(i)
	{
		i = 0;
		while((nhp = hpp[i])!=NULL)
		{
			nhpp[i] = nhp;
			++i;
		} 
		Free(hpp);
	}
	nhpp[i] = hp;
	nhpp[i + 1] = NULL;

	return nhpp;
}

