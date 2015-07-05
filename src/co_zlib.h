
/*

	zlib を利用したデータの圧縮/展開を行います。

	圧縮されたデータの先頭４バイトに、圧縮前と圧縮後のデータサイズが
	記録されています。

*/

//==============================================================
#ifndef CO_ZLIB_H
#define CO_ZLIB_H
//==============================================================

#include "co_common.h"
#if defined (_MSC_VER)
#ifdef DEBUG
#pragma comment (lib, "zlibd.lib")
#else
#pragma comment (lib, "zlib.lib")
#endif
#endif

#ifdef __cplusplus
extern              "C"
{
#endif

// zlib による圧縮
extern void *ZlibEncode(void *ptr, int size);
// zlib による展開
#define ZlibDecode(ptr, size)  _ZlibDecode(ptr, size, MEM_APP)
// zlib による展開(メモリ指定)
extern void *_ZlibDecode(void *ptr, int size, int mem);

// 圧縮後のデータサイズ
extern int ZlibEncodeSize(void *ptr);
// 圧縮前のデータサイズ
extern int ZlibDecodeSize(void *ptr);

#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================

