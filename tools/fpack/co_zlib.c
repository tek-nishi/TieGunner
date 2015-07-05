//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
//  < TEST >
//
//--------------------------------------------------------------
//
//	zlib �������܂�
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  $Id: co_file.c,v 1.6 2004/02/13 09:07:52 nishi Exp $
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/********************************************/
/*           �C���N���[�h�t�@�C��           */
/********************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <zlib.h>

#include "default.h"
#include "misc.h"
#include "co_zlib.h"


/********************************************/
/*             �萔�E�}�N���錾             */
/********************************************/
#define ZINBUFSIZ		(32768*8)				// ���̓o�b�t�@�T�C�Y
#define ZOUTBUFSIZ		(32768*8)				// �o�̓o�b�t�@�T�C�Y

#define COMP_LEVEL  Z_DEFAULT_COMPRESSION		// ���k���x��

#define PRINTF  printf
#define ASSERT  assert

// �������Ǘ�
#define fsMalloc(a, b)      malloc(a)
#define fsRealloc(a, b, c)  realloc(a, b)
#define Free(a)             free(a)


/********************************************/
/*                �\���̐錾                */
/********************************************/


/********************************************/
/*                 �ϐ��錾                 */
/********************************************/
//static char zoutbuf[ZOUTBUFSIZ];


/********************************************/
/*                �v���O����                */
/********************************************/

//==============================================================
void *ZlibEncode(void *ptr, int size)
//--------------------------------------------------------------
// zlib �ɂ�鈳�k
//--------------------------------------------------------------
// in:	ptr  = ���k����f�[�^
//		size = �f�[�^�T�C�Y
//--------------------------------------------------------------
// out:	���k���ꂽ�f�[�^���i�[����Ă���|�C���^
//==============================================================
{
	z_stream z;
	size_t outsize;
	int flush, status;
	int in_size, out_size;
	char *src, *dst;
	char *zoutbuf;

	// �o�̓o�b�t�@�̊m��
	//--------------------
	zoutbuf = fsMalloc(ZOUTBUFSIZ, "zoutbuf");

	z.zalloc = Z_NULL;							// ���̏��������@���ƁAmalloc�Afree ���g���܂��c
	z.zfree  = Z_NULL;
	z.opaque = Z_NULL;

#ifndef RAWMODE_NOT_SUPPORTED
#if MAX_MEM_LEVEL >= 8
	#define DEF_MEM_LEVEL 8						/* from 'zutil.h' */
#else
	#define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif
	/* see deflate.c(240) */
	if(deflateInit2(&z, COMP_LEVEL, Z_DEFLATED, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY) != Z_OK)
	{
		PRINTF("deflateInit: %s\n", (z.msg != NULL) ? z.msg : "Unknown error");
		Free(zoutbuf);

		return NULL;
	}
#else
	if(deflateInit(&z, COMP_LEVEL) != Z_OK)
	{
		PRINTF("deflateInit: %s\n", (z.msg != NULL) ? z.msg : "Unknown error");
		Free(zoutbuf);

		return NULL;
	}
#endif
	z.avail_in  = 0;
	z.next_out  = (Bytef *)zoutbuf;
	z.avail_out = ZOUTBUFSIZ;

	src = (char *)ptr;
	in_size = size;

	dst = fsMalloc(8, "zlib");
	ASSERT(dst);
	putvalue((unsigned char *)dst, size);		// ���f�[�^�T�C�Y���L�^

	out_size = 8;								// ���W�o�C�g�́A�f�[�^�T�C�Y�ێ��p
	flush = Z_NO_FLUSH;
	do
	{
		if(z.avail_in == 0)
		{
			// �ǂݍ��ރf�[�^�ʒu��ݒ�
			//--------------------------
			z.next_in = (Bytef *)src;
			z.avail_in = (in_size < ZINBUFSIZ) ? in_size : ZINBUFSIZ;
			if(z.avail_in < ZINBUFSIZ)
			{
				flush = Z_FINISH;
			}
			src += z.avail_in;					// ���̈ʒu������
			in_size -= z.avail_in;
		}

		status = deflate(&z, flush);
		if(status < 0)
		{
			/* error */
			PRINTF("deflate: %s\n", (z.msg != NULL) ? z.msg : "Unknown error");
			Free(zoutbuf);
			Free(dst);

			return NULL;
		}

		if((z.avail_out == 0) || (status == Z_STREAM_END))
		{
			outsize = ZOUTBUFSIZ - z.avail_out;

			// �f�[�^���o��
			//--------------
			dst = fsRealloc(dst, out_size + outsize, "zlib");
			ASSERT(dst);
			memcpy(dst + out_size, zoutbuf, outsize);
			out_size += outsize;

			z.next_out  = (Bytef *)zoutbuf;
			z.avail_out = ZOUTBUFSIZ;
		}
	}
	while(status != Z_STREAM_END);

	if(deflateEnd(&z) != Z_OK)
	{
		PRINTF("deflateEnd: %s\n", (z.msg != NULL) ? z.msg : "Unknown error");
		Free(zoutbuf);
		Free(dst);

		return NULL;
	}

	// ���k��̃T�C�Y���L�^
	//----------------------
	putvalue((unsigned char *)(dst + 4), out_size - 8);	
	Free(zoutbuf);								// �m�ۂ��Ă����o�b�t�@���J��

	return dst;
}

