
#include "co_graph.h"
#include "co_memory.h"
#include "co_objlink.h"
#include "co_misc.h"
#include "co_strings.h"


#define GRPOBJ_MAXNUM  4096						// 管理数
#define GRP_CIRCLE_DIV  12						// 円描画時の分割数


struct _sGRPOBJ {
	int type;									// オブジェクトタイプ(enmGRP_TYPE)
	int prio;									// プライオリティ
	int blend_mode;								// アルファブレンディングモード(enmGRP_BLEND)
	sTexture *tex;								// 参照するテクスチャ

	FVector2 pos[4];							// 表示位置
	FVector2 uv[4];								// UV座標
	sRGBA col[4];								// 頂点カラー

	FVector2 disp_pos;							// 表示位置
	FVector2 center;							// スケーリング、回転中心
	FVector2 scale;								// スケーリング
	REAL rot;									// 回転(ラジアン)
	REAL draw_size;								// 点や線の描画サイズ(ドット)
	int line_num;								// ライン描画数
	REAL fan_angle;
	FVector2 donut_size;						// ドーナッツの内径

	BOOL flip_h;								// TRUE = 水平方向フリップ
	BOOL flip_v;								// TRUE = 垂直方向フリップ
	BOOL smooth;								// TRUE = エイリアシング ON
	BOOL filter;								// TRUE = フィルタリング ON

	BOOL vtx_direct;							// TRUE = ４頂点直接指定

	BOOL make_mtx;
};


static sLink *grp_link;
static sGRPOBJ *grp_list[GRPOBJ_MAXNUM];
static int grp_num, grp_num_prev;
static sTexture *cur_tex;
static BOOL mtx_identity;


//==============================================================
static void l_grpAddList(sGRPOBJ *obj)
//--------------------------------------------------------------
// プライオリティをチェックして登録
//--------------------------------------------------------------
// in:	obj  = ハンドル
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	int i;
	int num;
	sGRPOBJ **list;

	list = grp_list;
	num = grp_num;
	for(i = 0; i < num; ++i)
	{
		if(obj->prio > (*(list + i))->prio)
			break;
	}

	list = grp_list + num;
	num = num - i;
	for(i = 0; i < num; ++i)
	{
		*list = *(list - 1);
		list -= 1;
	}
	*list = obj;

	grp_num += 1;
}

//==============================================================
static void l_grpSetBlending(sGRPOBJ *obj)
//--------------------------------------------------------------
// ブレンディング設定
//--------------------------------------------------------------
// in:	obj = ハンドル
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	//--------------------------
	// TODO: 正しく実装する
	//--------------------------
	switch(obj->blend_mode)
	{
		case GRP_BLEND_NORMAL:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;

		case GRP_BLEND_ADD:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;

		case GRP_BLEND_REV:
		glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
		break;

		case GRP_BLEND_XOR:
		glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR);
		break;

		case GRP_BLEND_MUL:
		glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
		break;

		case GRP_BLEND_SCREEN:
		glBlendFunc(GL_DST_COLOR, GL_ONE);
		break;
	}
}

// 表示用行列作成
static void l_grpMakeMatrix(sGRPOBJ *obj)
{
	glLoadIdentity();							// 単位行列
	glTranslatef(obj->disp_pos.x, obj->disp_pos.y, 0);
	glScalef(obj->scale.x, obj->scale.y, 1.0);
	glRotatef(RAD2ANG(obj->rot), 0, 0, 1);
	glTranslatef(-obj->center.x, -obj->center.y, 0);
	mtx_identity = FALSE;
}

// 表示用行列作成
static void l_grpMakeIdentityMatrix(void)
{
	if(!mtx_identity)
	{
		glLoadIdentity();
		mtx_identity = TRUE;
		/* 正規化行列をなるべく作らないよう小細工 */
	}
}

