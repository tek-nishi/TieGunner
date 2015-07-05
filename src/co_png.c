
#include "co_png.h"
#include "co_memory.h"
#include "co_file.h"


#define USE_USER_MEMORY							// �Ǝ����������������Ǘ������𗘗p����

// �������m��
//------------
#define pngMalloc(a, b)  MemMalloc(MEM_FS, a, b)


//==============================================================
static png_voidp libpng_Malloc(png_structp png_ptr, png_size_t size)
//--------------------------------------------------------------
// �������m��
//--------------------------------------------------------------
// in:	png_ptr = �A�N�Z�X�n���h��
//		size    = �T�C�Y
//--------------------------------------------------------------
// out:	�m�ۂ����|�C���^
//==============================================================
{
	return pngMalloc(size, "libpng");
}

//==============================================================
static void libpng_Free(png_structp png_ptr, png_voidp ptr)
//--------------------------------------------------------------
// �������J��
//--------------------------------------------------------------
// in:	ptr = �J������|�C���^
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	Free(ptr);
}

//==============================================================
static void readFunc(png_struct *hdl, png_bytep buf, png_size_t size)
//--------------------------------------------------------------
// �X�g���[���ǂݍ��ݏ���
//--------------------------------------------------------------
// in:	hdl  = �n���h��
//		buf  = �f�[�^�|�C���^
//		size = �f�[�^�T�C�Y
//--------------------------------------------------------------
// out:	�Ȃ�
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
// �f�[�^�ǂݍ���
//--------------------------------------------------------------
// in:	ptr  = �f�[�^�|�C���^
//		size = �f�[�^�T�C�Y
//--------------------------------------------------------------
// out:	�A�N�Z�X�n���h��(NULL = ���s)
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

	// �w�b�_�`�F�b�N
	//----------------
	if(!png_check_sig((png_bytep)ptr, size))
		return NULL;

	// ��{�\���̂̏�����
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

#if 0											// OpenGL�Ŏg���ꍇ�͕s�v
	// �A���t�@�t��PNG
	//-----------------
	if(type & PNG_COLOR_MASK_ALPHA)
		png_set_bgr(hdl);
#endif

	// �����F���擾
	//----------------
	trans_ex = png_get_tRNS(hdl, info, &trans, &trans_num, &trans_16);

	// �ݒ�𔽉f
	//------------
	png_read_update_info(hdl, info);

	// �p���b�g���擾
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
				// �擾�������ɏ]���Đݒ�
				//--------------------------
				pal[i].alpha = *trans;
				++trans;
				--trans_num;
			}
			else
			{
				// �s����
				//--------
				pal[i].alpha = 255;
			}
		}
	}
	else
	{
		pal = NULL;
	}

	// �f�[�^�i�[�̈���m��
	//----------------------
	image = (u_char **)pngMalloc(sizeof(u_char *) * height, "png"); 
	ASSERT(image);
	w_size = png_get_rowbytes(hdl, info);
	for(h = 0; h < height; ++h)
	{
		image[h] = (u_char *)pngMalloc(w_size, "png");
		ASSERT(image[h]);
	}

	// �f�[�^��ǂݍ���
	//------------------
	png_read_image(hdl, image);

	// �ǂݍ��ݏI������
	//------------------
	png_read_end(hdl, info);

	// ��n��
	//--------
	png_destroy_read_struct(&hdl, &info, NULL);

	// �A�N�Z�X�n���h�����쐬
	//------------------------
	hdr = (sPNG *)pngMalloc(sizeof(sPNG), "sPNG");
	ASSERT(hdr);

	hdr->type = type;
	hdr->width = width;
	hdr->height = height;
	hdr->image = (u_char *)pngMalloc(w_size * height, "sPNG image");
	hdr->palnum = num_palette;
	hdr->pal = pal;

	// �C���[�W���R�s�[
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
// �n���h���j��
//--------------------------------------------------------------
// in:	hdr = �A�N�Z�X�n���h��
//--------------------------------------------------------------
// out:	�Ȃ�
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
// �f�[�^�o��
// 32bit �� 32bit
//--------------------------------------------------------------
// in:	file = �t�@�C����
//		w, h = �f�[�^�T�C�Y
//		ptr  = �f�[�^�|�C���^
//--------------------------------------------------------------
// out:	�Ȃ�
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

	// �t�@�C���̐V�K�쐬
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

