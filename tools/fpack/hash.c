//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
//  < fpack >
//   2004 ASTROLL Inc. All Rights Reserved.
//--------------------------------------------------------------
//
//	�n�b�V���@�ɂ�錟��
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  $Id: hash.c,v 1.1.1.1 2004/02/09 08:13:36 nishi Exp $
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/********************************************/
/*           �C���N���[�h�t�@�C��           */
/********************************************/
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "default.h"
#include "hash.h"
#include "main.h"


/********************************************/
/*             �萔�E�}�N���錾             */
/********************************************/
#define _MODIFY_NISHI							// _NISHI �C����

/********************************************/
/*                �\���̐錾                */
/********************************************/


/********************************************/
/*                 �ϐ��錾                 */
/********************************************/


/********************************************/
/*                �v���O����                */
/********************************************/

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

//	printf("Hash max: %d\n", j);
	info = (IHASH *)malloc(sizeof(IHASH));
	info->hashMax = j;
	info->hashMask = j - 1;
	info->hashTop = (HASH **)malloc(sizeof(HASH *) * info->hashMax);
	info->words = info->strings = 0;
	info->imageSize = 0;
	for(i=0; i<hashMax; ++i)
		info->hashTop[i] = NULL;

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
				if(m->s) free(m->s);
				free(m);
			}
			while(h!=NULL);
		}
	}
	free(info->hashTop);
	free(info);
}


