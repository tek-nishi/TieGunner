//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
//  < TEST >
//
//--------------------------------------------------------------
//
//	zlib を扱います
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  $Id: co_file.c,v 1.6 2004/02/13 09:07:52 nishi Exp $
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/********************************************/
/*           インクルードファイル           */
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
/*             定数・マクロ宣言             */
/********************************************/
#define ZINBUFSIZ		(32768*8)				// 入力バッファサイズ
#define ZOUTBUFSIZ		(32768*8)				// 出力バッファサイズ

#define COMP_LEVEL  Z_DEFAULT_COMPRESSION		// 圧縮レベル

#define PRINTF  printf
#define ASSERT  assert

// メモリ管理
#define fsMalloc(a, b)      malloc(a)
#define fsRealloc(a, b, c)  realloc(a, b)
#define Free(a)             free(a)


/********************************************/
/*                構造体宣言                */
/********************************************/


/********************************************/
/*                 変数宣言                 */
/********************************************/
//static char zoutbuf[ZOUTBUFSIZ];


/********************************************/
/*                プログラム                */
/********************************************/

//==============================================================
void *ZlibEncode(void *ptr, int size)
//--------------------------------------------------------------
// zlib による圧縮
//--------------------------------------------------------------
// in:	ptr  = 圧縮するデータ
//		size = データサイズ
//--------------------------------------------------------------
// out:	圧縮されたデータが格納されているポインタ
//==============================================================
{
	z_stream z;
	size_t outsize;
	int flush, status;
	int in_size, out_size;
	char *src, *dst;
	char *zoutbuf;

	// 出力バッファの確保
	//--------------------
	zoutbuf = fsMalloc(ZOUTBUFSIZ, "zoutbuf");

	z.zalloc = Z_NULL;							// この初期化方法だと、malloc、free が使われます…
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
	putvalue((unsigned char *)dst, size);		// 元データサイズを記録

	out_size = 8;								// 頭８バイトは、データサイズ保持用
	flush = Z_NO_FLUSH;
	do
	{
		if(z.avail_in == 0)
		{
			// 読み込むデータ位置を設定
			//--------------------------
			z.next_in = (Bytef *)src;
			z.avail_in = (in_size < ZINBUFSIZ) ? in_size : ZINBUFSIZ;
			if(z.avail_in < ZINBUFSIZ)
			{
				flush = Z_FINISH;
			}
			src += z.avail_in;					// 次の位置を準備
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

			// データを出力
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

	// 圧縮後のサイズを記録
	//----------------------
	putvalue((unsigned char *)(dst + 4), out_size - 8);	
	Free(zoutbuf);								// 確保していたバッファを開放

	return dst;
}

//==============================================================
void *ZlibDecode(void *ptr, int size)
//--------------------------------------------------------------
// zlib による展開
//--------------------------------------------------------------
// in:	ptr  = 展開するデータ
//		size = データサイズ
//--------------------------------------------------------------
// out:	展開されたデータが格納されているポインタ
//==============================================================
{
	z_stream z;
	size_t outsize;
	int status;
	int in_size, out_size;
	char *src, *dst;
	char *zoutbuf;

	// 出力バッファの確保
	//--------------------
	zoutbuf = fsMalloc(ZOUTBUFSIZ, "zoutbuf");

	z.zalloc = Z_NULL;							// この初期化方法だと、malloc、free が使われます…
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

	src = (char *)ptr + 8;						// 頭８バイトは、データサイズ保持用
	in_size = size;
	dst = NULL;
	out_size = 0;
	do
	{
		if(z.avail_in == 0)
		{
			// 読み込むデータ位置を設定
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

			// データを出力
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
// 圧縮後のデータサイズ
//--------------------------------------------------------------
// in:	ptr = 圧縮されたデータ
//--------------------------------------------------------------
// out:	データサイズ
//==============================================================
{
	return getvalue((unsigned char *)ptr + 4);
}

//==============================================================
int ZlibDecodeSize(void *ptr)
//--------------------------------------------------------------
// 圧縮前のデータサイズ
//--------------------------------------------------------------
// in:	ptr = 圧縮されたデータ
//--------------------------------------------------------------
// out:	データサイズ
//==============================================================
{
	return getvalue((unsigned char *)ptr);
}
