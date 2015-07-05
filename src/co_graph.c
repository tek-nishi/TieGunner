
#include "co_graph.h"
#include "co_memory.h"
#include "co_objlink.h"
#include "co_misc.h"
#include "co_strings.h"


#define GRPOBJ_MAXNUM  4096						// �Ǘ���
#define GRP_CIRCLE_DIV  12						// �~�`�掞�̕�����


struct _sGRPOBJ {
	int type;									// �I�u�W�F�N�g�^�C�v(enmGRP_TYPE)
	int prio;									// �v���C�I���e�B
	int blend_mode;								// �A���t�@�u�����f�B���O���[�h(enmGRP_BLEND)
	sTexture *tex;								// �Q�Ƃ���e�N�X�`��

	FVector2 pos[4];							// �\���ʒu
	FVector2 uv[4];								// UV���W
	sRGBA col[4];								// ���_�J���[

	FVector2 disp_pos;							// �\���ʒu
	FVector2 center;							// �X�P�[�����O�A��]���S
	FVector2 scale;								// �X�P�[�����O
	REAL rot;									// ��](���W�A��)
	REAL draw_size;								// �_����̕`��T�C�Y(�h�b�g)
	int line_num;								// ���C���`�搔
	REAL fan_angle;
	FVector2 donut_size;						// �h�[�i�b�c�̓��a

	BOOL flip_h;								// TRUE = ���������t���b�v
	BOOL flip_v;								// TRUE = ���������t���b�v
	BOOL smooth;								// TRUE = �G�C���A�V���O ON
	BOOL filter;								// TRUE = �t�B���^�����O ON

	BOOL vtx_direct;							// TRUE = �S���_���ڎw��

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
// �v���C�I���e�B���`�F�b�N���ēo�^
//--------------------------------------------------------------
// in:	obj  = �n���h��
//--------------------------------------------------------------
// out:	�Ȃ�
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
// �u�����f�B���O�ݒ�
//--------------------------------------------------------------
// in:	obj = �n���h��
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	//--------------------------
	// TODO: ��������������
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

// �\���p�s��쐬
static void l_grpMakeMatrix(sGRPOBJ *obj)
{
	glLoadIdentity();							// �P�ʍs��
	glTranslatef(obj->disp_pos.x, obj->disp_pos.y, 0);
	glScalef(obj->scale.x, obj->scale.y, 1.0);
	glRotatef(RAD2ANG(obj->rot), 0, 0, 1);
	glTranslatef(-obj->center.x, -obj->center.y, 0);
	mtx_identity = FALSE;
}

// �\���p�s��쐬
static void l_grpMakeIdentityMatrix(void)
{
	if(!mtx_identity)
	{
		glLoadIdentity();
		mtx_identity = TRUE;
		/* ���K���s����Ȃ�ׂ����Ȃ��悤���׍H */
	}
}

//==============================================================
static void l_grpDrawPoint(sGRPOBJ *obj)
//--------------------------------------------------------------
// �|�C���g�`��
//--------------------------------------------------------------
// in:	obj = �I�u�W�F�N�g
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	//--------------------------
	// �Z�b�g�A�b�v
	//--------------------------
	l_grpMakeIdentityMatrix();
	glDisable(GL_TEXTURE_2D);
	l_grpSetBlending(obj);
	glPointSize(obj->draw_size);
	if(obj->smooth)		glEnable(GL_POINT_SMOOTH);
	else				glDisable(GL_POINT_SMOOTH);

	//--------------------------
	// �`��
	//--------------------------
	glBegin(GL_POINTS);
	glColor4f(obj->col[0].red, obj->col[0].green, obj->col[0].blue, obj->col[0].alpha);
	glVertex2f(obj->disp_pos.x, obj->disp_pos.y);
	glEnd();
}