//==============================================================
static void l_grpDrawPoint(sGRPOBJ *obj)
//--------------------------------------------------------------
// ポイント描画
//--------------------------------------------------------------
// in:	obj = オブジェクト
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	//--------------------------
	// セットアップ
	//--------------------------
	l_grpMakeIdentityMatrix();
	glDisable(GL_TEXTURE_2D);
	l_grpSetBlending(obj);
	glPointSize(obj->draw_size);
	if(obj->smooth)		glEnable(GL_POINT_SMOOTH);
	else				glDisable(GL_POINT_SMOOTH);

	//--------------------------
	// 描画
	//--------------------------
	glBegin(GL_POINTS);
	glColor4f(obj->col[0].red, obj->col[0].green, obj->col[0].blue, obj->col[0].alpha);
	glVertex2f(obj->disp_pos.x, obj->disp_pos.y);
	glEnd();
}

//==============================================================
static void l_grpDrawLine(sGRPOBJ *obj)
//--------------------------------------------------------------
// ライン描画
//--------------------------------------------------------------
// in:	obj = オブジェクト
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	int i;

	//--------------------------
	// セットアップ
	//--------------------------
	l_grpMakeIdentityMatrix();
	glDisable(GL_TEXTURE_2D);
	l_grpSetBlending(obj);
	glLineWidth(obj->draw_size);
	if(obj->smooth)		glEnable(GL_LINE_SMOOTH);
	else				glDisable(GL_LINE_SMOOTH);

	//--------------------------
	// 描画
	//--------------------------
	glBegin(GL_LINE_STRIP);
	for(i = 0; i < obj->line_num; ++i)
	{
		glColor4f(obj->col[i].red, obj->col[i].green, obj->col[i].blue, obj->col[i].alpha);
		glVertex2f(obj->pos[i].x, obj->pos[i].y);
	}
	glEnd();
}

//==============================================================
static void l_grpDrawBox(sGRPOBJ *obj)
//--------------------------------------------------------------
// ボックス描画
//--------------------------------------------------------------
// in:	obj = オブジェクト
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	//--------------------------
	// セットアップ
	//--------------------------
	l_grpMakeMatrix(obj);
	glDisable(GL_TEXTURE_2D);
	l_grpSetBlending(obj);
	glLineWidth(obj->draw_size);
	if(obj->smooth)		glEnable(GL_LINE_SMOOTH);
	else				glDisable(GL_LINE_SMOOTH);

	//--------------------------
	// 描画
	//--------------------------
	glBegin(GL_LINE_LOOP);
	if(obj->vtx_direct)
	{
		int i;

		//----------------------
		// ４頂点を直接指定
		//----------------------
		for(i = 0; i < 4; ++i)
		{
			glColor4f(obj->col[i].red, obj->col[i].green, obj->col[i].blue, obj->col[i].alpha);
			glVertex2f(obj->pos[i].x, obj->pos[i].y);
		}
	}
	else
	{
		//----------------------
		// 位置とサイズを指定
		//----------------------
		glColor4f(obj->col[0].red, obj->col[0].green, obj->col[0].blue, obj->col[0].alpha);
		glVertex2f(0, 0);

		glColor4f(obj->col[1].red, obj->col[1].green, obj->col[1].blue, obj->col[1].alpha);
		glVertex2f(0, obj->pos[1].y);

		glColor4f(obj->col[2].red, obj->col[2].green, obj->col[2].blue, obj->col[2].alpha);
		glVertex2f(obj->pos[1].x, obj->pos[1].y);

		glColor4f(obj->col[3].red, obj->col[3].green, obj->col[3].blue, obj->col[3].alpha);
		glVertex2f(obj->pos[1].x, 0);
	}
	glEnd();
}

