
#include "co_font.h"
#include "co_texture.h"
#include "co_debug.h"
#include "co_misc.h"
#include "co_strings.h"
#include "co_graph.h"


#define FONT_DEFAULT_FILE    PATH_DEVELOP"/font3.png"
#define FONT_DEFAULT_WIDTH   6
#define FONT_DEFAULT_HEIGHT  12

#define FONT_TILE_NUM  16						// タイリング数
#define TAB_NUM  4								// タブ数

enum enmFONTPRINT {
	FONT_PRINT,									// 通常表示
	FONT_GET_WIDTH,								// 最大表示幅を取得
	FONT_GET_STR,								// 修飾文字を省いた文字列を取得
};


typedef struct {
	sTexture *texture;							// テクスチャ
	int width;									// １文字あたりのデータサイズ
	int height;

	int next_x;									// 文字送り X
	int next_y;									// 文字送り Y
} sFont;


static sRGBA font_col[] = {
	{ 0, 0, 0, 1 },
	{ 0, 0, 1, 1 },
	{ 1, 0, 0, 1 },
	{ 1, 0, 1, 1 },
	{ 0, 1, 0, 1 },
	{ 0, 1, 1, 1 },
	{ 1, 1, 0, 1 },
	{ 1, 1, 1, 1 },

	{ 1, 0.85, 0, 1 },
	{ 0, 0, 0, 1 },
};

static sFont font_list[FONT_NUM];



static void fontMakeCol(sRGBA *res, int col, int hilight, float alpha)
{
	*res = font_col[col];
	if(hilight == 1)
	{
		res->red *= 0.5f;
		res->green *= 0.5f;
		res->blue *= 0.5f;
	}
	else
	if(hilight == 2)
	{
		res->alpha *= 0.5f;
	}
	res->alpha *= alpha;
}

//==============================================================
static int fontPrint(int mode, int x, int y, int prio, char *str)
//--------------------------------------------------------------
// 表示
//--------------------------------------------------------------
// in:	mode = enmFONTPRINT
//		x, y = 表示開始位置
//		prio = プライオリティ
//		str  = 表示文字列
//--------------------------------------------------------------
// out:	実行結果
//==============================================================
{
	int put_x;
	int put_y;
	int chara_num;
	int col = 7;
	int blink = 0;
	int hilight = 0;
	int font_index = 0;
	int print_width = 0;
	int width = 0;
	float alpha = 1.0f;
	sRGBA color = { 1.0f, 1.0f, 1.0f, 1.0f };
	sFont *font;

	put_x = x;
	put_y = y;
	chara_num = 0;
	fontMakeCol(&color, col, hilight, alpha);
	font = &font_list[font_index];
	while(*str != '\0')
	{
		int a;
		sGRPOBJ *obj;

		a = *str;
		if(a == '\n')
		{
			// 改行
			put_x = x;
			put_y += font->next_y;
			chara_num = 0;
			if(print_width < width) print_width = width;
			width = 0;
		}
		else
		if(a == '\t')
		{
			// タブ
			chara_num = ((chara_num + TAB_NUM) / TAB_NUM) * TAB_NUM;
			put_x = x + chara_num * font->next_x;
		}
		else
		if(a == '$')
		{
			// 拡張コード
			a = *(str + 1);
			switch(a)
			{
				case 'C':
				{
					col = *(str + 2) - '0';
					ASSERT((col >= 0) && (col <= 9));
					fontMakeCol(&color, col, hilight, alpha);
					str += 2;
				}
				break;

				case 'H':
				{
					hilight = *(str + 2) - '0';
					ASSERT((hilight >= 0) && (hilight <= 2));
					fontMakeCol(&color, col, hilight, alpha);
					str += 2;
				}
				break;

				case 'B':
				{
					blink = *(str + 2) - '0';
					ASSERT((blink >= 0) && (blink <= 2));
					str += 2;
				}
				break;

				case 'F':
				{
					font_index = *(str + 2) - '0';
					ASSERT((font_index >= FONT_SYS) && (font_index < FONT_NUM));
					font = &font_list[font_index];
					str += 2;
				}
				break;

			case 'A':
				{
					str += 2;

					char value[FNAME_MAXLEN];
					char *dst = value;
					while((*str != '$') && (*str != '\0'))
					{
						*dst  = *str;
						dst += 1;
						str += 1;
					}
					*dst = '\0';
					alpha = atof(value);
					fontMakeCol(&color, col, hilight, alpha);

					if(*str == '\0') str -= 1;
				}
				break;

				case 'E':
				{
					//-------------------
					// TODO:しばらく無視
					//-------------------
					str += 1;
				}
				break;
			}

			//-----------------------------------------------------
			// データエラーの場合、'$'を飛ばして文字列表示を続ける
			//-----------------------------------------------------
		}
		else
		{
			switch(mode)
			{
				case FONT_PRINT:
				{
					int u;
					int v;
					BOOL disp = TRUE;

					if(blink == 1)
					{
						disp = (g.time & 8) ? TRUE : FALSE;
					}
					else
					if(blink == 2)
					{
						disp = (g.time & 32) ? TRUE : FALSE;
					}

					if(disp)
					{
						u = (a & 0xf) * font->width;
						v = ((a & 0xf0) >> 4) * font->height;

						obj = GRPOBJ_QUAD(prio);
						GrpSetRGBA(obj, color.red, color.green, color.blue, color.alpha);
						GrpSetTexture(obj, font->texture);
						GrpSetPos(obj, put_x, put_y);
						GrpSetSize(obj, font->width - 1, font->height - 1);
						GrpSetUV(obj, u, v);
						GrpSetFilter(obj, TRUE);
					}
				}
				break;
			}
			put_x += font->next_x;
			chara_num += 1;
		}

		str += 1;
	}

	width = put_x - x;
	if(print_width < width) print_width = width;

	return print_width;
}

