/*

  テクスチャ管理

*/

#include "co_texture.h"
#include "co_objlink.h"
#include "co_png.h"
#include "co_memory.h"
#include "co_misc.h"
#include "co_fileutil.h"
#include "co_hash16.h"
#include "co_strings.h"


#define TEXTURE_NUM	  256							// 最大管理数
#define PIXEL_SIZE	  sizeof(Pixcel)				// 1ドットあたりのバイト数
#define PIXEL_SIZE_16 2

typedef struct {
	u_char r, g, b, a;
} Pixcel;

struct _sTexture {
	char id_str[ID_MAXLEN];
	GLuint id;
	
	int w;										// 幅(2のべき乗)
	int h;										// 高さ(2のべき乗)
	int orig_w;									// オリジナルの幅
	int orig_h;									// オリジナルの高さ

	REAL u;										// 最大 U 値
	REAL v;										// 最大 V 値

	int reference;								// リファレンスカウンタ
};


static sLink *tex_link;
static sHASH *tex_hash;


//==============================================================
static void l_textureCopy32(sTexture *texture, sPNG *png)
//--------------------------------------------------------------
// テクスチャコピー(32bit版)
// RGBAタイプ
//--------------------------------------------------------------
// in:	texture = ハンドル
//		png     = PNGハンドル
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	int h;
	Pixcel *png_img;
	Pixcel *image;
	Pixcel matPixcel = { 0, 0, 0, 0 };				/* FIXME:微妙 */

	png_img = (Pixcel *)png->image;
	image = (Pixcel *)fsMalloc(texture->h * texture->w * PIXEL_SIZE, "image");
	ASSERT(image);
	for(h = 0; h < png->height; ++h)
	{
		memcpy(image + h * texture->w, png_img + h * png->width, png->width * PIXEL_SIZE);
		Pixcel *pixcel = image + h * texture->w;
		for(int i = 0; i < png->width; i += 1)
		{
			if(pixcel->a == 0)
			{
				*pixcel = matPixcel;
			}
			pixcel += 1;
		}
		/* FIXME:マットカラー */
		
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->w, texture->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	Free(image);
}

//==============================================================
static void l_textureCopy32i(sTexture *texture, sPNG *png)
//--------------------------------------------------------------
// テクスチャコピー
// ※インデックスタイプ
//--------------------------------------------------------------
// in:	texture = ハンドル
//		png     = PNGハンドル
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	int h;
	int i;
	u_int *image;

	image = (u_int *)fsMalloc(texture->h * texture->w * PIXEL_SIZE, "image");
	ASSERT(image);
	for(h = 0; h < png->height; ++h)
	{
		for(i = 0; i < png->width; ++i)
		{
			sPalette *pal;

			pal = &png->pal[*(png->image + i + h * png->width)];
			*(image + i + h * texture->w) = (pal->red << 24) + (pal->green << 16) + (pal->blue << 8) + pal->alpha;
		}
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->w, texture->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	Free(image);
}

//==============================================================
static void l_createImage(sTexture *texture, sPNG *png)
//--------------------------------------------------------------
// イメージ生成
//--------------------------------------------------------------
// in:	texture = ハンドル
//		png     = PNGハンドル
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	GLint pack;
	GLint unpack;

	texture->orig_w = png->width;
	texture->orig_h = png->height;
	texture->w = int2pow(png->width);
	texture->h = int2pow(png->height);
	texture->u = (REAL)texture->orig_w / (REAL)texture->w;
	texture->v = (REAL)texture->orig_h / (REAL)texture->h;

	// バッファの読み取りバイト数の設定
	glGetIntegerv(GL_PACK_ALIGNMENT, &pack);
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &unpack);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	if(png->type == PNG_COLOR_TYPE_PALETTE)
	{
		// インデックスタイプ
		l_textureCopy32i(texture, png);
	}
	else
	{
		// RGBAタイプ
	//	l_textureCopy16(texture, png);
		l_textureCopy32(texture, png);
	}

	// バッファの読み取りバイト数の再設定
	glPixelStorei(GL_PACK_ALIGNMENT, pack);
	glPixelStorei(GL_UNPACK_ALIGNMENT, unpack);
}