//==============================================================
static void l_grpDrawQuad(sGRPOBJ *obj)
//--------------------------------------------------------------
// 矩形描画
//--------------------------------------------------------------
// in:	obj = オブジェクト
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	FVector2 uv = { 1.0f, 1.0f };
	FVector2 tex_size = { 1.0f, 1.0f };
	float x_ofs = 0.0f;
	float y_ofs = 0.0f;

	//--------------------------
	// セットアップ
	//--------------------------
	if(obj->make_mtx)
	{
		l_grpMakeMatrix(obj);						// 変換行列を準備
	}
	else
	{
		x_ofs = obj->disp_pos.x;
		y_ofs = obj->disp_pos.y;
		l_grpMakeIdentityMatrix();
	}
	l_grpSetBlending(obj);
	if(obj->tex)
	{
		GLint magfilter;
		GLint minfilter;

		glEnable(GL_TEXTURE_2D);
		if(obj->tex != cur_tex)
		{
			TexBind(obj->tex);
			cur_tex = obj->tex;
		}
		TexGetUV(&uv, obj->tex);
		TexGetSize(&tex_size, obj->tex);

		if(obj->filter)
		{
			magfilter = GL_LINEAR;
			minfilter = GL_LINEAR;
		}
		else
		{
			magfilter = GL_NEAREST;
			minfilter = GL_NEAREST;
		}
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magfilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minfilter);
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
	}
/* 	if(obj->smooth)		glEnable(GL_POLYGON_SMOOTH); */
/* 	else				glDisable(GL_POLYGON_SMOOTH); */

	//--------------------------
	// 描画
	//--------------------------
	glBegin(GL_QUADS);
	if(obj->vtx_direct)
	{
		int i;

		//----------------------
		// ４頂点を直接指定
		//----------------------
		for(i = 0; i < 4; ++i)
		{
			glColor4f(obj->col[i].red, obj->col[i].green, obj->col[i].blue, obj->col[i].alpha);
			glTexCoord2f((obj->uv[i].x / tex_size.x) * uv.x, (obj->uv[i].y / tex_size.y) * uv.y);
			glVertex2f(obj->pos[i].x, obj->pos[i].y);
		}
	}
	else
	{
		//----------------------
		// 位置とサイズを指定
		//----------------------
		FVector2 uv_top = { 0.0f, 0.0f };
		FVector2 uv_bottom = { 1.0f, 1.0f };

		if(obj->tex)
		{
			uv_top.x = (obj->uv[0].x / tex_size.x) * uv.x;
			uv_top.y = (obj->uv[0].y / tex_size.y) * uv.y;
			uv_bottom.x = ((obj->uv[0].x + obj->pos[1].x) / tex_size.x) * uv.x;
			uv_bottom.y = ((obj->uv[0].y + obj->pos[1].y) / tex_size.y) * uv.y;
			if(obj->flip_h)
			{
				REAL a;

				a = uv_top.x;
				uv_top.x = uv_bottom.x;
				uv_bottom.x = a;
			}
			if(obj->flip_v)
			{
				REAL a;

				a = uv_top.y;
				uv_top.y = uv_bottom.y;
				uv_bottom.y = a;
			}
		}

#if 0
		GLfloat col[4 * 1 * 4];
		GLfloat uv[2 * 1 * 4];
		GLfloat vtx[2 * 1 * 4];
		GLuint idx[4] = { 0, 1, 2, 3 };
		
		for(int i = 0; i < 4; i += 1)
		{
			col[i * 4 + 0] = obj->col[i].red;
			col[i * 4 + 1] = obj->col[i].green;
			col[i * 4 + 2] = obj->col[i].blue;
			col[i * 4 + 3] = obj->col[i].alpha;
		}
		uv[0 * 2 + 0] = uv_top.x;
		uv[0 * 2 + 1] = uv_top.y;
		uv[1 * 2 + 0] = uv_top.x;
		uv[1 * 2 + 1] = uv_bottom.y;
		uv[2 * 2 + 0] = uv_bottom.x;
		uv[2 * 2 + 1] = uv_bottom.y;
		uv[3 * 2 + 0] = uv_bottom.x;
		uv[3 * 2 + 1] = uv_top.y;

		vtx[0 * 2 + 0] = 0.0f;
		vtx[0 * 2 + 1] = 0.0f;
		vtx[1 * 2 + 0] = 0.0f;
		vtx[1 * 2 + 1] = obj->pos[1].y;
		vtx[2 * 2 + 0] = obj->pos[1].x;
		vtx[2 * 2 + 1] = obj->pos[1].y;
		vtx[3 * 2 + 0] = obj->pos[1].x;
		vtx[3 * 2 + 1] = 0.0f;
		
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glVertexPointer(2, GL_FLOAT, 0, vtx);
		glColorPointer(4, GL_FLOAT, 0, col);
		glTexCoordPointer(2, GL_FLOAT, 0, uv);

		glDrawElements(GL_QUADS, 1 * 4, GL_UNSIGNED_INT, idx);
		
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#else
		glColor4f(obj->col[0].red, obj->col[0].green, obj->col[0].blue, obj->col[0].alpha);
		glTexCoord2f(uv_top.x, uv_top.y);
		glVertex2f(x_ofs, y_ofs);

		glColor4f(obj->col[1].red, obj->col[1].green, obj->col[1].blue, obj->col[1].alpha);
		glTexCoord2f(uv_top.x, uv_bottom.y);
		glVertex2f(x_ofs, y_ofs + obj->pos[1].y);

		glColor4f(obj->col[2].red, obj->col[2].green, obj->col[2].blue, obj->col[2].alpha);
		glTexCoord2f(uv_bottom.x, uv_bottom.y);
		glVertex2f(x_ofs + obj->pos[1].x, y_ofs + obj->pos[1].y);

		glColor4f(obj->col[3].red, obj->col[3].green, obj->col[3].blue, obj->col[3].alpha);
		glTexCoord2f(uv_bottom.x, uv_top.y);
		glVertex2f(x_ofs + obj->pos[1].x, y_ofs);
#endif
	}
	glEnd();
}

