//
//	�ėp�p�����[�^�Ǘ�
//

#include "co_param.h"
#include "co_debug.h"
#include "co_objlink.h"
#include "co_hash16.h"
#include "co_strings.h"
#include "co_memory.h"
#include "co_file.h"
#include "co_fileutil.h"
#include "co_misc.h"


#define PARAM_TEXTURE_CACHE							/* �e�N�X�`���L���b�V�� */

#define PARAM_STATIC_NUM   256					// �ÓI�f�[�^������


typedef struct {
	char id_str[ID_MAXLEN];						// ���ʎq
	int type;									// enmPARAM_TYPE
	union {
		REAL r_val;
		FVector2 v2_val;
		FVector3 v3_val;
		FVector4 v4_val;
		char *str_val;
		sTexture *tex;
		SndObj *snd;
	} uParamVal;
} sParamValue;

struct _sParam {
	sParamValue *val;							// �p�����[�^����
	int num;									// �p�����[�^��
	sHASH *hash;								// �n�b�V��
};


static sLink *param_link;


//==============================================================
static int paramCountLine(char *str)
//--------------------------------------------------------------
// �L���ȍs�𐔂���
// ���R�����g�̓X�L�b�v����
//--------------------------------------------------------------
// in:	str = �f�[�^�|�C���^
//--------------------------------------------------------------
// out:	�s��
//==============================================================
{
	int line = 0;

	while(str)
	{
		if(!StrIsComment(str) && !StrIsBlank(str))
		{
			++line;
		}
		str = StrNextLine(str);
	}

	return line;
}

//==============================================================
static sParamValue *paramGetValue(sParam *param, char *id_str)
//--------------------------------------------------------------
// �f�[�^���擾
//--------------------------------------------------------------
// in:	param  = �p�����[�^
//		id_str = ���ʎq
//--------------------------------------------------------------
// out:	�f�[�^�|�C���^(NULL = �f�[�^����)
//==============================================================
{
	return (sParamValue *)HashGet(param->hash, id_str);
}

//==============================================================
void ParamInit(void)
//--------------------------------------------------------------
// ������
//--------------------------------------------------------------
// in:	�Ȃ�
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	param_link = ObjLinkCreate(sizeof(struct _sParam), PARAM_STATIC_NUM, MEM_SYS, FALSE);

	SYSINFO(".... param initialize");
}

//==============================================================
void ParamFin(void)
//--------------------------------------------------------------
// �I��
//--------------------------------------------------------------
// in:	�Ȃ�
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	ParamDestroyAll();
	ObjLinkDestroy(param_link);

	SYSINFO(".... param finish");
}