//==============================================================
void *ZlibDecode(void *ptr, int size)
//--------------------------------------------------------------
// zlib �ɂ��W�J
//--------------------------------------------------------------
// in:	ptr  = �W�J����f�[�^
//		size = �f�[�^�T�C�Y
//--------------------------------------------------------------
// out:	�W�J���ꂽ�f�[�^���i�[����Ă���|�C���^
//==============================================================
{
	z_stream z;
	size_t outsize;
	int status;
	int in_size, out_size;
	char *src, *dst;
	char *zoutbuf;

	// �o�̓o�b�t�@�̊m��
	//--------------------
	zoutbuf = fsMalloc(ZOUTBUFSIZ, "zoutbuf");

	z.zalloc = Z_NULL;							// ���̏��������@���ƁAmalloc�Afree ���g���܂��c
	z.zfree  = Z_NULL;
	z.opaque = Z_NULL;

#ifndef RAWMODE_NOT_SUPPORTED
#ifndef DEF_WBITS
	#define DEF_WBITS MAX_WBITS					/* from 'zutil.h' */
#endif
	/* see inflate.c(106) */
	if(inflateInit2(&z, -DEF_WBITS) != Z_OK)
	{
		PRINTF("inflateInit: %s\n", (z.msg != NULL) ? z.msg : "Unknown error");
		Free(zoutbuf);

		return NULL;
	}
#else
	if(inflateInit(&z) != Z_OK)
	{
		PRINTF("inflateInit: %s\n", (z.msg != NULL) ? z.msg : "Unknown error");
		Free(zoutbuf);

		return NULL;
	}
#endif
	z.avail_in  = 0;
	z.next_in   = Z_NULL;
	z.avail_out = ZOUTBUFSIZ;
	z.next_out  = (Bytef *)zoutbuf;

	src = (char *)ptr + 8;						// ���W�o�C�g�́A�f�[�^�T�C�Y�ێ��p
	in_size = size;
	dst = NULL;
	out_size = 0;
	do
	{
		if(z.avail_in == 0)
		{
			// �ǂݍ��ރf�[�^�ʒu��ݒ�
			//--------------------------
			z.next_in  = (Bytef *)src;
			z.avail_in = (in_size < ZINBUFSIZ) ? in_size : ZINBUFSIZ;
			src += z.avail_in;
			in_size -= z.avail_in;
		}

		status = inflate(&z, Z_NO_FLUSH);
		if(status < 0)
		{
			/* error */
			PRINTF("inflate: %s\n", (z.msg != NULL) ? z.msg : "Unknown error");
			Free(zoutbuf);
			if(dst)
				Free(dst);

			return NULL;
		}

		if(z.avail_out == 0 || status == Z_STREAM_END)
		{
			outsize = ZOUTBUFSIZ - z.avail_out;

			// �f�[�^���o��
			//--------------
			if(!dst)
			{
				dst = fsMalloc(outsize, "zlib");
				ASSERT(dst);
			}
			else
			{
				dst = fsRealloc(dst, out_size + outsize, "zlib");
				ASSERT(dst);
			}
			memcpy(dst + out_size, zoutbuf, outsize);
			out_size += outsize;

			z.next_out  = (Bytef *)zoutbuf;
			z.avail_out = ZOUTBUFSIZ;
		}
	}
	while(status != Z_STREAM_END);

	if(inflateEnd(&z) != Z_OK)
	{
		PRINTF("inflateEnd: %s\n", (z.msg != NULL) ? z.msg : "Unknown error");
		Free(zoutbuf);
		if(dst)
			Free(dst);

		return NULL;
	}

	return dst;
}

//==============================================================
int ZlibEncodeSize(void *ptr)
//--------------------------------------------------------------
// ���k��̃f�[�^�T�C�Y
//--------------------------------------------------------------
// in:	ptr = ���k���ꂽ�f�[�^
//--------------------------------------------------------------
// out:	�f�[�^�T�C�Y
//==============================================================
{
	return getvalue((unsigned char *)ptr + 4);
}

//==============================================================
int ZlibDecodeSize(void *ptr)
//--------------------------------------------------------------
// ���k�O�̃f�[�^�T�C�Y
//--------------------------------------------------------------
// in:	ptr = ���k���ꂽ�f�[�^
//--------------------------------------------------------------
// out:	�f�[�^�T�C�Y
//==============================================================
{
	return getvalue((unsigned char *)ptr);
}