//==============================================================
static void l_texRead(sTexture *texture, char *file)
//--------------------------------------------------------------
// テクスチャ読み込み
//--------------------------------------------------------------
// in:	texture   = ハンドル
//		file      = ファイル名
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	void *ptr;
	sPNG *png;

	ptr = MmFileLoadB(file);
	png = PngRead(ptr, MmFileGetSize());
	ASSERT(png);
	Free(ptr);

	glGenTextures(1, &texture->id);
	glBindTexture(GL_TEXTURE_2D, texture->id);
	l_createImage(texture, png);

	PngDestroy(png);
}

//==============================================================
void TexInit(void)
//--------------------------------------------------------------
// 初期化
//--------------------------------------------------------------
// in:	なし
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	tex_link = ObjLinkCreate(sizeof(sTexture), TEXTURE_NUM, MEM_SYS, FALSE);
	tex_hash = HashCreate("texture");

	SYSINFO(".... texture initialize");
}

//==============================================================
void TexFin(void)
//--------------------------------------------------------------
// 終了
//--------------------------------------------------------------
// in:	なし
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	void *p = ObjLinkGetTop(tex_link);
	while(p != NULL)
	{
		sTexture *tex = (sTexture *)p;
		glDeleteTextures(1, &tex->id);
		
		p = ObjLinkGetNext(p);
	}
	ObjLinkDestroy(tex_link);
	HashKill(tex_hash);

	SYSINFO(".... texture finish");
}

//==============================================================
sTexture *TexRead(char *file)
//--------------------------------------------------------------
// テクスチャ読み込み
//--------------------------------------------------------------
// in:	file = ファイル名
//--------------------------------------------------------------
// out:	ハンドル
//==============================================================
{
	char name[FNAME_MAXLEN];
	sTexture *texture;

	GetPathName(file, NULL, name, TRUE);		// 拡張子抜きのファイル名→識別子
	texture = (sTexture *)HashGet(tex_hash, name);
	if(texture)
	{
		texture->reference += 1;
	}
	else
	{
		texture = (sTexture *)ObjLinkNew(tex_link);
		ASSERT(texture);
		HashAdd(tex_hash, name, texture);

		STRCPY16(texture->id_str, name);
		texture->reference = 1;

		l_texRead(texture, file);
	}

	return texture;
}

//==============================================================
sTexture *TexClone(sTexture *texture)
//--------------------------------------------------------------
// テクスチャのクローン
//--------------------------------------------------------------
// in:	texture = クローン元
//--------------------------------------------------------------
// out:	ハンドル
//==============================================================
{
	texture->reference += 1;
	return texture;
}

//==============================================================
void TexDestroy(sTexture *texture)
//--------------------------------------------------------------
// テクスチャ破棄
//--------------------------------------------------------------
// in:	texture = ハンドル
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	texture->reference--;
	ASSERT(texture->reference >= 0);
	if(!texture->reference)
	{
		glDeleteTextures(1, &texture->id);
		HashDel(tex_hash, texture->id_str);
		ObjLinkDel(tex_link, texture);
	}
}

//==============================================================
sTexture *TexGetFromName(char *id_str)
//--------------------------------------------------------------
// ハンドルを取得
//--------------------------------------------------------------
// in:	id_str = 識別子
//--------------------------------------------------------------
// out:	ハンドル(NULL = エラー)
//==============================================================
{
	return (sTexture *)HashGet(tex_hash, id_str);
}

//==============================================================
char *TexGetName(sTexture *texture)
//--------------------------------------------------------------
// 識別子を取得
//--------------------------------------------------------------
// in:	texture = ハンドル
//--------------------------------------------------------------
// out:	識別子
//==============================================================
{
	return texture->id_str;
}

//==============================================================
void TexGetSize(FVector2 *res, sTexture *texture)
//--------------------------------------------------------------
// サイズを取得
//--------------------------------------------------------------
// in:	res     = 結果を格納するポインタ
//		texture = ハンドル
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	res->x = texture->orig_w;
	res->y = texture->orig_h;
}

//==============================================================
void TexGetUV(FVector2 *res, sTexture *texture)
//--------------------------------------------------------------
// UV最大値を取得
//--------------------------------------------------------------
// in:	res     = 結果を格納するポインタ
//		texture = ハンドル
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	res->x = texture->u;
	res->y = texture->v;
}

void TexBind(sTexture *texture)
{
	glBindTexture(GL_TEXTURE_2D, texture->id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
}