//==============================================================
void FontInit(void)
//--------------------------------------------------------------
// 初期化
//--------------------------------------------------------------
// in:	なし
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	ZEROMEMORY(font_list, sizeof(sFont) * FONT_NUM);
	FontSetup(FONT_SYS, FONT_DEFAULT_FILE, FONT_DEFAULT_WIDTH, FONT_DEFAULT_HEIGHT);

	SYSINFO(".... font initialize");
}

//==============================================================
void FontFin(void)
//--------------------------------------------------------------
// 終了
//--------------------------------------------------------------
// in:	なし
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	int i;

	for(i = FONT_SYS; i < FONT_NUM; ++i)
	{
		if(font_list[i].texture)
		{
			TexDestroy(font_list[i].texture);
		}
	}

	SYSINFO(".... font finish");
}

//==============================================================
void FontSetup(int index, char *file, int next_x, int next_y)
//--------------------------------------------------------------
// フォントのセットアップ
//--------------------------------------------------------------
// in:	index  = enmFONT_IDX
//		file   = テクスチャファイル
//		next_x = 文字送り
//		next_y = 改行幅
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	sFont *font;
	FVector2 size;

	font = &font_list[index];
	if(font->texture)
	{
		TexDestroy(font->texture);
	}

	font->texture = TexRead(file);
	TexGetSize(&size, font->texture);
	font->width = (int)size.x / FONT_TILE_NUM;
	font->height = (int)size.y / FONT_TILE_NUM;

	font->next_x = next_x;
	font->next_y = next_y;
}

//==============================================================
void FontGetInfo(sFontInfo *res, int index)
//--------------------------------------------------------------
// フォント情報の取得
//--------------------------------------------------------------
// in:	res   = 結果を格納するポインタ
//		index = enmFONT_IDX
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	char *id_str;

	id_str = TexGetName(font_list[index].texture);
	STRCPY16(res->tex_id, id_str);
	res->width = font_list[index].next_x;
	res->height = font_list[index].next_y;
}

//==============================================================
void FontPrint(int x, int y, int prio, char *str)
//--------------------------------------------------------------
// 表示
//--------------------------------------------------------------
// in:	x, y = 表示開始位置
//		prio = プライオリティ
//		str  = 表示文字列
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
#if 1
	fontPrint(FONT_PRINT, x, y, prio, str);
