/*

  �e�N�X�`���Ǘ�

*/

#include "co_texture.h"
#include "co_objlink.h"
#include "co_png.h"
#include "co_memory.h"
#include "co_misc.h"
#include "co_fileutil.h"
#include "co_hash16.h"
#include "co_strings.h"


#define TEXTURE_NUM	  256							// �ő�Ǘ���
#define PIXEL_SIZE	  sizeof(Pixcel)				// 1�h�b�g������̃o�C�g��
#define PIXEL_SIZE_16 2

typedef struct {
	u_char r, g, b, a;
} Pixcel;

struct _sTexture {
	char id_str[ID_MAXLEN];
	GLuint id;
	
	int w;										// ��(2�ׂ̂���)
	int h;										// ����(2�ׂ̂���)
	int orig_w;									// �I���W�i���̕�
	int orig_h;									// �I���W�i���̍���

	REAL u;										// �ő� U �l
	REAL v;										// �ő� V �l

	int reference;								// ���t�@�����X�J�E���^
};


static sLink *tex_link;
static sHASH *tex_hash;


//==============================================================
static void l_textureCopy32(sTexture *texture, sPNG *png)
//--------------------------------------------------------------
// �e�N�X�`���R�s�[(32bit��)
// RGBA�^�C�v
//--------------------------------------------------------------
// in:	texture = �n���h��
//		png     = PNG�n���h��
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	int h;
	Pixcel *png_img;
	Pixcel *image;
	Pixcel matPixcel = { 0, 0, 0, 0 };				/* FIXME:���� */

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
		/* FIXME:�}�b�g�J���[ */
		
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->w, texture->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	Free(image);
}

//==============================================================
static void l_textureCopy32i(sTexture *texture, sPNG *png)
//--------------------------------------------------------------
// �e�N�X�`���R�s�[
// ���C���f�b�N�X�^�C�v
//--------------------------------------------------------------
// in:	texture = �n���h��
//		png     = PNG�n���h��
//--------------------------------------------------------------
// out:	�Ȃ�
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
// �C���[�W����
//--------------------------------------------------------------
// in:	texture = �n���h��
//		png     = PNG�n���h��
//--------------------------------------------------------------
// out:	�Ȃ�
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

	// �o�b�t�@�̓ǂݎ��o�C�g���̐ݒ�
	glGetIntegerv(GL_PACK_ALIGNMENT, &pack);
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &unpack);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	if(png->type == PNG_COLOR_TYPE_PALETTE)
	{
		// �C���f�b�N�X�^�C�v
		l_textureCopy32i(texture, png);
	}
	else
	{
		// RGBA�^�C�v
	//	l_textureCopy16(texture, png);
		l_textureCopy32(texture, png);
	}

	// �o�b�t�@�̓ǂݎ��o�C�g���̍Đݒ�
	glPixelStorei(GL_PACK_ALIGNMENT, pack);
	glPixelStorei(GL_UNPACK_ALIGNMENT, unpack);
}

//==============================================================
static void l_texRead(sTexture *texture, char *file)
//--------------------------------------------------------------
// �e�N�X�`���ǂݍ���
//--------------------------------------------------------------
// in:	texture   = �n���h��
//		file      = �t�@�C����
//--------------------------------------------------------------
// out:	�Ȃ�
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
// ������
//--------------------------------------------------------------
// in:	�Ȃ�
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	tex_link = ObjLinkCreate(sizeof(sTexture), TEXTURE_NUM, MEM_SYS, FALSE);
	tex_hash = HashCreate("texture");

	SYSINFO(".... texture initialize");
}

//==============================================================
void TexFin(void)
//--------------------------------------------------------------
// �I��
//--------------------------------------------------------------
// in:	�Ȃ�
//--------------------------------------------------------------
// out:	�Ȃ�
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
// �e�N�X�`���ǂݍ���
//--------------------------------------------------------------
// in:	file = �t�@�C����
//--------------------------------------------------------------
// out:	�n���h��
//==============================================================
{
	char name[FNAME_MAXLEN];
	sTexture *texture;

	GetPathName(file, NULL, name, TRUE);		// �g���q�����̃t�@�C���������ʎq
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
// �e�N�X�`���̃N���[��
//--------------------------------------------------------------
// in:	texture = �N���[����
//--------------------------------------------------------------
// out:	�n���h��
//==============================================================
{
	texture->reference += 1;
	return texture;
}

//==============================================================
void TexDestroy(sTexture *texture)
//--------------------------------------------------------------
// �e�N�X�`���j��
//--------------------------------------------------------------
// in:	texture = �n���h��
//--------------------------------------------------------------
// out:	�Ȃ�
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
// �n���h�����擾
//--------------------------------------------------------------
// in:	id_str = ���ʎq
//--------------------------------------------------------------
// out:	�n���h��(NULL = �G���[)
//==============================================================
{
	return (sTexture *)HashGet(tex_hash, id_str);
}

//==============================================================
char *TexGetName(sTexture *texture)
//--------------------------------------------------------------
// ���ʎq���擾
//--------------------------------------------------------------
// in:	texture = �n���h��
//--------------------------------------------------------------
// out:	���ʎq
//==============================================================
{
	return texture->id_str;
}

//==============================================================
void TexGetSize(FVector2 *res, sTexture *texture)
//--------------------------------------------------------------
// �T�C�Y���擾
//--------------------------------------------------------------
// in:	res     = ���ʂ��i�[����|�C���^
//		texture = �n���h��
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	res->x = texture->orig_w;
	res->y = texture->orig_h;
}

//==============================================================
void TexGetUV(FVector2 *res, sTexture *texture)
//--------------------------------------------------------------
// UV�ő�l���擾
//--------------------------------------------------------------
// in:	res     = ���ʂ��i�[����|�C���^
//		texture = �n���h��
//--------------------------------------------------------------
// out:	�Ȃ�
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

