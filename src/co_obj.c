//
// �ėp�I�u�W�F�N�g����
//


#include "co_obj.h"
#include "co_objlink.h"
#include "co_hash16.h"
#include "co_memory.h"
#include "co_task.h"
#include "co_strings.h"
#include "co_misc.h"


#define OBJ_PARAM_NUM  64							// �p�����[�^�Ǘ���
#define OBJ_NUM        1024							// �f�[�^������
#define OBJ_FILE_PATH  PATH_DATA"/object"			// �f�[�^�ǂݍ��݃p�X


struct _sOBJ {
	u_int type;									// �t�B���^�����O�p�^�C�v
	char id_str[ID_MAXLEN];						// ���ʎq
	sParam *param;								// �p�����[�^
	OBJ_PROC func;								// �R�[���o�b�N
	BOOL create;								// TRUE = �����t���[��
	BOOL kill_req;								// TRUE = �폜�\��
	void *var;									// �ėp���[�N�G���A

	FVector2 pos;								// �ʒu
	REAL dir;									// ����
	FVector2 vct;
	REAL radius;
	BOOL death;
};


static sHASH *param_hash;							// �p�����[�^�Ǘ��p
static sLink *obj_link;								// �f�[�^�̈�


//==============================================================
static sParam *objGetParam(char *id_str)
//--------------------------------------------------------------
// �p�����[�^���擾
//--------------------------------------------------------------
// in:	id_str = ���ʎq
//--------------------------------------------------------------
// out:	�f�[�^�|�C���^(NULL = �f�[�^����)
//==============================================================
{
	return (sParam *)HashGet(param_hash, id_str);
}

//==============================================================
static void objDelete(sOBJ *obj)
//--------------------------------------------------------------
// �I�u�W�F�N�g���폜
//--------------------------------------------------------------
// in:	obj = �n���h��
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	obj->kill_req = FALSE;
	ObjPostMsg(obj, MSG_KILL, 0, 0);

	FreeWork(obj->var);
	ObjLinkDel(obj_link, obj);
}

//==============================================================
static void objDeleteAll(u_int type, char *id_str)
//--------------------------------------------------------------
// �I�u�W�F�N�g��S�č폜
//--------------------------------------------------------------
// in:	type   = �t�B���^�����O
//		id_str = ���ʖ�
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	sOBJ *obj;

	obj = ObjGetNext(NULL, type, id_str);
	while(obj)
	{
		sOBJ *next;

		next = ObjGetNext(obj, type, id_str);
		objDelete(obj);
		obj = next;
	}
}

//==============================================================
static int objTask(sTaskBody *body, int msg, int lParam, int rParam)
//--------------------------------------------------------------
// �^�X�N����
//--------------------------------------------------------------
// in:	body           = �n���h��
//		msg            = ���b�Z�[�W
//		lParam, rParam = �ėp�p�����[�^
//--------------------------------------------------------------
// out:	���s����
//==============================================================
{
	int res = 0;

	switch(msg)
	{
		case MSG_CREATE:
		{
			param_hash = HashCreate("obj");
			obj_link = ObjLinkCreate(sizeof(sOBJ), OBJ_NUM, MEM_SYS, FALSE);

			SYSINFO(".... object initialize");
		}
		break;

		case MSG_KILL:
		{
			ObjDeleteParamAll();

			HashKill(param_hash);
			ObjLinkDestroy(obj_link);

			SYSINFO(".... object finish");
		}
		break;

		case MSG_PREPROC:
		{
			{
				sOBJ *obj;

				obj = ObjGetNext(NULL, OBJ_TYPE_ALL, NULL);
				while(obj)
				{
					obj->create = FALSE;
					obj = ObjGetNext(obj, OBJ_TYPE_ALL, NULL);
				}
			}
			ObjPostMsgAll(OBJ_TYPE_ALL, msg, FALSE, 0, 0);
		}
		break;

		case MSG_STEP:
		{
			ObjPostMsgAll(OBJ_TYPE_ALL, msg, FALSE, 0, 0);
			{
				sOBJ *obj;

				obj = ObjGetNext(NULL, OBJ_TYPE_ALL, NULL);
				while(obj)
				{
					sOBJ *next;

					next = ObjGetNext(obj, OBJ_TYPE_ALL, NULL);
					if(obj->kill_req)	objDelete(obj);
					obj = next;
				}
			}
		}
		break;

		case MSG_UPDATE:
		{
			ObjPostMsgAll(OBJ_TYPE_ALL, msg, FALSE, 0, 0);
		}
		break;

		case MSG_DRAW:
		{
			ObjPostMsgAll(OBJ_TYPE_ALL, msg, FALSE, 0, 0);
		}
		break;

		default:
		{
			ObjPostMsgAll(OBJ_TYPE_ALL, msg, FALSE, lParam, rParam);
		}
		break;
	}

	return res;
}