//==============================================================
static void l_grpDrawLine(sGRPOBJ *obj)
//--------------------------------------------------------------
// ���C���`��
//--------------------------------------------------------------
// in:	obj = �I�u�W�F�N�g
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	int i;

	//--------------------------
	// �Z�b�g�A�b�v
	//--------------------------
	l_grpMakeIdentityMatrix();
	glDisable(GL_TEXTURE_2D);
	l_grpSetBlending(obj);
	glLineWidth(obj->draw_size);
	if(obj->smooth)		glEnable(GL_LINE_SMOOTH);
	else				glDisable(GL_LINE_SMOOTH);

	//--------------------------
	// �`��
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
// �{�b�N�X�`��
//--------------------------------------------------------------
// in:	obj = �I�u�W�F�N�g
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	//--------------------------
	// �Z�b�g�A�b�v
	//--------------------------
	l_grpMakeMatrix(obj);
	glDisable(GL_TEXTURE_2D);
	l_grpSetBlending(obj);
	glLineWidth(obj->draw_size);
	if(obj->smooth)		glEnable(GL_LINE_SMOOTH);
	else				glDisable(GL_LINE_SMOOTH);

	//--------------------------
	// �`��
	//--------------------------
	glBegin(GL_LINE_LOOP);
	if(obj->vtx_direct)
	{
		int i;

		//----------------------
		// �S���_�𒼐ڎw��
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
		// �ʒu�ƃT�C�Y���w��
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
// ��`�`��
//--------------------------------------------------------------
// in:	obj = �I�u�W�F�N�g
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	FVector2 uv = { 1.0f, 1.0f };
	FVector2 tex_size = { 1.0f, 1.0f };
	float x_ofs = 0.0f;
	float y_ofs = 0.0f;

	//--------------------------
	// �Z�b�g�A�b�v
	//--------------------------
	if(obj->make_mtx)
	{
		l_grpMakeMatrix(obj);						// �ϊ��s�������
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
	// �`��
	//--------------------------
	glBegin(GL_QUADS);
	if(obj->vtx_direct)
	{
		int i;

		//----------------------
		// �S���_�𒼐ڎw��
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
		// �ʒu�ƃT�C�Y���w��
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
// �~�`��
//--------------------------------------------------------------
// in:	obj = �I�u�W�F�N�g
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	int i;

	//--------------------------
	// �Z�b�g�A�b�v
	//--------------------------
	l_grpMakeMatrix(obj);
	glDisable(GL_TEXTURE_2D);
	l_grpSetBlending(obj);
	glLineWidth(obj->draw_size);
	if(obj->smooth)		glEnable(GL_LINE_SMOOTH);
	else				glDisable(GL_LINE_SMOOTH);

	//--------------------------
	// �`��
	//--------------------------
	glBegin(GL_LINE_LOOP);

	// FIXME: �������́A���a���犄��o���悤�ɂ���
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
// �h��Ԃ��~�`��
//--------------------------------------------------------------
// in:	obj = �I�u�W�F�N�g
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	int i;

	//--------------------------
	// �Z�b�g�A�b�v
	//--------------------------
	l_grpMakeMatrix(obj);
	glDisable(GL_TEXTURE_2D);
	l_grpSetBlending(obj);
//	glLineWidth(obj->draw_size);
/* 	if(obj->smooth)		glEnable(GL_POLYGON_SMOOTH); */
/* 	else				glDisable(GL_POLYGON_SMOOTH); */

	//--------------------------
	// �`��
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
// �O�p�|���S���`��
//--------------------------------------------------------------
// in:	obj = �I�u�W�F�N�g
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	//--------------------------
	// �Z�b�g�A�b�v
	//--------------------------
	l_grpMakeMatrix(obj);						// �ϊ��s�������
	glDisable(GL_TEXTURE_2D);
	l_grpSetBlending(obj);
//	glLineWidth(obj->draw_size);
/* 	if(obj->smooth)		glEnable(GL_POLYGON_SMOOTH); */
/* 	else				glDisable(GL_POLYGON_SMOOTH); */

	//--------------------------
	// �`��
	//--------------------------
	glBegin(GL_TRIANGLES);
	for(int i = 0; i < 3; ++i)
	{
		glColor4f(obj->col[i].red, obj->col[i].green, obj->col[i].blue, obj->col[i].alpha);
		glVertex2f(obj->pos[i].x, obj->pos[i].y);
	}
	glEnd();
}

// ��`��
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

// �h�[�i�b�c�`��
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
// ������
//--------------------------------------------------------------
// in:	�Ȃ�
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	grp_link = ObjLinkCreate(sizeof(sGRPOBJ), GRPOBJ_MAXNUM, MEM_SYS, FALSE);
	grp_num_prev = grp_num = 0;
	GrpSetup();

	//--------------------------
	// �e�퐸�x�̐ݒ�
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
// �I��
//--------------------------------------------------------------
// in:	�Ȃ�
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	ObjLinkDestroy(grp_link);

	SYSINFO(".... graphic finish");
}

//==============================================================
void GrpSetup(void)
//--------------------------------------------------------------
// �Z�b�g�A�b�v
//--------------------------------------------------------------
// in:	�Ȃ�
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	ObjLinkDelAll(grp_link);
	grp_num_prev = grp_num;
	grp_num = 0;
}

