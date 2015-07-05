
#include "co_file.h"
#include "co_memory.h"
#include "co_os.h"
#include "co_hash.h"
#include "co_strings.h"
#include "co_zlib.h"
#include "co_misc.h"


enum enmPATH_MODE {
	PATH_IMAGE,									// �C���[�W��̃t�@�C����
	PATH_HOST,									// HOST ��̃t�@�C����
};

#define SECTOR_SIZE  2048						// �Z�N�^�T�C�Y

// malloc �֘A
//-------------
#define fioMalloc(size, name)  MemMalloc(MEM_FS, size, name)

// �C���[�W�t�@�C�����
//----------------------
#define FILE_HEADER_SIZE  (4 + sizeof(int) * 2)		// �w�b�_�T�C�Y
#define FILE_INFO_SIZE  (FNAME_MAXLEN + (sizeof(int) * 2))
#define HEADER_CODE  "FPAK"						// �w�b�_������


struct _sFILE {
	BOOL	on_image;								// TRUE = �C���[�W��̃t�@�C��
	FILE   *fd;										// �t�@�C���n���h��
	size_t	ofs;									// �擪����̃I�t�Z�b�g(�C���[�W�t�@�C���p)
	size_t	size;									// �t�@�C���T�C�Y(�Z�N�^�T�C�Y�Ő؂�グ)
	size_t  size_orig;								// �t�@�C�����T�C�Y(�Z�N�^�T�C�Y�Ő؂�グ�Ă��Ȃ��T�C�Y)
	size_t  acc_pos;								// �A�N�Z�X�ʒu
};

typedef struct {
	int ofs;									// �f�[�^�擪����̃I�t�Z�b�g
	int size;									// �t�@�C���T�C�Y(�Z�N�^�T�C�Y�Ő؂�グ)
	int size_orig;								// �t�@�C�����T�C�Y
} sFileInfo;

typedef struct {
	char	   name[FNAME_MAXLEN];					// �C���[�W�t�@�C���̃t�@�C����
	sFILE	  *fp;									// �C���[�W�t�@�C���̃t�@�C���n���h��
	sFileInfo *info;								// �C���[�W�t�@�C�����m�񃏁[�N
	IHASH	  *hash;								// �n�b�V���n���h��
} sImage;


static int imageExist;							// TRUE = �C���[�W�t�@�C�����w�肳��Ă���
static sImage imageInfo;						// �}�E���g���


//==============================================================
static const char *l_makePath(int mode, const char *name)
//--------------------------------------------------------------
// �p�X����ϊ�����
//--------------------------------------------------------------
// in:	mode = ���샂�[�h(enmPATH_MODE)
//		name = �t�@�C����
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	static char fname[FNAME_MAXLEN];

	if(mode == PATH_IMAGE)
	{
		// �C���[�W��̃t�@�C�����Ȃ�A���̂܂ܕԂ�
		return name;
	}

	// �ő咷��蒷���ꍇ�̓A�T�[�g�Ŏ~�߂�
	//--------------------------------------
	ASSERT(strlen(name) < (FNAME_MAXLEN - 2));

  char f[FNAME_MAXLEN];
	PATHCOPY(f, name);
  
	OsGetFileName(fname, f);				// �t�@�C�������@��ˑ��d�l�֕ϊ�

	return fname;
}

//==============================================================
static sFILE *l_createHandle(void)
//--------------------------------------------------------------
// �t�@�C���n���h���̎擾
// ���K�v�ȃ��[�N�����������Ă���
//--------------------------------------------------------------
// in:	�Ȃ�
//--------------------------------------------------------------
// out:	�t�@�C���n���h��
//==============================================================
{
	sFILE *fp;

	fp = (sFILE *)fioMalloc(sizeof(sFILE), "sFILE");
	ASSERT(fp);

	return fp;
}

//==============================================================
static void l_destroyHandle(sFILE *fp)
//--------------------------------------------------------------
// �t�@�C���n���h���̔j��
//--------------------------------------------------------------
// in:	fp = �t�@�C���n���h��
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	Free(fp);
}

//==============================================================
static size_t l_readFile(sFILE *fp, u_char *ptr, size_t size)
//--------------------------------------------------------------
// �f�[�^�ǂݍ���
//--------------------------------------------------------------
// in:	fp   = �t�@�C���n���h��
//		ptr  = �Ǎ���
//		size = �T�C�Y
//--------------------------------------------------------------
// out:	�ǂݍ��񂾃o�C�g��
//==============================================================
{
	size_t len;

	if(fp->on_image)
	{
		// �C���[�W�t�@�C������̓����I�[�v���ɑΉ�����ׁA���������V�[�N����
		fseek(fp->fd, (long)(fp->ofs + fp->acc_pos), SEEK_SET);
	}
	len = fread(ptr, 1, size, fp->fd);
	fp->acc_pos += len;

	return len;
}