#else
	int put_x;
	int put_y;
	int chara_num;
	int col = 7;
	int blink = 0;
	int hilight = 0;
	int font_index = 0;
	sRGBA color;
	sFont *font;

	put_x = x;
	put_y = y;
	chara_num = 0;
	fontMakeCol(&color, col, hilight);
	font = &font_list[font_index];
	while(*str != '\0')
	{
		int a;
		sGRPOBJ *obj;

		a = *str;
		if(a == '\n')
		{
			// 改行
			put_x = x;
			put_y += font->next_y;
			chara_num = 0;
		}
		else
		if(a == '\t')
		{
			// タブ
			chara_num = ((chara_num + TAB_NUM) / TAB_NUM) * TAB_NUM;
			put_x = x + chara_num * font->next_x;
		}
		else
		if(a == '$')
		{
			// 拡張コード
			a = *(str + 1);
			switch(a)
			{
				case 'C':
				{
					col = *(str + 2) - '0';
					ASSERT((col >= 0) && (col <= 7));
					fontMakeCol(&color, col, hilight);
					str += 2;
				}
				break;

				case 'H':
				{
					hilight = *(str + 2) - '0';
					ASSERT((hilight >= 0) && (hilight <= 2));
					fontMakeCol(&color, col, hilight);
					str += 2;
				}
				break;

				case 'B':
				{
					blink = *(str + 2) - '0';
					ASSERT((blink >= 0) && (blink <= 2));
					str += 2;
				}
				break;

				case 'F':
				{
					font_index = *(str + 2) - '0';
					ASSERT((font_index >= FONT_SYS) && (font_index < FONT_NUM));
					font = &font_list[font_index];
					str += 2;
				}
				break;

				case 'E':
				{
					//-------------------
					// TODO:しばらく無視
					//-------------------
					str += 1;
				}
				break;
			}

			//-----------------------------------------------------
			// データエラーの場合、'$'を飛ばして文字列表示を続ける
			//-----------------------------------------------------
		}
		else
		{
			int u;
			int v;
			BOOL disp = TRUE;

			if(blink == 1)
			{
				disp = (g.time & 8) ? TRUE : FALSE;
			}
			else
			if(blink == 2)
			{
				disp = (g.time & 32) ? TRUE : FALSE;
			}

			if(disp)
			{
				u = (a & 0xf) * font->width;
				v = ((a & 0xf0) >> 4) * font->height;

				obj = GRPOBJ_QUAD(prio);
				GrpSetRGBA(obj, color.red, color.green, color.blue, color.alpha);
				GrpSetTexture(obj, font->texture);
				GrpSetPos(obj, put_x, put_y);
				GrpSetSize(obj, font->width, font->height);
				GrpSetUV(obj, u, v);
				GrpSetFilter(obj, TRUE);			/* FIXME:テスト */

			}
			put_x += font->next_x;
			chara_num += 1;
		}

		str += 1;
	}
#endif
}

//==============================================================
void FontPrintF(int x, int y, int prio, char *fmt, ...)
//--------------------------------------------------------------
// 表示(printf準拠)
//--------------------------------------------------------------
// in:	x, y = 表示開始位置
//		prio = プライオリティ
//		fmt	 = 表示フォーマット(以下 printf の可変長引数と同じ)
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	va_list arg;
	char work[1024];

	// 可変長引数の取得
	//------------------
	va_start(arg, fmt);
	vsprintf(work, fmt, arg);
	va_end(arg);

	FontPrint(x, y, prio, work);
}

//==============================================================
int FontGetPrintWidth(char *str)
//--------------------------------------------------------------
// 表示幅を取得
//--------------------------------------------------------------
// in:	str  = 表示文字列
//--------------------------------------------------------------
// out:	結果
//==============================================================
{
	return fontPrint(FONT_GET_WIDTH, 0, 0, 0, str);
}

void FontSetExColor(int index, sRGBA *col)
{
	font_col[index + 8] = *col;
}