//==============================================================
void GrpDraw(void)
//--------------------------------------------------------------
// �`��
//--------------------------------------------------------------
// in:	�Ȃ�
//--------------------------------------------------------------
// out:	�Ȃ�
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
// �I�u�W�F�N�g�𐶐�
//--------------------------------------------------------------
// in:	type   = enmGRP_TYPE
//		prio   = �v���C�I���e�B
//--------------------------------------------------------------
// out:	���������I�u�W�F�N�g(NULL = �쐬���s)
//==============================================================
{
	sGRPOBJ *obj;

	obj = (sGRPOBJ *)ObjLinkNew(grp_link);
	if(obj)
	{
		int i;

		// �����l�ݒ�
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

		// ���X�g�ɓo�^
		l_grpAddList(obj);
	}

	return obj;
}

//==============================================================
void GrpSetRGBA(sGRPOBJ *obj, REAL red, REAL green, REAL blue, REAL alpha)
//--------------------------------------------------------------
// �F�ݒ�
//--------------------------------------------------------------
// in:	obj   = �I�u�W�F�N�g
//		red   = �Ԑ���
//		green = �ΐ���
//		blue  = ����
//		alpha = ������
//--------------------------------------------------------------
// out:	�Ȃ�
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
// �F�ݒ�
//--------------------------------------------------------------
// in:	obj    = �I�u�W�F�N�g
//		c1�`c4 = �ݒ�l(NULL = �ȗ�)
//--------------------------------------------------------------
// out:	�Ȃ�
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
// ���W�ݒ�
//--------------------------------------------------------------
// in:	obj  = �I�u�W�F�N�g
//		x, y = �ݒ�l
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	SetV2d(&obj->disp_pos, x, y);
}

//==============================================================
void GrpSetSize(sGRPOBJ *obj, REAL w, REAL h)
//--------------------------------------------------------------
// �T�C�Y�ݒ�
//--------------------------------------------------------------
// in:	obj  = �I�u�W�F�N�g
//		w, h = �ݒ�l
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	obj->pos[1].x = w;
	obj->pos[1].y = h;
	obj->vtx_direct = FALSE;
}

//==============================================================
void GrpSetVtx(sGRPOBJ *obj, FVector2 *v1, FVector2 *v2, FVector2 *v3, FVector2 *v4)
//--------------------------------------------------------------
// ���W�ݒ�(�S���_)
//--------------------------------------------------------------
// in:	obj    = �I�u�W�F�N�g
//		v1�`v4 = �ݒ�l(NULL = �ȗ�)
//--------------------------------------------------------------
// out:	�Ȃ�
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
// UV���W�ݒ�
//--------------------------------------------------------------
// in:	obj  = �I�u�W�F�N�g
//		x, y = �ݒ�l
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	obj->uv[0].x = x;
	obj->uv[0].y = y;
}

//==============================================================
void GrpSetUV4(sGRPOBJ *obj, FVector2 *v1, FVector2 *v2, FVector2 *v3, FVector2 *v4)
//--------------------------------------------------------------
// UV���W�ݒ�(�S���_�܂Ƃ߂Đݒ�)
//--------------------------------------------------------------
// in:	obj    = �I�u�W�F�N�g
//		v1�`v4 = �ݒ�l
//--------------------------------------------------------------
// out:	�Ȃ�
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
// �_����̕`��T�C�Y�ݒ�
//--------------------------------------------------------------
// in:	obj  = �I�u�W�F�N�g
//		size = �ݒ�l
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	obj->draw_size = size;
}