//==============================================================
static void l_grpDrawCircle(sGRPOBJ *obj)
//--------------------------------------------------------------
// 円描画
//--------------------------------------------------------------
// in:	obj = オブジェクト
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	int i;

	//--------------------------
	// セットアップ
	//--------------------------
	l_grpMakeMatrix(obj);
	glDisable(GL_TEXTURE_2D);
	l_grpSetBlending(obj);
	glLineWidth(obj->draw_size);
	if(obj->smooth)		glEnable(GL_LINE_SMOOTH);
	else				glDisable(GL_LINE_SMOOTH);

	//--------------------------
	// 描画
	//--------------------------
	glBegin(GL_LINE_LOOP);

	// FIXME: 分割数は、半径から割り出すようにする
	//---------------------------------------------
	for(i = 0; i < obj->line_num; ++i)
	{
		REAL r;

		r = (PI * 2.0f * (float)i) / obj->line_num;
		glColor4f(obj->col[0].red, obj->col[0].green, obj->col[0].blue, obj->col[0].alpha);
		glVertex2f(obj->pos[1].x * sinf(r), obj->pos[1].y * cosf(r));
	}
	glEnd();
}

//==============================================================
static void l_grpDrawFillCircle(sGRPOBJ *obj)
//--------------------------------------------------------------
// 塗りつぶし円描画
//--------------------------------------------------------------
// in:	obj = オブジェクト
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	int i;

	//--------------------------
	// セットアップ
	//--------------------------
	l_grpMakeMatrix(obj);
	glDisable(GL_TEXTURE_2D);
	l_grpSetBlending(obj);
//	glLineWidth(obj->draw_size);
/* 	if(obj->smooth)		glEnable(GL_POLYGON_SMOOTH); */
/* 	else				glDisable(GL_POLYGON_SMOOTH); */

	//--------------------------
	// 描画
	//--------------------------
	glBegin(GL_TRIANGLE_FAN);
	glColor4f(obj->col[0].red, obj->col[0].green, obj->col[0].blue, obj->col[0].alpha);
	glVertex2f(0.0f, 0.0f);

	for(i = 0; i <= obj->line_num; ++i)
	{
		REAL r;

		r = (PI * 2.0f * (float)i) / obj->line_num;

		glColor4f(obj->col[1].red, obj->col[1].green, obj->col[1].blue, obj->col[1].alpha);
		glVertex2f(obj->pos[1].x * sinf(r), obj->pos[1].y * cosf(r));
	}
	glEnd();
}

