//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
//  < fpack >
//   2004 ASTROLL Inc. All Rights Reserved.
//--------------------------------------------------------------
//
//	�t�@�C���p�b�N
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  $Id: main.c,v 1.3 2004/02/09 12:57:13 nishi Exp $
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
#include "misc.h"
#include "co_zlib.h"
#include "main.h"


/********************************************/
/*             �萔�E�}�N���錾             */
/********************************************/
#define FILE_HEADER_SIZE  (4 + sizeof(int) * 2)		// �w�b�_�T�C�Y
#define FILE_INFO_SIZE  (FNAME_MAXLEN + (sizeof(int) * 3))

#define SECTOR_SIZE  2048						// �Z�N�^�T�C�Y

#define PACK_BUFFER  0x100000					// �t�@�C�������o���o�b�t�@(1MB)


/********************************************/
/*                �\���̐錾                */
/********************************************/
typedef struct tagList sList;
struct tagList {
	char file[FNAME_MAXLEN];					// �t�@�C����
	int len;									// �t�@�C���T�C�Y
	int pack;									// TRUE = �t�@�C�����p�b�N����
	sList *next;								// ���̍\���̂ւ̃|�C���^
};

typedef struct {
	int output_file;							// TRUE = �o�̓t�@�C�����擾����
	int file_num;								// ���̓t�@�C����
	int pack_num;								// �ŏI�I�Ƀp�b�N����t�@�C����
	sList *top, *last;							// �t�@�C�����X�g(�����N���X�g�ɂ��āA���Ɉˑ����Ȃ��悤�ɂ��Ă��܂�)

	unsigned int flag;							// �p�b�N���̃t���O

	unsigned char *header;						// �w�b�_���
	int header_size;							// �w�b�_�T�C�Y
} sPack;


/********************************************/
/*                 �ϐ��錾                 */
/********************************************/
static sPack pack;
static char currentDir[FNAME_MAXLEN];


/********************************************/
/*                �v���O����                */
/********************************************/

//==============================================================
static void l_title(void)
//--------------------------------------------------------------
// �^�C�g���\��
//--------------------------------------------------------------
// in:	�Ȃ�
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	printf("fpack Version 0.9\n");
	printf("Copyright (C) 2004 BitStep All Rights Reserved.\n");
	printf("by Nishiyama Nobuyuki (nishi@bitstep.com)\n");
	printf("\n");
}

//==============================================================
static void l_usage(void)
//--------------------------------------------------------------
// �g�����̕\��
//--------------------------------------------------------------
// in:	�Ȃ�
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	printf("fpack [options] arcive-file files...\n");
	printf("       -c [path]    �J�����g�p�X�̎w��\n");
}

//==============================================================
static sList *l_listCreate(void)
//--------------------------------------------------------------
// ���X�g�\���̂��P�쐬
//--------------------------------------------------------------
// in:	�Ȃ�
//--------------------------------------------------------------
// out:	�\����
//==============================================================
{
	sList *ptr;

	ptr = malloc(sizeof(sList));
	ptr->next = NULL;
	ptr->pack = TRUE;

	if(!pack.top)
		pack.top = ptr;

	if(pack.last)
		pack.last->next = ptr;
	pack.last = ptr;

	return ptr;
}

//==============================================================
static void l_workInit(void)
//--------------------------------------------------------------
// ���[�N������
//--------------------------------------------------------------
// in:	�Ȃ�
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	pack.output_file = FALSE;
	pack.file_num = 0;
	pack.top = pack.last = NULL;
	pack.flag = 0;

	pack.header = NULL;

	currentDir[0] = '\0';						// �J�����g�f�B���N�g���̏����l
}

//==============================================================
static void l_workFin(void)
//--------------------------------------------------------------
// ���[�N��n��
//--------------------------------------------------------------
// in:	�Ȃ�
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	sList *cur, *next;

	cur = pack.top;
	while(cur)
	{
		next = cur->next;
		free(cur);

		cur = next;
	}

	if(pack.header)
		free(pack.header);
}