//==============================================================
void GrpSetLineNum(sGRPOBJ *obj, int num)
//--------------------------------------------------------------
// ���C���̕`�搔��ݒ�
// ���~�ʂ̕������̕ύX�����p���Ă��܂�
//--------------------------------------------------------------
// in:	obj = �I�u�W�F�N�g
//		num = �ݒ�l
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	obj->line_num = num;
}

// ��`�̊p�x��ݒ�
void GrpSetFanAngle(sGRPOBJ *obj, float angle)
{
	obj->fan_angle = angle;
}

// �h�[�i�c�̃T�C�Y��ݒ�
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
// �X�P�[�����O�A��]���S��ݒ�
//--------------------------------------------------------------
// in:	obj  = �I�u�W�F�N�g
//		x, y = �ݒ�l
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	obj->center.x = x;
	obj->center.y = y;
	obj->make_mtx = TRUE;
}

//==============================================================
void GrpSetScale(sGRPOBJ *obj, REAL x, REAL y)
//--------------------------------------------------------------
// �X�P�[���l��ݒ�
//--------------------------------------------------------------
// in:	obj  = �I�u�W�F�N�g
//		x, y = �ݒ�l
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	obj->scale.x = x;
	obj->scale.y = y;
	obj->make_mtx = TRUE;
}

//==============================================================
void GrpSetRot(sGRPOBJ *obj, REAL r)
//--------------------------------------------------------------
// ��]�p��ݒ�
//--------------------------------------------------------------
// in:	obj = �I�u�W�F�N�g
//		r   = �ݒ�l(���W�A��)
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	obj->rot = r;
	obj->make_mtx = TRUE;
}

//==============================================================
void GrpSetFlip(sGRPOBJ *obj, BOOL flip_h, BOOL flip_v)
//--------------------------------------------------------------
// �t���b�v�ݒ�
//--------------------------------------------------------------
// in:	obj    = �I�u�W�F�N�g
//		flip_h = TRUE: ���������t���b�v
//		flip_v = TRUE: ���������t���b�v
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	obj->flip_h = flip_h;
	obj->flip_v = flip_v;
}

//==============================================================
void GrpSetBlendMode(sGRPOBJ *obj, int mode)
//--------------------------------------------------------------
// �u�����f�B���O���[�h�ݒ�
//--------------------------------------------------------------
// in:	obj  = �I�u�W�F�N�g
//		mode = �u�����f�B���O���[�h(enmGRP_BLEND)
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	obj->blend_mode = mode;
}

//==============================================================
void GrpSetTexture(sGRPOBJ *obj, sTexture *tex)
//--------------------------------------------------------------
// �e�N�X�`����ݒ�
//--------------------------------------------------------------
// in:	obj = �I�u�W�F�N�g
//		tex = �e�N�X�`��(NULL = ����)
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	obj->tex = tex;
}

//==============================================================
void GrpSetSmooth(sGRPOBJ *obj, BOOL smooth)
//--------------------------------------------------------------
// �G�C���A�V���O���[�h��ύX
//--------------------------------------------------------------
// in:	obj    = �I�u�W�F�N�g
//		smooth = TRUE : �G�C���A�V���O ON
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	obj->smooth = smooth;
}

//==============================================================
BOOL GrpGetSmooth(sGRPOBJ *obj)
//--------------------------------------------------------------
// �G�C���A�V���O���[�h���擾
//--------------------------------------------------------------
// in:	obj = �I�u�W�F�N�g
//--------------------------------------------------------------
// out:	TRUE : �G�C���A�V���O ON
//==============================================================
{
	return obj->smooth;
}

//==============================================================
void GrpSetFilter(sGRPOBJ *obj, BOOL filter)
//--------------------------------------------------------------
// �t�B���^�����O���[�h��ύX
//--------------------------------------------------------------
// in:	obj    = �I�u�W�F�N�g
//		filter = TRUE: �e�N�X�`���t�B���^�����O ON
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	obj->filter = filter;
}

//==============================================================
BOOL GrpGetFilter(sGRPOBJ *obj, BOOL filter)
//--------------------------------------------------------------
// �t�B���^�����O���[�h���擾
//--------------------------------------------------------------
// in:	obj = �I�u�W�F�N�g
//--------------------------------------------------------------
// out:	TRUE: �e�N�X�`���t�B���^�����O ON
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
