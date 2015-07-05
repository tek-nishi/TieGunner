//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
//  < fpack >
//   2004 ASTROLL Inc. All Rights Reserved.
//--------------------------------------------------------------
//
//	ファイルパック
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  $Id: main.c,v 1.3 2004/02/09 12:57:13 nishi Exp $
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/********************************************/
/*           インクルードファイル           */
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
/*             定数・マクロ宣言             */
/********************************************/
#define FILE_HEADER_SIZE  (4 + sizeof(int) * 2)		// ヘッダサイズ
#define FILE_INFO_SIZE  (FNAME_MAXLEN + (sizeof(int) * 3))

#define SECTOR_SIZE  2048						// セクタサイズ

#define PACK_BUFFER  0x100000					// ファイル書き出しバッファ(1MB)


/********************************************/
/*                構造体宣言                */
/********************************************/
typedef struct tagList sList;
struct tagList {
	char file[FNAME_MAXLEN];					// ファイル名
	int len;									// ファイルサイズ
	int pack;									// TRUE = ファイルをパックする
	sList *next;								// 次の構造体へのポインタ
};

typedef struct {
	int output_file;							// TRUE = 出力ファイルを取得した
	int file_num;								// 入力ファイル数
	int pack_num;								// 最終的にパックするファイル数
	sList *top, *last;							// ファイルリスト(リンクリストにして、数に依存しないようにしています)

	unsigned int flag;							// パック時のフラグ

	unsigned char *header;						// ヘッダ情報
	int header_size;							// ヘッダサイズ
} sPack;


/********************************************/
/*                 変数宣言                 */
/********************************************/
static sPack pack;
static char currentDir[FNAME_MAXLEN];


/********************************************/
/*                プログラム                */
/********************************************/

//==============================================================
static void l_title(void)
//--------------------------------------------------------------
// タイトル表示
//--------------------------------------------------------------
// in:	なし
//--------------------------------------------------------------
// out:	なし
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
// 使い方の表示
//--------------------------------------------------------------
// in:	なし
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	printf("fpack [options] arcive-file files...\n");
	printf("       -c [path]    カレントパスの指定\n");
}

//==============================================================
static sList *l_listCreate(void)
//--------------------------------------------------------------
// リスト構造体を１つ作成
//--------------------------------------------------------------
// in:	なし
//--------------------------------------------------------------
// out:	構造体
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
// ワーク初期化
//--------------------------------------------------------------
// in:	なし
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	pack.output_file = FALSE;
	pack.file_num = 0;
	pack.top = pack.last = NULL;
	pack.flag = 0;

	pack.header = NULL;

	currentDir[0] = '\0';						// カレントディレクトリの初期値
}