//==============================================================
static void l_getOption(int argc, char **argv)
//--------------------------------------------------------------
// �I�v�V�������
//--------------------------------------------------------------
// in:	argc = �����̐�
//		argv = �����̃|�C���^��
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	char *p;
	sList *ptr;

	while(argc > 0)
	{
		p = *argv;
		if(*p == '-')
		{
			switch(*(p + 1))
			{
				case 'c':						// �J�����g�f�B���N�g���w��
				if(argc > 2)
				{
					// ���̕�������f�B���N�g�����Ƃ��ēǂݍ���
					//------------------------------------------
					argv++;
					argc--;

					PATHCOPY(currentDir, *argv);

					// �p�X�̍Ōオ '/' �ɂȂ�Ȃ��悤�ɂ��Ă���
					//-------------------------------------------
					p = &currentDir[strlen(currentDir) - 1];
					if(*p == '/')
						*p = '\0';
					else
					if(strcmp(p, ".") == 0)
					{
						// '-c . ' �ƃI�v�V�����ŗ^����ꂽ�ꍇ�� currentDir ���N���A����
						//----------------------------------------------------------------
						currentDir[0] = '\0';
					}
				}
				break;
			}
		}
		else
		{
			ptr = l_listCreate();				// �o�^���[�N�̎擾
			if(pack.output_file)
			{
				// �J�����g�p�X�̐ݒ�
				//--------------------
				if(*p != '/')
				{
					if((strlen(currentDir) + strlen(p)) >= (FNAME_MAXLEN - 2))
					{
						// �t�@�C��������������
						//----------------------
						printf("Filename is too long!\n");
						printf("dir:  %s\n", currentDir);
						printf("file: %s\n", p);
					}
					else
					{
						// �኱�蔲���Ȗ��O����
						//----------------------
						if(currentDir[0] != '\0')
						{
							strcpy(ptr->file, currentDir);
							strcat(ptr->file, "/");
							strcat(ptr->file, p);
						}
						else
						{
							strcpy(ptr->file, p);
						}
					}
				}
				else
				{
					// ���[�g�n�܂�̃t�@�C�����͂��̂܂܎g��
					//----------------------------------------
					PATHCOPY(ptr->file, p);
				}
				pack.file_num++;
			}
			else
			{
				// ���̓t�@�C��
				//--------------
				PATHCOPY(ptr->file, p);
				pack.output_file = TRUE;
			}
		}

		argv++;
		argc--;
	}

	pack.pack_num = pack.file_num;
}

//==============================================================
static char **l_getStdin(FILE *fp, int *cnt)
//--------------------------------------------------------------
// �t�@�C�����͂���R�}���h���
// ��stdin ����̉�͂�����ōs����
//--------------------------------------------------------------
// in:	fp  = �A�N�Z�X�n���h��
//		cnt = �����̐����i�[����|�C���^
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	int i, j, k;
	char *line, **pp, *p, *tmp;
	int max;
	IHASH *hash;
	HASH *h, **hh;

	hash = OpenHash(16384);						// �n�b�V���𗘗p���Ĉ������X�g���쐬����

	max = 1024;									// �P�s�̍ő咷
	line = (char *)malloc(max);
	tmp = (char *)malloc(max);

	max = 1024;
	j = 0;
	while(1)
	{
		i = getLine(fp, line, max);
		if(i == 0)
			break;

		if(line[0]=='#')
			goto SkipLine;						// �s�����R�����g�̏ꍇ�̓X�L�b�v

		pp = separateString(line, " \t", NULL, NULL);
		k = 0;
		while((p = pp[k])!=NULL)
		{
			if(p[0])
			{
				sprintf(tmp, "%08X", j);		// �擪�ɐ��������āA���͏��Ƀ\�[�g�����悤�ɂ���
				strcpy(tmp+8, p);
				InstallString(hash, tmp);
				j++;
			}
			k++;
		}
		freeStringList(pp);
SkipLine:
		;										// VC �΍�
	}

	free(tmp);
	free(line);

	hh = SortHashValueA(hash);

	pp = (char **)malloc(sizeof(char *) * (hash->words + 1));
	if(cnt)
		*cnt = hash->words;

	for(i=0; i<hash->words; ++i)
	{
		h = hh[i];
		pp[i] = copyText(h->s + 8);
	}
	pp[i] = NULL;

	free(hh);

	CloseHash(hash);

	return pp;
}