//==============================================================
sParam *ParamRead(char *file)
//--------------------------------------------------------------
// �ǂݍ���
//--------------------------------------------------------------
// in:	file = �t�@�C����(�g���q�܂�)
//--------------------------------------------------------------
// out:	�A�N�Z�X�n���h��
//==============================================================
{
	sParam *param;
	char *str;
	char *s;
	int num;
	char path[FNAME_MAXLEN];
	char mem_id[FNAME_MAXLEN];

	param = (sParam *)ObjLinkNew(param_link);
	ASSERT(param);

	GetPathName(file, path, mem_id, TRUE);

	str = (char *)MmTextFileLoad(file);
	num = paramCountLine(str);					// �s���𐔂���
	param->val = (sParamValue *)appMalloc(sizeof(sParamValue) * num, mem_id);
	param->hash = HashCreate(mem_id);
	s = str;
	num = 0;
	while(s)
	{
		if(!StrIsComment(s) && !StrIsBlank(s))
		{
			char *st;
			int type;
			char key[ID_MAXLEN];
			char token[ID_MAXLEN];

			st = StrSkipSpace(s);
			StrGetTextId(key, st);

			st = StrNextToken(st);
			StrGetTextId(token, st);
			if(!strcmp(token, "TEX"))
			{
				char name[ID_MAXLEN];
				char fname[FNAME_MAXLEN];

				st = StrNextToken(st);
				type = PARAM_TEX;
				StrGetTextId(name, st);

				sTexture *tex;
#ifdef PARAM_TEXTURE_CACHE
				tex = TexGetFromName(name);
				if(tex == NULL)
#endif
				{
					sprintf(fname, "%s/%s.png", path, name);
					tex = TexRead(fname);
				}
				(param->val + num)->uParamVal.tex = tex;
			}
			else
			if(!strcmp(token, "SND"))
			{
				char name[ID_MAXLEN];
				char fname[FNAME_MAXLEN];

				st = StrNextToken(st);
				type = PARAM_SND;
				StrGetTextId(name, st);
				sprintf(fname, "%s/%s.wav", path, name);
				(param->val + num)->uParamVal.snd = SndReadWAV(fname);
			}
/*
			else
			if(!strcmp(token, "ANM"))
			{
				char name[ID_MAXLEN];
				char fname[FNAME_MAXLEN];

				st = StrNextToken(st);
				type = PARAM_ANM;
				StrGetTextId(name, st);
				sprintf(fname, "%s/%s.anm", path, name);
				(param->val + num)->uParamVal.anim.anm = AnimCreate(fname);
				STRCPY16((param->val + num)->uParamVal.anim.name, name);
			}
*/
/*
			else
			if(!strcmp(token, "MML"))
			{
				char name[ID_MAXLEN];
				char fname[FNAME_MAXLEN];

				st = StrNextToken(st);
				type = PARAM_MML;
				StrGetTextId(name, st);
				sprintf(fname, "%s/%s.mml", path, name);
				(param->val + num)->uParamVal.mml.ptr = MmlRead(fname);
				STRCPY16((param->val + num)->uParamVal.mml.name, name);
			}
*/
			else
			if(*st == '"')
			{
				char *dst;
				int len;
				int i;

				type = PARAM_STR;

				++st;
				len = 0;
				while((st[len] != '"') && (st[len] >= ' '))
				{
					++len;
				}
				dst = (char *)appMalloc(len + 1, "str_val");
				(param->val + num)->uParamVal.str_val = dst;
				for(i = 0; i < len; ++i)
				{
					dst[i] = st[i];
				}
				dst[i] = '\0';
			}
			else
			{
				FVector4 val;

				type = PARAM_REAL;
				val.x = StrGetReal(st);
				st = StrNextToken(st);
				if(!StrIsComment(st) && !StrIsTextEnd(st))
				{
					type = PARAM_V2;
					val.y = StrGetReal(st);
					st = StrNextToken(st);
					if(!StrIsComment(st) && !StrIsTextEnd(st))
					{
						type = PARAM_V3;
						val.z = StrGetReal(st);
						st = StrNextToken(st);
						if(!StrIsComment(st) && !StrIsTextEnd(st))
						{
							type = PARAM_V4;
							val.w = StrGetReal(st);
						}
					}
				}

				if(type == PARAM_REAL)	(param->val + num)->uParamVal.r_val = val.x;
				else
				if(type == PARAM_V2)	SetV2d(&(param->val + num)->uParamVal.v2_val, val.x, val.y);
				else
				if(type == PARAM_V3)	SetV3d(&(param->val + num)->uParamVal.v3_val, val.x, val.y, val.z);
				else					SetV4d(&(param->val + num)->uParamVal.v4_val, val.x, val.y, val.z, val.w);
			}

			(param->val + num)->type = type;
			STRCPY16((param->val + num)->id_str, key);
			HashAdd(param->hash, key, (void *)(param->val + num));

			++num;
		}
		s = StrNextLine(s);
	}
	Free(str);
	param->num = num;							// ���ۂ̃f�[�^��

	return param;
}