//==============================================================
void FsInit(void)
//--------------------------------------------------------------
// ������
//--------------------------------------------------------------
// in:	�Ȃ�
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	imageExist = FALSE;

	SYSINFO(".... file initialize");
}

//==============================================================
void FsFin(void)
//--------------------------------------------------------------
// �I��
//--------------------------------------------------------------
// in:	�Ȃ�
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	FsUnMountImage();

	SYSINFO(".... file finish");
}

//==============================================================
void FsMountImage(const char *image)
//--------------------------------------------------------------
// �C���[�W�t�@�C���̃}�E���g
//--------------------------------------------------------------
// in:	image = �C���[�W�t�@�C���̖��O(NULL = �C���[�W�t�@�C������)
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	sFILE *fp;
	char *top, *p;
	u_int flag;
	int num;
	int ofs, size, size_o;
	char header_code[4 + 1];
	IHASH *hash;
	HASH *ptr;
	sFileInfo *info;

	if(imageExist)
	{
		// ���Ƀ}�E���g����Ă���t�@�C�����L��ꍇ�́A�A���}�E���g���Ă���
		//------------------------------------------------------------------
		FsUnMountImage();
	}

	if(image)
	{
		PATHCOPY(imageInfo.name, image);
		fp = FsOpen(imageInfo.name);
		if(fp)
		{
			u_int tmp_b;
			int h_size;

			// �w�b�_�T�C�Y����
			//------------------
			FsRead(fp, &tmp_b, sizeof(u_int));
			h_size = ntohl(tmp_b);

			// �w�b�_�͈��k����Ă���̂ŁA�ǂݍ���œW�J����
			//------------------------------------------------
			p = (char *)fioMalloc(h_size - 4, "fsTmp");
			ASSERT(p);
			FsRead(fp, p, h_size - 4);
			top = (char *)_ZlibDecode(p, ZlibEncodeSize(p), MEM_FS);
			ASSERT(top);
			Free(p);

			// �w�b�_����
			//------------
			p = top;
			StrCopyLength(header_code, 4 + 1, p);
			if(strcmp(header_code, HEADER_CODE) != 0)
			{
				ASSERT(0);
			}
			p += 4;

			flag = ntohl(*(u_int *)p);		// �p�b�N���
			p += sizeof(u_int);
			num = ntohl(*(u_int *)p);		// �t�@�C����
			p += sizeof(u_int);

			hash = OpenHash(num);				// �n�b�V�����J��
			ASSERT(hash);

			info = (sFileInfo *)fioMalloc(sizeof(sFileInfo) * num, "sFileInfo");
			ASSERT(info);
			imageInfo.info = info;

			while(num)
			{
				ptr = InstallString(hash, p);
				ASSERT(ptr);
				p += FNAME_MAXLEN;
				ofs = ntohl(*(u_int *)p);			// �t�@�C���擪����̃I�t�Z�b�g
				p += sizeof(u_int);
				size_o = ntohl(*(u_int *)p);		// ���t�@�C���T�C�Y
				p += sizeof(u_int);
				size = ntohl(*(u_int *)p);			// �t�@�C���T�C�Y(�Z�N�^�P�ʂŐ؂�グ)

				ptr->p = info;
				info->ofs = ofs + h_size;			// �f�[�^���i�[(�����Ńw�b�_�T�C�Y��������)
				info->size = size;
				info->size_orig = size_o;
				info += 1;

				p += (FILE_INFO_SIZE - (FNAME_MAXLEN + 4));
				num--;
			}
			Free(top);

			imageInfo.fp = fp;					// �Ȍ�t�@�C���͊J�����܂܂ɂȂ�܂��c
			imageInfo.hash = hash;

			imageExist = TRUE;

			PRINTF("mount '%s'\n", imageInfo.name);
		}
	}
}

//==============================================================
void FsUnMountImage(void)
//--------------------------------------------------------------
// �C���[�W�t�@�C���̃A���}�E���g
//--------------------------------------------------------------
// in:	�Ȃ�
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	if(imageExist)
	{
		FsClose(imageInfo.fp);
		CloseHash(imageInfo.hash);
		Free(imageInfo.info);

		PRINTF("unmount %s\n", imageInfo.name);

		imageExist = FALSE;
	}
}

