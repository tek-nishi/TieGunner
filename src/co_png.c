
#include "co_png.h"
#include "co_memory.h"
#include "co_file.h"


#define USE_USER_MEMORY							// 独自実装したメモリ管理処理を利用する

// メモリ確保
//------------
#define pngMalloc(a, b)  MemMalloc(MEM_FS, a, b)


//==============================================================
static png_voidp libpng_Malloc(png_structp png_ptr, png_size_t size)
//--------------------------------------------------------------
// メモリ確保
//--------------------------------------------------------------
// in:	png_ptr = アクセスハンドル
//		size    = サイズ
//--------------------------------------------------------------
// out:	確保したポインタ
//==============================================================
{
	return pngMalloc(size, "libpng");
}

//==============================================================
static void libpng_Free(png_structp png_ptr, png_voidp ptr)
//--------------------------------------------------------------
// メモリ開放
//--------------------------------------------------------------
// in:	ptr = 開放するポインタ
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	Free(ptr);
}

//==============================================================
static void readFunc(png_struct *hdl, png_bytep buf, png_size_t size)
//--------------------------------------------------------------
// ストリーム読み込み処理
//--------------------------------------------------------------
// in:	hdl  = ハンドル
//		buf  = データポインタ
//		size = データサイズ
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
    unsigned char **p;

    p = (unsigned char **)png_get_io_ptr(hdl);
    memcpy(buf, *p, size);
    *p += (int)size;
}

//==============================================================
sPNG *PngRead(void *ptr, size_t size)
//--------------------------------------------------------------
// データ読み込み
//--------------------------------------------------------------
// in:	ptr  = データポインタ
//		size = データサイズ
//--------------------------------------------------------------
// out:	アクセスハンドル(NULL = 失敗)
//==============================================================
{
	png_struct *hdl;
	png_info *info;
	u_char *filepos;
	png_uint_32 width, height;
	int depth, type;
	u_char **image;
	int num_palette;
	png_colorp palette;
	sPalette *pal;
	size_t w_size;

	int trans_ex;
	png_bytep trans;
	png_color_16p trans_16;
	int trans_num;

	int i;
	unsigned int h;
	sPNG *hdr;

	// ヘッダチェック
	//----------------
	if(!png_check_sig((png_bytep)ptr, size))
		return NULL;

	// 基本構造体の初期化
	//--------------------
#ifdef USE_USER_MEMORY
	hdl = png_create_read_struct_2(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL, NULL, libpng_Malloc, libpng_Free);
#else
	hdl = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
#endif
	if(!hdl)
		return NULL;
	info = png_create_info_struct(hdl);
	if(!info)
	{
		png_destroy_read_struct(&hdl, NULL, NULL);
		return NULL;
	}

	filepos = (u_char *)ptr;
	png_set_read_fn(hdl, (png_voidp)&filepos, (png_rw_ptr)readFunc);

	png_read_info(hdl, info);

	png_get_IHDR(hdl, info, &width, &height, &depth, &type, NULL, NULL, NULL);

	if(depth == 16)
	{
		png_set_strip_16(hdl);					// 16bit -> 8bit
	}
	else
	if(depth == 4)
	{
		png_set_packing(hdl);					// 4bit -> 8bit
	}
	else
	if(depth != 8)
	{
		PRINTF("Format error(%d)\n", depth);
		return NULL;
	}

#if 0
	if(type != PNG_COLOR_TYPE_PALETTE)
	{
		PRINTF("Format error(not palette type)\n");
		return NULL;
	}
#endif

#if 0											// OpenGLで使う場合は不要
	// アルファ付きPNG
	//-----------------
	if(type & PNG_COLOR_MASK_ALPHA)
		png_set_bgr(hdl);
#endif

	// 透明色情報取得
	//----------------
	trans_ex = png_get_tRNS(hdl, info, &trans, &trans_num, &trans_16);

	// 設定を反映
	//------------
	png_read_update_info(hdl, info);

	// パレットを取得
	//----------------
	if(png_get_PLTE(hdl, info, &palette, &num_palette))
	{
//		PRINTF("palette: %d\n", num_palette);

		pal = (sPalette *)pngMalloc(sizeof(sPalette) * num_palette, "sPalette");
		ASSERT(pal);

		for(i = 0; i < num_palette; ++i)
		{
			pal[i].red   = palette[i].red;
			pal[i].green = palette[i].green;
			pal[i].blue  = palette[i].blue;
			if(trans_ex && trans_num)
			{
				// 取得した情報に従って設定
				//--------------------------
				pal[i].alpha = *trans;
				++trans;
				--trans_num;
			}
			else
			{
				// 不透明
				//--------
				pal[i].alpha = 255;
			}
		}
	}
	else
	{
		pal = NULL;
	}

	// データ格納領域を確保
	//----------------------
	image = (u_char **)pngMalloc(sizeof(u_char *) * height, "png"); 
	ASSERT(image);
	w_size = png_get_rowbytes(hdl, info);
	for(h = 0; h < height; ++h)
	{
		image[h] = (u_char *)pngMalloc(w_size, "png");
		ASSERT(image[h]);
	}

	// データを読み込む
	//------------------
	png_read_image(hdl, image);

	// 読み込み終了処理
	//------------------
	png_read_end(hdl, info);

	// 後始末
	//--------
	png_destroy_read_struct(&hdl, &info, NULL);

	// アクセスハンドルを作成
	//------------------------
	hdr = (sPNG *)pngMalloc(sizeof(sPNG), "sPNG");
	ASSERT(hdr);

	hdr->type = type;
	hdr->width = width;
	hdr->height = height;
	hdr->image = (u_char *)pngMalloc(w_size * height, "sPNG image");
	hdr->palnum = num_palette;
	hdr->pal = pal;

	// イメージをコピー
	//------------------
	for(h = 0; h < height; ++h)
	{
		memcpy(&hdr->image[w_size * h], image[h], w_size);
		Free(image[h]);
	}
	Free(image);

	return hdr;
}