//==============================================================
static _INLINE int CreateHash(const char *s)
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
		k += j++ ^ ((i << 5) + i + k);

		s++;
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
static _INLINE int CreateHashImage(const char *s, int size)
//--------------------------------------------------------------
// �w�蒷�̒l����̃n�b�V���쐬
//--------------------------------------------------------------
// in:	s    = ������
//		size = �f�[�^�T�C�Y
//--------------------------------------------------------------
// out:	�n�b�V���l
//==============================================================
{
#ifdef _MODIFY_NISHI
	int i, j, k;

	k = j = 0;
	while(size > 0)
	{
		i = *(const unsigned char *)s;
		k += j++ ^ ((i << 5) + i + k);

		s++;
		size--;
	}

	return k;
#else
	register int	i, j, k;

	k = j = 0;
	while(size > 0)
	{
	i = *(const unsigned char *)s++;
	k += j++ ^ ((i << 5) + i + k);
	size--;
	}
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
	while(p!=NULL)
	{
		if(strcmp(p->s, s)==0)
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
static _INLINE int ImageCmp(char *d, const char *s, int sz)
//--------------------------------------------------------------
// �w�蒷�̃f�[�^���r
//--------------------------------------------------------------
// in:	d  = ��r��
//		s  = ��r��
//		sz = �T�C�Y
//--------------------------------------------------------------
// out:	0 = ���S��v
//==============================================================
{
	while(sz > 0)
	{
		if( *d != *s )
			return 1;

		d++;
		s++;
		sz--;
	}

	return 0;
}

//==============================================================
static _INLINE void ImageCpy(char *d, const char *s, int sz)
//--------------------------------------------------------------
// �w�蒷�̃f�[�^���R�s�[
//--------------------------------------------------------------
// in:	d  = �R�s�[��
//		s  = �R�s�[��
//		sz = �T�C�Y
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	while(sz > 0)
	{
		*d++ = *s++;
		sz--;
	}
}

//==============================================================
HASH *LookupImage(IHASH *info, const char *s)
//--------------------------------------------------------------
// �n�b�V������
//--------------------------------------------------------------
// in:	info = �A�N�Z�X�n���h��
//		s    = ����������
//--------------------------------------------------------------
// out:	�o�^�l(NULL = �Y������l�Ȃ�)
//==============================================================
{
	HASH	*p;

#ifdef _MODIFY_NISHI
	p = info->hashTop[ CreateHashImage(s, info->imageSize) & info->hashMask ];
	while(p!=NULL)
	{
		if(ImageCmp(p->s, s, info->imageSize)==0)
			break;

		p = (HASH *)p->n;
	}
	return p;
#else
	p = info->hashTop[ CreateHashImage(s, info->imageSize) & info->hashMask ];
	while(p!=NULL)
	{
	if(ImageCmp(p->s, s, info->imageSize)==0) return p;
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
//		s    = �o�^���镶����
//--------------------------------------------------------------
// out:	�o�^�l
//==============================================================
{
	int i, j;
	HASH *p;

	i = CreateHash(s) & info->hashMask;
	p = info->hashTop[i];
	if(p==NULL)
	{
		p = (HASH *)malloc(sizeof(HASH));
		j = strlen(s) + 1;
		p->s = (char *)malloc(j);
		strcpy(p->s, s);
		p->a = info->words;
		p->n = NULL;
		info->words++;
		info->strings += j;

		info->hashTop[i] = p;

#ifdef MSG_TEXT
		printf("'%s' new install.(Words:%d, Strings:%d)\n", s, info->words, info->strings);
#endif

	}
	else
	{
		while(p!=NULL)
		{
			if(strcmp(p->s, s)==0)
			{
#ifdef MSG_TEXT
				printf("'%s' installed.\n", s);
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
				p->n = (HASH *)malloc(sizeof(HASH));
				p = (HASH *)p->n;
				j = strlen(s) + 1;
				p->s = (char *)malloc(j);
				strcpy(p->s, s);
				p->a = info->words;
				p->n = NULL;
				info->words++;
				info->strings += j;
#ifdef MSG_TEXT
				printf("'%s' lookup and install.(Words:%d, Strings:%d)\n", s, info->words, info->strings);
#endif
				return p;
			}
		}
	}
	return p;
}

//==============================================================
HASH *InstallImage(IHASH *info, const char *s)
//--------------------------------------------------------------
// �C���[�W�̓o�^
//--------------------------------------------------------------
// in:	info = �A�N�Z�X�n���h��
//		s    = �o�^���镶����
//--------------------------------------------------------------
// out:	�o�^�l
//==============================================================
{
	int	i, j;
	HASH	*p;

	i = CreateHashImage(s, info->imageSize) & info->hashMask;
	p = info->hashTop[i];
	if(p==NULL)
	{
		p = (HASH *)malloc(sizeof(HASH));
		j = info->imageSize;
		p->s = (char *)malloc(j);
		ImageCpy(p->s, s, j);
		p->a = info->words;
		p->n = NULL;
		info->words++;
		info->strings += j;

		info->hashTop[i] = p;
#ifdef MSG_TEXT
		printf("New install.(Words:%d, Image:%d)\n", info->words, info->strings);
#endif
	}
	else
	{
		while(p!=NULL)
		{
			if(ImageCmp(p->s, s, info->imageSize)==0)
			{
#ifdef MSG_TEXT
				printf("Installed.\n");
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
				p->n = (HASH *)malloc(sizeof(HASH));
				p = (HASH *)p->n;
				j = info->imageSize;
				p->s = (char *)malloc(j);
				ImageCpy(p->s, s, j);
				p->a = info->words;
				p->n = NULL;
				info->words++;
				info->strings += j;
#ifdef MSG_TEXT
				printf("Lookup and install.(Words:%d, Strings:%d)\n", info->words, info->strings);
#endif
				return p;
			}
		}
	}

	return p;
}


//==============================================================
static _INLINE void FreeHashString(IHASH *info, HASH *hp)
//--------------------------------------------------------------
// �o�^������̔j��
//--------------------------------------------------------------
// in:	info = �A�N�Z�X�n���h��
//		hp   = �폜����l
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	info->strings -= strlen(hp->s) + 1;
	info->words--;
//	printf("Uninstall string: '%s' (%d)\n", hp->s, strlen(hp->s));
	free(hp->s);
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
			free(h);
			*hh = NULL;

			return;
		}
		else
		{
			if(strcmp(h->s, s)==0)
			{
				*hh = (HASH *)h->n;				/* ���̃|�C���^�[���q�� */
				FreeHashString(info, h);
				free(h);
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
static _INLINE void FreeHashImage(IHASH *info, HASH *hp)
//--------------------------------------------------------------
// �o�^�C���[�W�̔j��
//--------------------------------------------------------------
// in:	info = �A�N�Z�X�n���h��
//		hp   = �폜����l
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	info->strings -= info->imageSize;
	info->words--;
	free(hp->s);
}

//==============================================================
void UninstallImage(IHASH *info, const char *s)
//--------------------------------------------------------------
// �o�^�C���[�W���폜
//--------------------------------------------------------------
// in:	info = �A�N�Z�X�n���h��
//		hp   = �폜����l
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	HASH	*h, **hh;

	hh = &info->hashTop[ CreateHashImage(s, info->imageSize) & info->hashMask ];

	h = *hh;
	while(TRUE)
	{
		if(h->n==NULL)
		{
			FreeHashImage(info, h);
			h->s = NULL;
			*hh = NULL;

			return;
		}
		else
		{
			if(ImageCmp(h->s, s, info->imageSize)==0)
			{
				*hh = (HASH *)h->n;
				FreeHashImage(info, h);
				free(h);

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

	ip = (HASH **)malloc(sizeof(HASH *) * (info->words + 1));
	j = 0;
	for(i=0; i<info->hashMax; ++i)
	{
		if((p = info->hashTop[i])!=NULL)
		{
			do
			{
				if(p->s)
				{
					ip[j] = p;
					j++;
				}
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
		if(strcmp(v[i]->s, v[left]->s) < 0)
			SwapHash(v, ++last, i);
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
		fprintf(fp, "(%5d) $%08X, $%08X : '%s'\n", i, p->a, p->b, p->s);
		i++;
	}
	free(pp);
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
				i++;
		}
	}

	nhpp = (HASH **)malloc(sizeof(HASH *) * (i + 2));

	if(i)
	{
		i = 0;
		while((nhp = hpp[i])!=NULL)
		{
			nhpp[i] = nhp;
			++i;
		} 
		free(hpp);
	}
	nhpp[i] = hp;
	nhpp[i + 1] = NULL;

	return nhpp;
}