//==============================================================
static void l_checkEvenFile(void)
//--------------------------------------------------------------
// �������O�̃t�@�C�������O����
//--------------------------------------------------------------
// in:	�Ȃ�
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	sList *ptr;
	IHASH *hash;

	// �n�b�V���e�[�u�����쐬(�኱�傫�߂ȃT�C�Y�ō쐬���ďՓ˂������)
	//------------------------------------------------------------------
	hash = OpenHash((pack.file_num + 255) & ~255);
	ptr = pack.top;
	ptr = ptr->next;							// �ŏ��͏o�͐�Ȃ̂ŃX�L�b�v
	while(ptr)
	{
		if(!InstallString(hash, ptr->file))
		{
			ptr->pack = FALSE;
			pack.pack_num--;
		}
		ptr = ptr->next;
	}
	CloseHash(hash);
}

//==============================================================
static void l_checkFileExist(void)
//--------------------------------------------------------------
// �t�@�C�������݂��邩�`�F�b�N
//--------------------------------------------------------------
// in:	�Ȃ�
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	sList *ptr;
	ptr = pack.top;
	ptr = ptr->next;							// �ŏ��͏o�͐�Ȃ̂ŃX�L�b�v
	while(ptr)
	{
		if(ptr->pack)
		{
			if(!fileExist(ptr->file))
			{
				printf("No File. '%s'\n", ptr->file);

				ptr->pack = FALSE;
				pack.pack_num--;
			}
		}
		ptr = ptr->next;
	}
}

//==============================================================
static void l_createHeadder(void)
//--------------------------------------------------------------
// �w�b�_�쐬
//--------------------------------------------------------------
// in:	�Ȃ�
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	unsigned char *p, *p_chg;
	int flag;
	int ofs, len;
	sList *ptr;
	FILE *fp;

	// �w�b�_�[�T�C�Y�����߁A��Ɨ̈���擾
	//--------------------------------------
	ofs = FILE_HEADER_SIZE + FILE_INFO_SIZE * pack.pack_num;
//	ofs = CEILING(ofs, SECTOR_SIZE);			// �Z�N�^�T�C�Y�Ńp�f�B���O
	pack.header = malloc(ofs);
	pack.header_size = ofs;
	memset(pack.header, 0, ofs);				// ���[�N���[���N���A

	p = pack.header;
	*(p + 0) = 'F';								// �Ƃ肠�������ʃR�[�h
	*(p + 1) = 'P';
	*(p + 2) = 'A';
	*(p + 3) = 'K';
	p += 4;

	putvalue(p, pack.flag);						// �p�b�N�������̏��
	p += 4;

	putvalue(p, pack.pack_num);					// �p�b�N������
	p += 4;

	ptr = pack.top;
	ptr = ptr->next;							// �ŏ��͏o�͐�Ȃ̂ŃX�L�b�v
	ofs = 0;
	while(ptr)
	{
		if(ptr->pack)
		{
			PATHCOPY(p, ptr->file);				// �t�@�C�������R�s�[
			p_chg = p;
			while(*p_chg != '\0')
			{
				if(*p_chg == '\\')
					*p_chg = '/';
				p_chg++;
			}
			p += FNAME_MAXLEN;

			putvalue(p, ofs);					// �I�t�Z�b�g
			p += 4;

			len = getFileSize(ptr->file);
			if(!len)
			{
				// �t�@�C���T�C�Y���[���̃t�@�C��(�G���[�̉\��������)
				//----------------------------------------------------
				printf("File size is Zero. '%s'\n", ptr->file);
			}

			putvalue(p, len);					// ���t�@�C���T�C�Y���i�[
			p += 4;

			// �t�@�C���T�C�Y�̓Z�N�^�T�C�Y�Ő؂�グ�Ċi�[
			//----------------------------------------------
			len = CEILING(len, SECTOR_SIZE);
			ptr->len = len;
			putvalue(p, len);
			p += 4;

			ofs += len;
		}
		ptr = ptr->next;
	}

//	printf("%d  %d\n", pack.header_size, p - pack.header);
}