//==============================================================
static void l_workFin(void)
//--------------------------------------------------------------
// ワーク後始末
//--------------------------------------------------------------
// in:	なし
//--------------------------------------------------------------
// out:	なし
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
// オプション解析
//--------------------------------------------------------------
// in:	argc = 引数の数
//		argv = 引数のポインタ列
//--------------------------------------------------------------
// out:	なし
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
				case 'c':						// カレントディレクトリ指定
				if(argc > 2)
				{
					// 次の文字列をディレクトリ名として読み込む
					//------------------------------------------
					argv++;
					argc--;

					PATHCOPY(currentDir, *argv);

					// パスの最後が '/' にならないようにしておく
					//-------------------------------------------
					p = &currentDir[strlen(currentDir) - 1];
					if(*p == '/')
						*p = '\0';
					else
					if(strcmp(p, ".") == 0)
					{
						// '-c . ' とオプションで与えられた場合は currentDir をクリアする
						//----------------------------------------------------------------
						currentDir[0] = '\0';
					}
				}
				break;
			}
		}
		else
		{
			ptr = l_listCreate();				// 登録ワークの取得
			if(pack.output_file)
			{
				// カレントパスの設定
				//--------------------
				if(*p != '/')
				{
					if((strlen(currentDir) + strlen(p)) >= (FNAME_MAXLEN - 2))
					{
						// ファイル名が長すぎる
						//----------------------
						printf("Filename is too long!\n");
						printf("dir:  %s\n", currentDir);
						printf("file: %s\n", p);
					}
					else
					{
						// 若干手抜きな名前生成
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
					// ルート始まりのファイル名はそのまま使う
					//----------------------------------------
					PATHCOPY(ptr->file, p);
				}
				pack.file_num++;
			}
			else
			{
				// 入力ファイル
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
// ファイル入力からコマンド解析
// ※stdin からの解析もこれで行える
//--------------------------------------------------------------
// in:	fp  = アクセスハンドル
//		cnt = 引数の数を格納するポインタ
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	int i, j, k;
	char *line, **pp, *p, *tmp;
	int max;
	IHASH *hash;
	HASH *h, **hh;

	hash = OpenHash(16384);						// ハッシュを利用して引数リストを作成する

	max = 1024;									// １行の最大長
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
			goto SkipLine;						// 行頭がコメントの場合はスキップ

		pp = separateString(line, " \t", NULL, NULL);
		k = 0;
		while((p = pp[k])!=NULL)
		{
			if(p[0])
			{
				sprintf(tmp, "%08X", j);		// 先頭に数字をつけて、入力順にソートされるようにする
				strcpy(tmp+8, p);
				InstallString(hash, tmp);
				j++;
			}
			k++;
		}
		freeStringList(pp);
SkipLine:
		;										// VC 対策
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
// 同じ名前のファイルを除外する
//--------------------------------------------------------------
// in:	なし
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	sList *ptr;
	IHASH *hash;

	// ハッシュテーブルを作成(若干大きめなサイズで作成して衝突を避ける)
	//------------------------------------------------------------------
	hash = OpenHash((pack.file_num + 255) & ~255);
	ptr = pack.top;
	ptr = ptr->next;							// 最初は出力先なのでスキップ
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
// ファイルが存在するかチェック
//--------------------------------------------------------------
// in:	なし
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	sList *ptr;
	ptr = pack.top;
	ptr = ptr->next;							// 最初は出力先なのでスキップ
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
// ヘッダ作成
//--------------------------------------------------------------
// in:	なし
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	unsigned char *p, *p_chg;
	int flag;
	int ofs, len;
	sList *ptr;
	FILE *fp;

	// ヘッダーサイズを求め、作業領域を取得
	//--------------------------------------
	ofs = FILE_HEADER_SIZE + FILE_INFO_SIZE * pack.pack_num;
//	ofs = CEILING(ofs, SECTOR_SIZE);			// セクタサイズでパディング
	pack.header = malloc(ofs);
	pack.header_size = ofs;
	memset(pack.header, 0, ofs);				// ワークをゼロクリア

	p = pack.header;
	*(p + 0) = 'F';								// とりあえず識別コード
	*(p + 1) = 'P';
	*(p + 2) = 'A';
	*(p + 3) = 'K';
	p += 4;

	putvalue(p, pack.flag);						// パックした時の情報
	p += 4;

	putvalue(p, pack.pack_num);					// パックした数
	p += 4;

	ptr = pack.top;
	ptr = ptr->next;							// 最初は出力先なのでスキップ
	ofs = 0;
	while(ptr)
	{
		if(ptr->pack)
		{
			PATHCOPY(p, ptr->file);				// ファイル名をコピー
			p_chg = p;
			while(*p_chg != '\0')
			{
				if(*p_chg == '\\')
					*p_chg = '/';
				p_chg++;
			}
			p += FNAME_MAXLEN;

			putvalue(p, ofs);					// オフセット
			p += 4;

			len = getFileSize(ptr->file);
			if(!len)
			{
				// ファイルサイズがゼロのファイル(エラーの可能性もあり)
				//----------------------------------------------------
				printf("File size is Zero. '%s'\n", ptr->file);
			}

			putvalue(p, len);					// 実ファイルサイズを格納
			p += 4;

			// ファイルサイズはセクタサイズで切り上げて格納
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
// ファイルをパック
//--------------------------------------------------------------
// in:	なし
//--------------------------------------------------------------
// out:	なし
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
	// ヘッダーの書き出し
	//--------------------
#if 1
	{
		void *ptr;
		int size, s_size;
		unsigned char h_size[4];

		ptr = ZlibEncode(pack.header, pack.header_size);
		size = ZlibEncodeSize(ptr);

		// セクタサイズでパディング
		s_size = CEILING(size + 8 + 4, SECTOR_SIZE);

		putvalue(h_size, s_size);

		fwrite(h_size, 4, 1, dst);
		fwrite(ptr, size + 8, 1, dst);
		free(ptr);


		if(s_size > (size + 8 + 4))
		{
			//余白部分を書き出す
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
	// 対象ファイルを連結
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
				// 正しくエラー処理をする事
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

				// セクタサイズでパディング
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
// オプション内容を表示
//--------------------------------------------------------------
// in:	なし
//--------------------------------------------------------------
// out:	なし
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
// メインプログラム
//--------------------------------------------------------------
// in:	argc = 引数の数
//		argv = 引数のポインタ列
//--------------------------------------------------------------
// out:	0 = 処理完了
//==============================================================
{
	int count;
	char **pp;

	// タイトル表示
	//--------------
	l_title();

	// ワークを初期化
	//----------------
	l_workInit();

	// コマンドラインからのオプション解析
	//------------------------------------
	l_getOption(argc - 1, argv + 1);

	// stdin からのオプション解析
	//----------------------------
	pp = l_getStdin(stdin, &count);
	l_getOption(count, pp);
	freeStringList(pp);

	if(!pack.pack_num)
	{
		l_usage();
		return 1;
	}

	// 同じ名前のファイルをチェックする
	//----------------------------------
	l_checkEvenFile();

	// ファイルの存在チェック
	//------------------------
	l_checkFileExist();

	// ヘッダ作成
	//------------
	l_createHeadder();

	// 解析結果(TEST)
	//----------------
	l_printOption();

	// データ出力
	//------------
	l_packFiles();

	// 後始末
	//--------
	l_workFin();

	return 0;
}