//==============================================================
static void l_grpDrawTriangle(sGRPOBJ *obj)
//--------------------------------------------------------------
// 三角ポリゴン描画
//--------------------------------------------------------------
// in:	obj = オブジェクト
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	//--------------------------
	// セットアップ
	//--------------------------
	l_grpMakeMatrix(obj);						// 変換行列を準備
	glDisable(GL_TEXTURE_2D);
	l_grpSetBlending(obj);
//	glLineWidth(obj->draw_size);
/* 	if(obj->smooth)		glEnable(GL_POLYGON_SMOOTH); */
/* 	else				glDisable(GL_POLYGON_SMOOTH); */

	//--------------------------
	// 描画
	//--------------------------
	glBegin(GL_TRIANGLES);
	for(int i = 0; i < 3; ++i)
	{
		glColor4f(obj->col[i].red, obj->col[i].green, obj->col[i].blue, obj->col[i].alpha);
		glVertex2f(obj->pos[i].x, obj->pos[i].y);
	}
	glEnd();
}

// 扇描画
static void l_grpDrawFan(sGRPOBJ *obj)
{
	int i;

	l_grpMakeMatrix(obj);
	glDisable(GL_TEXTURE_2D);
	l_grpSetBlending(obj);
	glLineWidth(obj->draw_size);
	if(obj->smooth)		glEnable(GL_LINE_SMOOTH);
	else				glDisable(GL_LINE_SMOOTH);

	glBegin(GL_LINE_LOOP);

	glColor4f(obj->col[0].red, obj->col[0].green, obj->col[0].blue, obj->col[0].alpha);
	glVertex2f(0.0f, 0.0f);

	for(i = 0; i <= obj->line_num; ++i)
	{
		REAL r;

		r = (obj->fan_angle * (float)i) / obj->line_num;
		glColor4f(obj->col[0].red, obj->col[0].green, obj->col[0].blue, obj->col[0].alpha);
		glVertex2f(obj->pos[1].x * sinf(r), obj->pos[1].y * -cosf(r));
	}
	glEnd();
}

// ドーナッツ描画
static void l_grpDrawFillDonut(sGRPOBJ *obj)
{
	l_grpMakeMatrix(obj);
	glDisable(GL_TEXTURE_2D);
	l_grpSetBlending(obj);

	glBegin(GL_TRIANGLE_STRIP);
	for(int i = 0; i <= obj->line_num; i += 1)
	{
		REAL r;

		r = (PI * 2.0f * (float)i) / obj->line_num;
		float sin_r = sinf(r);
		float cos_r = cosf(r);

		glColor4f(obj->col[1].red, obj->col[1].green, obj->col[1].blue, obj->col[1].alpha);
		glVertex2f(obj->pos[1].x * sin_r, obj->pos[1].y * cos_r);

		glColor4f(obj->col[0].red, obj->col[0].green, obj->col[0].blue, obj->col[0].alpha);
		glVertex2f(obj->pos[2].x * sin_r, obj->pos[2].y * cos_r);
	}
	glEnd();
}


//==============================================================
void GrpInit(void)
//--------------------------------------------------------------
// 初期化
//--------------------------------------------------------------
// in:	なし
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	grp_link = ObjLinkCreate(sizeof(sGRPOBJ), GRPOBJ_MAXNUM, MEM_SYS, FALSE);
	grp_num_prev = grp_num = 0;
	GrpSetup();

	//--------------------------
	// 各種精度の設定
	// GL_FASTEST
	// GL_NICEST
	//--------------------------
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
//	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	SYSINFO(".... graphic initialize");
}