//==============================================================
static void l_packFiles(void)
//--------------------------------------------------------------
// �t�@�C�����p�b�N
//--------------------------------------------------------------
// in:	�Ȃ�
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	sList *ptr;
	char *file;
	FILE *dst, *src;
	char *buffer;
	int size, len;

	ptr = pack.top;
	if(!ptr)
		return;

	file = ptr->file;
	dst = fopen(file, "wb");
	if(!dst)
	{
		printf("Can't create file '%s'\n", file);
		return;
	}

	//--------------------
	// �w�b�_�[�̏����o��
	//--------------------
#if 1
	{
		void *ptr;
		int size, s_size;
		unsigned char h_size[4];

		ptr = ZlibEncode(pack.header, pack.header_size);
		size = ZlibEncodeSize(ptr);

		// �Z�N�^�T�C�Y�Ńp�f�B���O
		s_size = CEILING(size + 8 + 4, SECTOR_SIZE);

		putvalue(h_size, s_size);

		fwrite(h_size, 4, 1, dst);
		fwrite(ptr, size + 8, 1, dst);
		free(ptr);


		if(s_size > (size + 8 + 4))
		{
			//�]�������������o��
			//-------------------
			s_size = s_size - (size + 8 + 4);
			buffer = malloc(s_size);
			memset(buffer, 0xff, s_size);
			fwrite(buffer, s_size, 1, dst);
			free(buffer);

		}
	}
#else
	fwrite(pack.header, pack.header_size, 1, dst);
#endif

	//--------------------
	// �Ώۃt�@�C����A��
	//--------------------
	buffer = malloc(PACK_BUFFER);

	ptr = ptr->next;
	while(ptr)
	{
		if(ptr->pack)
		{
			src = fopen(ptr->file, "rb");
			if(!src)
			{
				printf("Can't open file '%s'\n", ptr->file);
				//--------------------------
				// �������G���[���������鎖
				//--------------------------
			}
			else
			{
				len = 0;
				do
				{
					size = fread(buffer, 1, PACK_BUFFER, src);
					len += size;
					if(size)
						fwrite(buffer, 1, size, dst);
				}
				while(size == PACK_BUFFER);
				fclose(src);

				// �Z�N�^�T�C�Y�Ńp�f�B���O
				//--------------------------
				if(len != ptr->len)
				{
					len = ptr->len - len;
					memset(buffer, 0, len);
					fwrite(buffer, 1, len, dst);
				}
			}
		}
		ptr = ptr->next;
	}
	free(buffer);

	fclose(dst);
}

//==============================================================
static void l_printOption(void)
//--------------------------------------------------------------
// �I�v�V�������e��\��
//--------------------------------------------------------------
// in:	�Ȃ�
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	sList *ptr;

	if(!pack.file_num)
		return;

	ptr = pack.top;
	printf("out: %s\n", ptr->file);

	ptr = ptr->next;
	while(ptr)
	{
		if(ptr->pack)
			printf("in:  %s\n", ptr->file);
		ptr = ptr->next;
	}
}

//==============================================================
int main(int argc, char **argv)
//--------------------------------------------------------------
// ���C���v���O����
//--------------------------------------------------------------
// in:	argc = �����̐�
//		argv = �����̃|�C���^��
//--------------------------------------------------------------
// out:	0 = ��������
//==============================================================
{
	int count;
	char **pp;

	// �^�C�g���\��
	//--------------
	l_title();

	// ���[�N��������
	//----------------
	l_workInit();

	// �R�}���h���C������̃I�v�V�������
	//------------------------------------
	l_getOption(argc - 1, argv + 1);

	// stdin ����̃I�v�V�������
	//----------------------------
	pp = l_getStdin(stdin, &count);
	l_getOption(count, pp);
	freeStringList(pp);

	if(!pack.pack_num)
	{
		l_usage();
		return 1;
	}

	// �������O�̃t�@�C�����`�F�b�N����
	//----------------------------------
	l_checkEvenFile();

	// �t�@�C���̑��݃`�F�b�N
	//------------------------
	l_checkFileExist();

	// �w�b�_�쐬
	//------------
	l_createHeadder();

	// ��͌���(TEST)
	//----------------
	l_printOption();

	// �f�[�^�o��
	//------------
	l_packFiles();

	// ��n��
	//--------
	l_workFin();

	return 0;
}