//==============================================================
void ObjInit(void)
//--------------------------------------------------------------
// ������
//--------------------------------------------------------------
// in:	�Ȃ�
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	TaskCreate("sys_obj", TASK_PRI_SYS, objTask, 0, 0);
}

//==============================================================
sParam *ObjSetup(char *id_str)
//--------------------------------------------------------------
// �I�u�W�F�N�g�̃Z�b�g�A�b�v
// ���������O�����݂���ꍇ�A�ǂݍ��ݒ���
//--------------------------------------------------------------
// in:	id_str = ���ʎq
//--------------------------------------------------------------
// out:	�ǂݍ��񂾃p�����[�^
//==============================================================
{
	char file[FNAME_MAXLEN];
	sParam *param;

	param = (sParam *)HashGet(param_hash, id_str);
	if(param)	ParamDestroy(param);

	sprintf(file, OBJ_FILE_PATH"/%s.param", id_str);
	param = ParamRead(file);
	HashAdd(param_hash, id_str, param);

	return param;
}

//==============================================================
sParam *ObjGetSetupParam(char *id_str)
//--------------------------------------------------------------
// �Z�b�g�A�b�v�ς݂̃p�����[�^���擾
//--------------------------------------------------------------
// in:	id_str = �n���h��
//--------------------------------------------------------------
// out:	�p�����[�^
//==============================================================
{
	return objGetParam(id_str);
}

//==============================================================
void ObjDeleteParam(char *id_str)
//--------------------------------------------------------------
// �Z�b�g�A�b�v�����f�[�^�̔j��
//--------------------------------------------------------------
// in:	id_str = ���ʎq
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	sParam *param;

	param = objGetParam(id_str);
	ASSERT(param);

	// ��ɃI�u�W�F�N�g��j��
	objDeleteAll(OBJ_TYPE_ALL, id_str);

	ParamDestroy(param);
	HashDel(param_hash, id_str);
}

//==============================================================
void ObjDeleteParamAll(void)
//--------------------------------------------------------------
// �Z�b�g�A�b�v�����S�f�[�^�̔j��
//--------------------------------------------------------------
// in:	�Ȃ�
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	sHASH_KEY **list;
	int ofs;

	// ��ɃI�u�W�F�N�g��j��
	objDeleteAll(OBJ_TYPE_ALL, NULL);

	// �S�p�����[�^�폜
	list = HashGetKeyList(param_hash);
	ofs = 0;
	while(list[ofs] != NULL)
	{
		ObjDeleteParam(HashGetKeyId(list[ofs]));
		ofs += 1;
	}
	Free(list);

	HashCleanup(param_hash);
}

void ObjDeleteAll(u_int type)
{
	objDeleteAll(type, NULL);
}

//==============================================================
sOBJ *ObjGetNext(sOBJ *prev, u_int type, char *id_str)
//--------------------------------------------------------------
// �I�u�W�F�N�g�擾
//--------------------------------------------------------------
// in:	prev   = �n���h��(NULL = �擪����)
//		type   = �t�B���^�����O
//		id_str = �t�B���^�����O��
//--------------------------------------------------------------
// out:	���̃n���h��(NULL = �f�[�^����)
//==============================================================
{
	sOBJ *obj;
	BOOL res;

	obj = prev;
	do
	{
		obj = obj ? (sOBJ *)ObjLinkGetNext(obj) : (sOBJ *)ObjLinkGetTop(obj_link);
		res = obj ? obj->type & type : FALSE;
		if(res && id_str)
		{
			res = !strcmp(id_str, obj->id_str) ? TRUE : FALSE;
		}
	}
	while(obj && !res);

	return obj;
}

//==============================================================
int ObjPostMsg(sOBJ *obj, int msg, int lParam, int rParam)
//--------------------------------------------------------------
// ���b�Z�[�W���M
//--------------------------------------------------------------
// in:	obj    = �n���h��
//		msg    = ���b�Z�[�W
//		lParam = �p�����[�^�P
//		rParam = �p�����[�^�Q
//--------------------------------------------------------------
// out:	���s����
//==============================================================
{
	int res = 0;

	if(!obj->kill_req)
	{
		res = obj->func(obj, obj->param, msg, lParam, rParam);
	}

	return res;
}