//==============================================================
void ParamWrite(char *file, sParam *param)
//--------------------------------------------------------------
// �����o��
//--------------------------------------------------------------
// in:	file  = �t�@�C����(�g���q�܂�)
//		param = �A�N�Z�X�n���h��
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	char *str;
	char *s;
	sFILE *fd;
	int i;
	sParamValue *val;

	str = (char *)fsMalloc(sizeof(char) * 1024 * 256, "objwrite");

	val = param->val;
	s = str;
	for(i = 0; i < param->num; ++i)
	{
		int len;
		int tab;

		// ���ʎq��
		len = sprintf(s, "%s", val->id_str);
		s += len;
		tab = 6 - (len + 4) / 4;
		for( ; tab > 0; --tab)
		{
			*s = '\t';
			s += 1;
		}

		// �f�[�^
		switch(val->type)
		{
		case PARAM_REAL:
			{
				s += sprintf(s, "%f\n", val->uParamVal.r_val);
			}
			break;

		case PARAM_V2:
			{
				s += sprintf(s, "%f  %f\n", val->uParamVal.v2_val.x, val->uParamVal.v2_val.y);
			}
			break;

		case PARAM_V3:
			{
				s += sprintf(s, "%f  %f  %f\n", val->uParamVal.v3_val.x, val->uParamVal.v3_val.y, val->uParamVal.v3_val.z);
			}
			break;

		case PARAM_V4:
			{
				s += sprintf(s, "%f  %f  %f  %f\n", val->uParamVal.v4_val.x, val->uParamVal.v4_val.y, val->uParamVal.v4_val.z, val->uParamVal.v4_val.w);
			}
			break;

		case PARAM_STR:
			{
				s += sprintf(s, "\"%s\"\n", val->uParamVal.str_val);
			}
			break;

		case PARAM_TEX:
			{
				s += sprintf(s, "TEX %s\n", TexGetName(val->uParamVal.tex));
			}
			break;

		case PARAM_SND:
			{
				s += sprintf(s, "SND %s\n", SndObjGetName(val->uParamVal.snd));
			}
			break;

			/*
			case PARAM_ANM:
			{
				s += sprintf(s, "ANM %s\n", val->uParamVal.anim.name);
			}
			break;
			*/
		}

		++val;
	}
	s += sprintf(s, "\n");

	fd = FsCreate(file);
	FsWrite(fd, str, s - str);
	FsClose(fd);
	Free(str);
}

//==============================================================
void ParamDestroy(sParam *param)
//--------------------------------------------------------------
// �f�[�^�j��
// �����I�ɐ������ꂽ�I�u�W�F�N�g���ꏏ�ɔj��
//--------------------------------------------------------------
// in:	param = �A�N�Z�X�n���h��
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	int i;

	for(i = 0; i < param->num; ++i)
	{
		if((param->val + i)->type == PARAM_TEX)
		{
#ifndef PARAM_TEXTURE_CACHE
			TexDestroy((param->val + i)->uParamVal.tex);
#endif
		}
		else
		if((param->val + i)->type == PARAM_SND)
		{
			SndDestroy((param->val + i)->uParamVal.snd);
		}
/*
		else
		if((param->val + i)->type == PARAM_ANM)
		{
			// �A�j���[�V�����j��
			AnimDestroy((param->val + i)->uParamVal.anim.anm);
		}
*/
/*
		else
		if((param->val + i)->type == PARAM_MML)
		{
			// MML�f�[�^�j��
			MmlDestroy((param->val + i)->uParamVal.mml.ptr);
		}
*/
		else
		if((param->val + i)->type == PARAM_STR)
		{
			// ������
			Free((param->val + i)->uParamVal.str_val);
		}
	}
	HashKill(param->hash);
	Free(param->val);
	ObjLinkDel(param_link, param);
}

//==============================================================
void ParamDestroyAll(void)
//--------------------------------------------------------------
// �S�f�[�^�j��
// �����I�ɐ������ꂽ�I�u�W�F�N�g���ꏏ�ɔj��
//--------------------------------------------------------------
// in:	�Ȃ�
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	sParam *param;

	param = (sParam *)ObjLinkGetTop(param_link);
	while(param)
	{
		sParam *next;

		next = (sParam *)ObjLinkGetNext(param);
		ParamDestroy(param);
		param = next;
	}
}

//==============================================================
BOOL ParamIsExists(sParam *param, char *id_str)
//--------------------------------------------------------------
// �p�����[�^�����݂��邩�`�F�b�N
//--------------------------------------------------------------
// in:	param  = �p�����[�^
//		id_str = ���ʎq
//--------------------------------------------------------------
// out:	TRUE = ���݂���
//==============================================================
{
	return (paramGetValue(param, id_str) == NULL) ? FALSE : TRUE;
}

//==============================================================
int ParamGetType(sParam *param, char *id_str)
//--------------------------------------------------------------
// �p�����[�^�̌^���擾
//--------------------------------------------------------------
// in:	param  = �p�����[�^
//		id_str = ���ʎq
//--------------------------------------------------------------
// out:	enmPARAM_TYPE
//==============================================================
{
	int type = PARAM_NONE;
	sParamValue *val;

	val = paramGetValue(param, id_str);
	if(val)		type = val->type;

	return type;
}