//==============================================================
void GrpFin(void)
//--------------------------------------------------------------
// 終了
//--------------------------------------------------------------
// in:	なし
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	ObjLinkDestroy(grp_link);

	SYSINFO(".... graphic finish");
}

//==============================================================
void GrpSetup(void)
//--------------------------------------------------------------
// セットアップ
//--------------------------------------------------------------
// in:	なし
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	ObjLinkDelAll(grp_link);
	grp_num_prev = grp_num;
	grp_num = 0;
}

//==============================================================
void GrpDraw(void)
//--------------------------------------------------------------
// 描画
//--------------------------------------------------------------
// in:	なし
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	sGRPOBJ **list;
	int i;

	glEnable(GL_BLEND);
	glMatrixMode(GL_MODELVIEW);

	cur_tex = NULL;
	mtx_identity = FALSE;
	list = grp_list;
	for(i = 0; i < grp_num; i += 1)
	{
		sGRPOBJ *obj;

		obj = *list;
		switch(obj->type)
		{
			case GRP_POINT:
			{
				l_grpDrawPoint(obj);
			}
			break;

			case GRP_LINE:
			{
				l_grpDrawLine(obj);
			}
			break;

			case GRP_BOX:
			{
				l_grpDrawBox(obj);
			}
			break;

			case GRP_QUAD:
			{
				l_grpDrawQuad(obj);
			}
			break;

			case GRP_CIRCLE:
			{
				l_grpDrawCircle(obj);
			}
			break;

			case GRP_FILLCIRCLE:
			{
				l_grpDrawFillCircle(obj);
			}
			break;

			case GRP_TRIANGLE:
			{
				l_grpDrawTriangle(obj);
			}
			break;

		case GRP_FAN:
			{
				l_grpDrawFan(obj);
			}
			break;

		case GRP_FILLDONUT:
			{
				l_grpDrawFillDonut(obj);
			}
			break;
		}
		list += 1;
	}
	glDisable(GL_BLEND);
}

//==============================================================
sGRPOBJ *GrpCreate(int type, int prio)
//--------------------------------------------------------------
// オブジェクトを生成
//--------------------------------------------------------------
// in:	type   = enmGRP_TYPE
//		prio   = プライオリティ
//--------------------------------------------------------------
// out:	生成したオブジェクト(NULL = 作成失敗)
//==============================================================
{
	sGRPOBJ *obj;

	obj = (sGRPOBJ *)ObjLinkNew(grp_link);
	if(obj)
	{
		int i;

		// 初期値設定
		obj->type = type;
		obj->prio = prio;
		SetV2d(&obj->disp_pos, 0.0f, 0.0f);
		SetV2d(&obj->scale, 1.0f, 1.0f);
		obj->draw_size = 1.0f;
		obj->line_num = 2;
		for(i = 0; i < 4; ++i)
		{
			SetRGBA(&obj->col[i], 1.0f, 1.0f, 1.0f, 1.0f);
		}

		if((type == GRP_CIRCLE) || (type == GRP_FILLCIRCLE))	obj->line_num = GRP_CIRCLE_DIV;

		// リストに登録
		l_grpAddList(obj);
	}

	return obj;
}

//==============================================================
void GrpSetRGBA(sGRPOBJ *obj, REAL red, REAL green, REAL blue, REAL alpha)
//--------------------------------------------------------------
// 色設定
//--------------------------------------------------------------
// in:	obj   = オブジェクト
//		red   = 赤成分
//		green = 緑成分
//		blue  = 青成分
//		alpha = α成分
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	int i;

	for(i = 0; i < 4; ++i)
	{
		SetRGBA(&obj->col[i], red, green, blue, alpha);
	}
}