//==============================================================
void PngDestroy(sPNG *hdr)
//--------------------------------------------------------------
// ハンドル破棄
//--------------------------------------------------------------
// in:	hdr = アクセスハンドル
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	Free(hdr->image);
	FreeWork(hdr->pal);
	Free(hdr);
}


//==============================================================
static void PNGAPI writeFunc(png_structp png_ptr, png_bytep data, png_size_t length)
//--------------------------------------------------------------
// 
//--------------------------------------------------------------
// in:	
//--------------------------------------------------------------
// out:	
//==============================================================
{
	sFILE *fp;

	fp = (sFILE *)png_get_io_ptr(png_ptr);
	FsWrite(fp, data, length);
}

//==============================================================
static void PNGAPI flushFunc(png_structp png_ptr)
//--------------------------------------------------------------
// 
//--------------------------------------------------------------
// in:	
//--------------------------------------------------------------
// out:	
//==============================================================
{
	sFILE *fp;

	fp = (sFILE *)png_get_io_ptr(png_ptr);
	FsClose(fp);
}

//==============================================================
void PngWrite(char *file, int w, int h, u_char *ptr)
//--------------------------------------------------------------
// データ出力
// 32bit → 32bit
//--------------------------------------------------------------
// in:	file = ファイル名
//		w, h = データサイズ
//		ptr  = データポインタ
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	sFILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;
	png_color_8 sig_bit;
	png_text text_ptr[1];
	png_bytep *row_pointers;
	int k;

#ifdef USE_USER_MEMORY
	png_ptr = png_create_write_struct_2(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL, NULL, libpng_Malloc, libpng_Free);
#else
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
#endif
	if(!png_ptr)
	{
		return;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr)
	{
		png_destroy_write_struct(&png_ptr, png_infopp_NULL);
		return;
	}

	// ファイルの新規作成
	//--------------------
	fp = FsCreate(file);
	ASSERT(fp);

	png_set_write_fn(png_ptr, (png_voidp)fp, writeFunc, flushFunc);
	png_set_IHDR(png_ptr, info_ptr, w, h, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_BASE);

	sig_bit.red = 8;
	sig_bit.green = 8;
	sig_bit.blue = 8;
	sig_bit.alpha = 0;
	png_set_sBIT(png_ptr, info_ptr, &sig_bit);

	text_ptr[0].key = "Description";
	text_ptr[0].text = "ktcDIB::Save() Data";
	text_ptr[0].compression = PNG_TEXT_COMPRESSION_NONE;
	png_set_text(png_ptr, info_ptr, text_ptr, 1);

	png_write_info(png_ptr, info_ptr);
	png_set_bgr(png_ptr);

	row_pointers = (png_bytep *)fsMalloc(sizeof(png_bytep *) * h, "png_bytep");
	ASSERT(row_pointers);
	for(k = 0; k < h; ++k)
	{
		png_byte *p;

		p = (png_byte *)fsMalloc(sizeof(png_byte) * w * 3, "png_byte");
		ASSERT(p);
		row_pointers[k] = (png_bytep)p;
		memcpy(p, ptr, w * 3);
		ptr += w * 3;
	}

	png_write_image(png_ptr, row_pointers);
	png_write_end(png_ptr, info_ptr);

	for(k = 0; k < h; ++k)
	{
		Free(row_pointers[k]);
	}
	Free(row_pointers);

	png_destroy_write_struct(&png_ptr, &info_ptr);
	FsClose(fp);
}