//==============================================================
char *ParamGetStr(sParam *param, char *id_str)
//--------------------------------------------------------------
// ������p�����[�^�擾
//--------------------------------------------------------------
// in:	param  = �p�����[�^
//		id_str = ���ʎq
//--------------------------------------------------------------
// out:	�p�����[�^
//==============================================================
{
	char *str = NULL;
	sParamValue *val;

	val = paramGetValue(param, id_str);
	if(val)
	{
		if(val->type == PARAM_STR)		str = val->uParamVal.str_val;
	}
	return str;
}

//==============================================================
REAL ParamGetReal(sParam *param, char *id_str)
//--------------------------------------------------------------
// REAL�^�p�����[�^�擾
//--------------------------------------------------------------
// in:	param  = �p�����[�^
//		id_str = ���ʎq
//--------------------------------------------------------------
// out:	�p�����[�^
//==============================================================
{
	REAL r = 0;
	sParamValue *val;

	val = paramGetValue(param, id_str);
	if(val)
	{
		if(val->type == PARAM_REAL)		r = val->uParamVal.r_val;
	}
	return r;
}

//==============================================================
FVector2 *ParamGetFVec2(sParam *param, char *id_str)
//--------------------------------------------------------------
// FVector2�^�p�����[�^�擾
//--------------------------------------------------------------
// in:	param  = �p�����[�^
//		id_str = ���ʎq
//--------------------------------------------------------------
// out:	�p�����[�^
//==============================================================
{
	FVector2 *vec = NULL;
	sParamValue *val;

	val = paramGetValue(param, id_str);
	if(val)
	{
		if(val->type == PARAM_V2)		vec = &val->uParamVal.v2_val;
	}
	return vec;
}

//==============================================================
FVector3 *ParamGetFVec3(sParam *param, char *id_str)
//--------------------------------------------------------------
// FVector3�^�p�����[�^�擾
//--------------------------------------------------------------
// in:	param  = �p�����[�^
//		id_str = ���ʎq
//--------------------------------------------------------------
// out:	�p�����[�^
//==============================================================
{
	FVector3 *vec = NULL;
	sParamValue *val;

	val = paramGetValue(param, id_str);
	if(val)
	{
		if(val->type == PARAM_V3)		vec = &val->uParamVal.v3_val;
	}
	return vec;
}

//==============================================================
FVector4 *ParamGetFVec4(sParam *param, char *id_str)
//--------------------------------------------------------------
// FVector4�^�p�����[�^�擾
//--------------------------------------------------------------
// in:	param  = �p�����[�^
//		id_str = ���ʎq
//--------------------------------------------------------------
// out:	�p�����[�^
//==============================================================
{
	FVector4 *vec = NULL;
	sParamValue *val;

	val = paramGetValue(param, id_str);
	if(val)
	{
		if(val->type == PARAM_V4)		vec = &val->uParamVal.v4_val;
	}
	return vec;
}

//==============================================================
sTexture *ParamGetTex(sParam *param, char *id_str)
//--------------------------------------------------------------
// �e�N�X�`���p�����[�^�擾
//--------------------------------------------------------------
// in:	param  = �p�����[�^
//		id_str = ���ʎq
//--------------------------------------------------------------
// out:	�p�����[�^
//==============================================================
{
	sTexture *tex = NULL;
	sParamValue *val;

	val = paramGetValue(param, id_str);
	if(val)
	{
		if(val->type == PARAM_TEX)		tex = val->uParamVal.tex;
	}
	return tex;
}

SndObj *ParamGetSnd(sParam *param, char *id_str)
{
	SndObj *obj = NULL;
	sParamValue *val;

	val = paramGetValue(param, id_str);
	if(val)
	{
		if(val->type == PARAM_SND)		obj = val->uParamVal.snd;
	}
	return obj;
}

#if 0

//==============================================================
sAnim *ParamGetAnm(sParam *param, char *id_str)
//--------------------------------------------------------------
// �A�j���[�V�����p�����[�^�擾
//--------------------------------------------------------------
// in:	param  = �p�����[�^
//		id_str = ���ʎq
//--------------------------------------------------------------
// out:	�p�����[�^
//==============================================================
{
	sAnim *anm = NULL;
	sParamValue *val;

	val = paramGetValue(param, id_str);
	if(val)
	{
		if(val->type == PARAM_ANM)		anm = val->uParamVal.anim.anm;
	}
	return anm;
}