//==============================================================
void GrpSetRGBA4(sGRPOBJ *obj, sRGBA *c1, sRGBA *c2, sRGBA *c3, sRGBA *c4)
//--------------------------------------------------------------
// 色設定
//--------------------------------------------------------------
// in:	obj    = オブジェクト
//		c1～c4 = 設定値(NULL = 省略)
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	if(c1)
	{
		SetRGBA(&obj->col[0], c1->red, c1->green, c1->blue, c1->alpha);
	}
	if(c2)
	{
		SetRGBA(&obj->col[1], c2->red, c2->green, c2->blue, c2->alpha);
	}
	if(c3)
	{
		SetRGBA(&obj->col[2], c3->red, c3->green, c3->blue, c3->alpha);
	}
	if(c4)
	{
		SetRGBA(&obj->col[3], c4->red, c4->green, c4->blue, c4->alpha);
	}
}

//==============================================================
void GrpSetPos(sGRPOBJ *obj, REAL x, REAL y)
//--------------------------------------------------------------
// 座標設定
//--------------------------------------------------------------
// in:	obj  = オブジェクト
//		x, y = 設定値
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	SetV2d(&obj->disp_pos, x, y);
}

//==============================================================
void GrpSetSize(sGRPOBJ *obj, REAL w, REAL h)
//--------------------------------------------------------------
// サイズ設定
//--------------------------------------------------------------
// in:	obj  = オブジェクト
//		w, h = 設定値
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	obj->pos[1].x = w;
	obj->pos[1].y = h;
	obj->vtx_direct = FALSE;
}

//==============================================================
void GrpSetVtx(sGRPOBJ *obj, FVector2 *v1, FVector2 *v2, FVector2 *v3, FVector2 *v4)
//--------------------------------------------------------------
// 座標設定(４頂点)
//--------------------------------------------------------------
// in:	obj    = オブジェクト
//		v1～v4 = 設定値(NULL = 省略)
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	if(v1)
	{
		SetV2d(&obj->pos[0], v1->x, v1->y);
	}
	if(v2)
	{
		SetV2d(&obj->pos[1], v2->x, v2->y);
	}
	if(v3)
	{
		SetV2d(&obj->pos[2], v3->x, v3->y);
	}
	if(v4)
	{
		SetV2d(&obj->pos[3], v4->x, v4->y);
	}
	obj->vtx_direct = TRUE;
}

//==============================================================
void GrpSetUV(sGRPOBJ *obj, REAL x, REAL y)
//--------------------------------------------------------------
// UV座標設定
//--------------------------------------------------------------
// in:	obj  = オブジェクト
//		x, y = 設定値
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	obj->uv[0].x = x;
	obj->uv[0].y = y;
}

//==============================================================
void GrpSetUV4(sGRPOBJ *obj, FVector2 *v1, FVector2 *v2, FVector2 *v3, FVector2 *v4)
//--------------------------------------------------------------
// UV座標設定(４頂点まとめて設定)
//--------------------------------------------------------------
// in:	obj    = オブジェクト
//		v1～v4 = 設定値
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	SetV2d(&obj->uv[0], v1->x, v1->y);
	SetV2d(&obj->uv[1], v2->x, v2->y);
	SetV2d(&obj->uv[2], v3->x, v3->y);
	SetV2d(&obj->uv[3], v4->x, v4->y);
}

//==============================================================
void GrpSetDrawSize(sGRPOBJ *obj, REAL size)
//--------------------------------------------------------------
// 点や線の描画サイズ設定
//--------------------------------------------------------------
// in:	obj  = オブジェクト
//		size = 設定値
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	obj->draw_size = size;
}

//==============================================================
void GrpSetLineNum(sGRPOBJ *obj, int num)
//--------------------------------------------------------------
// ラインの描画数を設定
// ※円弧の分割数の変更も兼用しています
//--------------------------------------------------------------
// in:	obj = オブジェクト
//		num = 設定値
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	obj->line_num = num;
}

// 扇形の角度を設定
void GrpSetFanAngle(sGRPOBJ *obj, float angle)
{
	obj->fan_angle = angle;
}