//==============================================================
sFILE *FsOpen(const char *name)
//--------------------------------------------------------------
// �t�@�C���I�[�v��
//--------------------------------------------------------------
// in:	name = �t�@�C����
//--------------------------------------------------------------
// out:	�t�@�C���n���h��(NULL = �G���[)
//==============================================================
{
	sFILE *fp;
	FILE *fd;
	int len;
	HASH *hp;

	if(imageExist)
	{
		// �C���[�W�t�@�C����̃t�@�C�����J��
		//------------------------------------
		hp = LookupString(imageInfo.hash, name);
		if(hp)
		{
			sFileInfo *info;

			fp = l_createHandle();				// �n���h���擾
			fp->fd = imageInfo.fp->fd;
			fp->on_image = TRUE;

			info = (sFileInfo *)hp->p;
			fp->ofs = info->ofs;
			fp->size = info->size;
			fp->size_orig = info->size_orig;
			fp->acc_pos = 0;
			fseek(fp->fd, info->ofs, SEEK_SET);

			PRINTF("open from image '%s'\n", name);

			return fp;
		}
		else
		{
			// �C���[�W��Ɍ�����Ȃ������ꍇ
			//----------------------------------
			PRINTF("No file on image '%s'\n", name);
		}
	}

	//---------------------------------------------------------
	// HOST ��̃t�@�C�����J��(�C���[�W��ɂȂ������ꍇ�ɓK�p)
	//---------------------------------------------------------
	fd = fopen(l_makePath(PATH_HOST, name), "rb");
	if(fd)
	{
		fp = l_createHandle();					// �n���h���擾
		fp->fd = fd;
		fp->on_image = FALSE;

		fseek(fd, 0, SEEK_END);					// �����Ȃ�t�@�C���T�C�Y�����߂Ă���
		len = ftell(fd);						// �o�C�i�����[�h�̏ꍇ�Afseek() �ŃT�C�Y���Ԃ��Ă��Ȃ��c
		fp->size = ceilingvalue(len, SECTOR_SIZE);
		fp->size_orig = len;					// ���T�C�Y
		fp->acc_pos = 0;

		fseek(fd, 0, SEEK_SET);					// �t�@�C���|�C���^�͖߂��Ă���
	}
	else
	{
		fp = NULL;
	}

	return fp;
}

//==============================================================
void FsClose(sFILE *fp)
//--------------------------------------------------------------
// �t�@�C���N���[�Y
//--------------------------------------------------------------
// in:	fp = �t�@�C���n���h��
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	if(!fp->on_image)
	{
		// ���ڊJ�����t�@�C��
		//--------------------
		fclose(fp->fd);
	}

	l_destroyHandle(fp);
}

//==============================================================
size_t FsRead(sFILE *fp, void *ptr, size_t size)
//--------------------------------------------------------------
// �t�@�C���ǂݍ���
//--------------------------------------------------------------
// in:	fp   = �t�@�C���n���h��
//		ptr  = �Ǎ���|�C���^
//		size = �ǂݍ��݃T�C�Y
//--------------------------------------------------------------
// out:	�ǂݍ��񂾃T�C�Y
//==============================================================
{
	size_t len;

	len = l_readFile(fp, (u_char *)ptr, size);
//	ASSERT(len == size);

	return len;
}

//==============================================================
size_t FsSeek(sFILE *fp, size_t ofs, int whence)
//--------------------------------------------------------------
// �t�@�C���V�[�N
//--------------------------------------------------------------
// in:	fp     = �t�@�C���n���h��
//		ofs    = �I�t�Z�b�g
//		whence = ���[�h
//				 SEEK_SET: �t�@�C���̐擪����
//				 SEEK_CUR: ���݈ʒu����
//				 SEEK_END: �t�@�C���̏I�[����
//--------------------------------------------------------------
// out:	0 = ����
//==============================================================
{
	int ret = -1;

	if(fp->on_image)
	{
		// �C���[�W��̃t�@�C��
		// TODO: �G���[�`�F�b�N���������Ƃ�
		//----------------------------------
		switch(whence)
		{
			case SEEK_SET:
			{
				fp->acc_pos = ofs;
			}
			break;

			case SEEK_CUR:
			{
				fp->acc_pos += ofs;
			}
			break;

			case SEEK_END:
			{
				fp->acc_pos = fp->size_orig - ofs;
			}
			break;
		}
		ret = 0;
	}
	else
	{
		// ���ڊJ�����t�@�C��
		//--------------------
		ret = fseek(fp->fd, (long)ofs, whence);
	}

	return ret;
}