//==============================================================
u_char *ParamGetMml(sParam *param, char *id_str)
//--------------------------------------------------------------
// MML�f�[�^�擾
//--------------------------------------------------------------
// in:	param  = �p�����[�^
//		id_str = ���ʎq
//--------------------------------------------------------------
// out:	MML�f�[�^
//==============================================================
{
	u_char *ptr = NULL;
	sParamValue *val;

	val = paramGetValue(param, id_str);
	if(val)
	{
		if(val->type == PARAM_MML)		ptr = val->uParamVal.mml.ptr;
	}
	return ptr;
}

#endif

//==============================================================
BOOL ParamSetStr(sParam *param, char *id_str, char *str)
//--------------------------------------------------------------
// ������p�����[�^�ύX
//--------------------------------------------------------------
// in:	param  = �p�����[�^
//		id_str = ���ʎq
//		str    = ������
//--------------------------------------------------------------
// out:	TRUE = ����
//==============================================================
{
	BOOL res = FALSE;
	sParamValue *val;

	val = paramGetValue(param, id_str);
	if(val)
	{
		if(val->type == PARAM_STR)
		{
			Free(val->uParamVal.str_val);
			val->uParamVal.str_val = (char *)appMalloc(strlen(str) + 1, "str_val");
			strcpy(val->uParamVal.str_val, str);
			res = TRUE;
		}
	}
	return res;
}

//==============================================================
BOOL ParamSetReal(sParam *param, char *id_str, REAL r)
//--------------------------------------------------------------
// REAL�^�p�����[�^�ύX
//--------------------------------------------------------------
// in:	param  = �p�����[�^
//		id_str = ���ʎq
//		r      = �l
//--------------------------------------------------------------
// out:	TRUE = ����
//==============================================================
{
	BOOL res = FALSE;
	sParamValue *val;

	val = paramGetValue(param, id_str);
	if(val)
	{
		if(val->type == PARAM_REAL)
		{
			val->uParamVal.r_val = r;
			res = TRUE;
		}
	}
	return res;
}

//==============================================================
BOOL ParamSetFVec2(sParam *param, char *id_str, FVector2 *vec)
//--------------------------------------------------------------
// FVector2�^�p�����[�^�ύX
//--------------------------------------------------------------
// in:	param  = �p�����[�^
//		id_str = ���ʎq
//		vec    = �l
//--------------------------------------------------------------
// out:	TRUE = ����
//==============================================================
{
	BOOL res = FALSE;
	sParamValue *val;

	val = paramGetValue(param, id_str);
	if(val)
	{
		if(val->type == PARAM_V2)
		{
			val->uParamVal.v2_val = *vec;
			res = TRUE;
		}
	}
	return res;
}

//==============================================================
BOOL ParamSetFVec3(sParam *param, char *id_str, FVector3 *vec)
//--------------------------------------------------------------
// FVector3�^�p�����[�^�ύX
//--------------------------------------------------------------
// in:	param  = �p�����[�^
//		id_str = ���ʎq
//		vec    = �l
//--------------------------------------------------------------
// out:	TRUE = ����
//==============================================================
{
	BOOL res = FALSE;
	sParamValue *val;

	val = paramGetValue(param, id_str);
	if(val)
	{
		if(val->type == PARAM_V3)
		{
			val->uParamVal.v3_val = *vec;
			res = TRUE;
		}
	}
	return res;
}

//==============================================================
BOOL ParamSetFVec4(sParam *param, char *id_str, FVector4 *vec)
//--------------------------------------------------------------
// FVector4�^�p�����[�^�ύX
//--------------------------------------------------------------
// in:	param  = �p�����[�^
//		id_str = ���ʎq
//		vec    = �l
//--------------------------------------------------------------
// out:	TRUE = ����
//==============================================================
{
	BOOL res = FALSE;
	sParamValue *val;

	val = paramGetValue(param, id_str);
	if(val)
	{
		if(val->type == PARAM_V4)
		{
			val->uParamVal.v4_val = *vec;
			res = TRUE;
		}
	}
	return res;
}