//==============================================================
BOOL ObjPostMsgAll(u_int type, int msg, BOOL abort, int lParam, int rParam)
//--------------------------------------------------------------
// �S�I�u�W�F�N�g�փ��b�Z�[�W���M
//--------------------------------------------------------------
// in:	type   = �t�B���^�����O
//		msg    = ���b�Z�[�W
//		abort  = TRUE: 0 �ȊO���Ԃ��ꂽ��A�r���Œ��f
//		lParam = �p�����[�^�P
//		rParam = �p�����[�^�Q
//--------------------------------------------------------------
// out:	TRUE = ���f���ꂽ
//==============================================================
{
	BOOL res = FALSE;
	sOBJ *obj;

	obj = ObjGetNext(NULL, type, NULL);
	while(obj)
	{
		if(!obj->create)
		{
			int result;

			result = ObjPostMsg(obj, msg, lParam, rParam);
			if(abort && result)
			{
				res = TRUE;
				break;
			}
		}
		obj = ObjGetNext(obj, type, NULL);
	}

	return res;
}

//==============================================================
sOBJ *ObjCreate(char *id_str, u_int type, OBJ_PROC func, int lParam, int rParam)
//--------------------------------------------------------------
// �I�u�W�F�N�g����
//--------------------------------------------------------------
// in:	id_str = �I�u�W�F�N�g���ʎq
//		type   = �t�B���^�����O
//		func   = �R�[���o�b�N
//		lParam = �p�����[�^�P
//		rParam = �p�����[�^�Q
//--------------------------------------------------------------
// out:	�n���h��
//==============================================================
{
	sOBJ *obj;

	obj = (sOBJ *)ObjLinkNew(obj_link);
	ASSERT(obj);
	obj->param = objGetParam(id_str);
	ASSERT(obj->param);
	obj->type = type;
	STRCPY16(obj->id_str, id_str);
	obj->func = func;

	ObjPostMsg(obj, MSG_CREATE, lParam, rParam);
	obj->create = TRUE;							// �����������珈�����J�n�ł���悤�ɂ���

	return obj;
}

//==============================================================
void ObjKillReq(sOBJ *obj)
//--------------------------------------------------------------
// �I�u�W�F�N�g�폜�v��
//--------------------------------------------------------------
// in:	obj = �n���h��
//--------------------------------------------------------------
// out:	�n���h��
//==============================================================
{
	obj->kill_req = TRUE;
}

//==============================================================
void *ObjGetVar(sOBJ *obj, int size)
//--------------------------------------------------------------
// ���[�N�擾
//--------------------------------------------------------------
// in:	obj  = �n���h��
//		size = �f�[�^�T�C�Y
//--------------------------------------------------------------
// out:	�擾�����|�C���^
//==============================================================
{
	if(obj->var == NULL)
	{
		obj->var = GetWork(size, "obj_var");
	}
	return obj->var;
}

//==============================================================
sParam *ObjGetParam(sOBJ *obj)
//--------------------------------------------------------------
// �p�����[�^�擾
//--------------------------------------------------------------
// in:	obj = �n���h��
//--------------------------------------------------------------
// out:	�p�����[�^
//==============================================================
{
	return obj->param;
}

//==============================================================
u_int ObjGetType(sOBJ *obj)
//--------------------------------------------------------------
// �^�C�v�擾
//--------------------------------------------------------------
// in:	obj = �n���h��
//--------------------------------------------------------------
// out:	�^�C�v
//==============================================================
{
	return obj->type;
}

//==============================================================
void ObjSetType(sOBJ *obj, u_int type)
//--------------------------------------------------------------
// �^�C�v�ύX
//--------------------------------------------------------------
// in:	obj  = �n���h��
//		type = �^�C�v
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	obj->type = type;
}

//==============================================================
FVector2 *ObjGetPos(sOBJ *obj)
//--------------------------------------------------------------
// �ʒu���擾
//--------------------------------------------------------------
// in:	obj = �n���h��
//--------------------------------------------------------------
// out:	�ʒu
//==============================================================
{
	return &obj->pos;
}

//==============================================================
void ObjSetPos(sOBJ *obj, REAL x, REAL y)
//--------------------------------------------------------------
// �ʒu��ύX
//--------------------------------------------------------------
// in:	obj  = �n���h��
//		x, y = �ʒu
//--------------------------------------------------------------
// out:	�Ȃ�
//==============================================================
{
	SetV2d(&obj->pos, x, y);
}

REAL ObjGetDir(sOBJ *obj)
{
	return obj->dir;
}

void ObjSetDir(sOBJ *obj, REAL dir)
{
	obj->dir = dir;
}

FVector2 *ObjGetVct(sOBJ *obj)
{
	return &obj->vct;
}

void ObjSetVct(sOBJ *obj, REAL x, REAL y)
{
	SetV2d(&obj->vct, x, y);
}

REAL ObjGetRadius(sOBJ *obj)
{
	return obj->radius;
}

void ObjSetRadius(sOBJ *obj, REAL radius)
{
	obj->radius = radius;
}

BOOL ObjIsDead(sOBJ *obj)
{
	return obj->death;
}

void ObjSetDeath(sOBJ *obj, BOOL death)
{
	obj->death = death;
}
