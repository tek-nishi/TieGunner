
/*
  
  TODO:ファイルサイズ4GB～サポート

*/

#include "co_zlib.h"
#include "co_memory.h"
#include "co_misc.h"


#define ZINBUFSIZ  32768							// 入力バッファサイズ
#define ZOUTBUFSIZ 32768							// 出力バッファサイズ
#define COMP_LEVEL Z_DEFAULT_COMPRESSION			// 圧縮レベル


// zlib用 malloc
static void *Zmalloc(void *opaque, unsigned int item, unsigned int size)
{
	return heapMalloc(item * size, "zlib");
}

// zlib用 free
static void Zfree(void *opaque, void *adrs)
{
	Free(adrs);
}

// zlib による圧縮
void *ZlibEncode(void *ptr, int size)
{
	z_stream z;
	int outsize;
	int flush, status;
	int in_size, out_size;
	char *src, *dst;
	char *zoutbuf;

	// 出力バッファの確保
	//--------------------
	zoutbuf = (char *)fsMalloc(ZOUTBUFSIZ, "zoutbuf");

	z.zalloc = Zmalloc;							// 独自実装した関数を指定
	z.zfree  = Zfree;
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
	z.next_in   = NULL;
	z.avail_out = ZOUTBUFSIZ;
	z.next_out  = (Bytef *)zoutbuf;

	src = (char *)ptr;
	in_size = size;

	dst = (char *)fsMalloc(sizeof(u_int) * 2, "zlib");
	ASSERT(dst);
	*(u_int *)dst = htonl(size);					// 元データサイズを記録

	out_size = sizeof(u_int) * 2;
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
			src += z.avail_in;
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

		if(z.avail_out == 0 || status == Z_STREAM_END)
		{
			outsize = ZOUTBUFSIZ - z.avail_out;

			// データを出力
			//--------------
			dst = (char *)fsRealloc(dst, out_size + outsize, "zlib");
			ASSERT(dst);
			memcpy(dst + out_size, zoutbuf, outsize);
			out_size += (int)outsize;

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

	*(u_int *)(dst + sizeof(u_int)) = htonl(out_size); // 圧縮後のサイズを記録
	Free(zoutbuf);										 // 確保していたバッファを開放

	return dst;
}

//==============================================================
void *_ZlibDecode(void *ptr, int size, int mem)
//--------------------------------------------------------------
// zlib による展開
//--------------------------------------------------------------
// in:	ptr  = 展開するデータ
//		size = データサイズ(圧縮後)
//		mem  = メモリ領域
//--------------------------------------------------------------
// out:	展開されたデータが格納されているポインタ
//==============================================================
{
	z_stream z;
	int outsize;
	int status;
	int in_size, out_size;
	char *src, *dst;
	char *zoutbuf;

	// 出力バッファの確保
	//--------------------
	zoutbuf = (char *)fsMalloc(ZOUTBUFSIZ, "zoutbuf");

	z.zalloc = Zmalloc;							// 独自実装した関数を指定
	z.zfree  = Zfree;
	z.opaque = Z_NULL;

#ifndef RAWMODE_NOT_SUPPORTED
#ifndef DEF_WBITS
	#define DEF_WBITS MAX_WBITS	/* from 'zutil.h' */
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

	src = (char *)ptr + 8;
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
			PRINTF("inflate: %s(%d)\n", (z.msg != NULL) ? z.msg : "Unknown error", status);
			Free(zoutbuf);
			FreeWork(dst);

			return NULL;
		}

		if((z.avail_out == 0) || (status == Z_STREAM_END))
		{
			outsize = ZOUTBUFSIZ - z.avail_out;

			// データを出力
			//--------------
			if(!dst)
			{
				dst = (char *)MemMalloc(mem, outsize, "zlib");
				ASSERT(dst);
			}
			else
			{
				dst = (char *)MemRealloc(mem, dst, out_size + outsize, "zlib");
				ASSERT(dst);
			}
			memcpy(dst + out_size, zoutbuf, outsize);
			out_size += (int)outsize;

			z.avail_out = ZOUTBUFSIZ;
			z.next_out  = (Bytef *)zoutbuf;
		}
	}
	while(status != Z_STREAM_END);

	if(inflateEnd(&z) != Z_OK)
	{
		PRINTF("inflateEnd: %s\n", (z.msg != NULL) ? z.msg : "Unknown error");
		Free(zoutbuf);
		FreeWork(dst);

		return NULL;
	}
	Free(zoutbuf);

	return dst;
}

// 圧縮後のデータサイズ
int ZlibEncodeSize(void *ptr)
{
	return ntohl(*((u_int *)ptr + 1));
}

// 圧縮前のデータサイズ
int ZlibDecodeSize(void *ptr)
{
	return ntohl(*((u_int *)ptr));
}