// ドーナツのサイズを設定
void GrpSetDonutSize(sGRPOBJ *obj, REAL w1, REAL h1, REAL w2, REAL h2)
{
	obj->pos[1].x = w1;
	obj->pos[1].y = h1;
	obj->pos[2].x = w2;
	obj->pos[2].y = h2;
	obj->vtx_direct = FALSE;
}

//==============================================================
void GrpSetCenter(sGRPOBJ *obj, REAL x, REAL y)
//--------------------------------------------------------------
// スケーリング、回転中心を設定
//--------------------------------------------------------------
// in:	obj  = オブジェクト
//		x, y = 設定値
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	obj->center.x = x;
	obj->center.y = y;
	obj->make_mtx = TRUE;
}

//==============================================================
void GrpSetScale(sGRPOBJ *obj, REAL x, REAL y)
//--------------------------------------------------------------
// スケール値を設定
//--------------------------------------------------------------
// in:	obj  = オブジェクト
//		x, y = 設定値
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	obj->scale.x = x;
	obj->scale.y = y;
	obj->make_mtx = TRUE;
}

//==============================================================
void GrpSetRot(sGRPOBJ *obj, REAL r)
//--------------------------------------------------------------
// 回転角を設定
//--------------------------------------------------------------
// in:	obj = オブジェクト
//		r   = 設定値(ラジアン)
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	obj->rot = r;
	obj->make_mtx = TRUE;
}

//==============================================================
void GrpSetFlip(sGRPOBJ *obj, BOOL flip_h, BOOL flip_v)
//--------------------------------------------------------------
// フリップ設定
//--------------------------------------------------------------
// in:	obj    = オブジェクト
//		flip_h = TRUE: 水平方向フリップ
//		flip_v = TRUE: 垂直方向フリップ
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	obj->flip_h = flip_h;
	obj->flip_v = flip_v;
}

//==============================================================
void GrpSetBlendMode(sGRPOBJ *obj, int mode)
//--------------------------------------------------------------
// ブレンディングモード設定
//--------------------------------------------------------------
// in:	obj  = オブジェクト
//		mode = ブレンディングモード(enmGRP_BLEND)
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	obj->blend_mode = mode;
}

//==============================================================
void GrpSetTexture(sGRPOBJ *obj, sTexture *tex)
//--------------------------------------------------------------
// テクスチャを設定
//--------------------------------------------------------------
// in:	obj = オブジェクト
//		tex = テクスチャ(NULL = 無効)
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	obj->tex = tex;
}

//==============================================================
void GrpSetSmooth(sGRPOBJ *obj, BOOL smooth)
//--------------------------------------------------------------
// エイリアシングモードを変更
//--------------------------------------------------------------
// in:	obj    = オブジェクト
//		smooth = TRUE : エイリアシング ON
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	obj->smooth = smooth;
}

//==============================================================
BOOL GrpGetSmooth(sGRPOBJ *obj)
//--------------------------------------------------------------
// エイリアシングモードを取得
//--------------------------------------------------------------
// in:	obj = オブジェクト
//--------------------------------------------------------------
// out:	TRUE : エイリアシング ON
//==============================================================
{
	return obj->smooth;
}

//==============================================================
void GrpSetFilter(sGRPOBJ *obj, BOOL filter)
//--------------------------------------------------------------
// フィルタリングモードを変更
//--------------------------------------------------------------
// in:	obj    = オブジェクト
//		filter = TRUE: テクスチャフィルタリング ON
//--------------------------------------------------------------
// out:	なし
//==============================================================
{
	obj->filter = filter;
}

//==============================================================
BOOL GrpGetFilter(sGRPOBJ *obj, BOOL filter)
//--------------------------------------------------------------
// フィルタリングモードを取得
//--------------------------------------------------------------
// in:	obj = オブジェクト
//--------------------------------------------------------------
// out:	TRUE: テクスチャフィルタリング ON
//==============================================================
{
	return obj->filter;
}

#ifdef DEBUG
int GrpGetDrawPrimNum(void)
{
	return grp_num_prev;
}
#endif