//==============================================================
size_t FsTell(sFILE *fp)
//--------------------------------------------------------------
// �t�@�C���̓ǂݍ��݈ʒu���擾
//--------------------------------------------------------------
// in:	fp = �t�@�C���n���h��
//--------------------------------------------------------------
// out:	�ǂݍ��݈ʒu
//==============================================================
{
	return fp->acc_pos;
}

//==============================================================
size_t FsGetSize(sFILE *fp)
//--------------------------------------------------------------
// �t�@�C���T�C�Y�̎擾
// ���Z�N�^�T�C�Y�ɐ؂�グ���l�ł�
// ��FsCreate() �ŊJ�����t�@�C���̏ꍇ�́A���T�C�Y��Ԃ��܂�
//--------------------------------------------------------------
// in:	fp = �t�@�C���n���h��
//--------------------------------------------------------------
// out:	�t�@�C���T�C�Y
//==============================================================
{
	return fp->size;
}

//==============================================================
size_t FsGetSizeOrig(sFILE *fp)
//--------------------------------------------------------------
// ���t�@�C���T�C�Y�̎擾
// ���Z�N�^�T�C�Y�ɐ؂�グ�Ă��Ȃ��l
//--------------------------------------------------------------
// in:	fp = �t�@�C���n���h��
//--------------------------------------------------------------
// out:	�t�@�C���T�C�Y
//==============================================================
{
	return fp->size_orig;
}

//==============================================================
size_t FsGetFileSize(const char *name)
//--------------------------------------------------------------
// �t�@�C���T�C�Y�̎擾
// ���t�@�C��������擾
//--------------------------------------------------------------
// in:	name = �t�@�C����
//--------------------------------------------------------------
// out:	�t�@�C���T�C�Y
//==============================================================
{
	sFILE *fp;
	size_t size = 0;

	fp = FsOpen(name);
	ASSERT(fp);
	if(fp)
	{
		size = FsGetSize(fp);
		FsClose(fp);
	}

	return size;
}

//==============================================================
size_t FsGetFileSizeOrig(const char *name)
//--------------------------------------------------------------
// ���t�@�C���T�C�Y�̎擾
// ���t�@�C��������擾
//--------------------------------------------------------------
// in:	name = �t�@�C����
//--------------------------------------------------------------
// out:	�t�@�C���T�C�Y
//==============================================================
{
	sFILE *fp;
	size_t size = 0;

	fp = FsOpen(name);
	ASSERT(fp);
	if(fp)
	{
		size = FsGetSizeOrig(fp);
		FsClose(fp);
	}

	return size;
}

//==============================================================
sFILE *FsCreate(const char *name)
//--------------------------------------------------------------
// �t�@�C����V�K�쐬����
//--------------------------------------------------------------
// in:	name = �t�@�C����
//--------------------------------------------------------------
// out:	�t�@�C���n���h��(NULL = �G���[)
//==============================================================
{
	sFILE *fp;
	FILE *fd;

	if(imageExist)
	{
		if(LookupString(imageInfo.hash, name))
		{
			// �C���[�W��ɑ��݂���ꍇ�́A�G���[�Ƃ��Ĉ���
			PRINTF("File '%s' already exists.\n", name);
			return NULL;
		}
	}

	fd = fopen(l_makePath(PATH_HOST, name), "wb");
//	ASSERT(fd);

	if(fd)
	{
		fp = l_createHandle();
		fp->fd = fd;
		fp->on_image = FALSE;
		fp->size = fp->size_orig = 0;
	}
	else
	{
		// �G���[
		//--------
		fp = NULL;
	}

	return fp;
}

//==============================================================
void FsWrite(sFILE *fp, void *ptr, size_t size)
//--------------------------------------------------------------
// �f�[�^���t�@�C���ɏ����o��
//--------------------------------------------------------------
// in:	fp   = �t�@�C���n���h��
//		ptr  = �f�[�^�|�C���^
//		size = �f�[�^�T�C�Y
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	size_t len;

	if(fp->on_image)
	{
		PRINTF("File on image.\n");
	}
	else
	{
		len = fwrite(ptr, size, 1, fp->fd);
		if(len >= 0)
		{
			fp->size += len;
			fp->size_orig = fp->size;
		}
		ASSERT(len >= 0);
	}
}

