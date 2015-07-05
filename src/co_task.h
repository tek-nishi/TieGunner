
/*

	�V�X�e�����ŕK�����s����郁�b�Z�[�W

	MSG_CREATE				����
	MSG_KILL				�폜

	MSG_PREPROC				���O����
	MSG_STEP				�X�e�b�v
	MSG_UPDATE				�X�V
	MSG_DRAW				�`��


	�^�X�N�𐶐�����ꍇ�A������ MSG_CREATE �𔭍s���܂����ATASK_CREATE_REQ
	���w�肳��Ă���ꍇ�AMSG_PREPROC ���O�� MSG_CREATE �𔭍s���܂��B
	�X���[�v�֘A������ MSG_PREPROC ���O�ɏ�������܂��B
	TASK_DELETE_REQ �̏����� MSG_STEP ����ɍs���܂��B

*/

//==============================================================
#ifndef CO_TASK_H
#define CO_TASK_H
//==============================================================

#include "co_common.h"

#ifdef __cplusplus
extern              "C"
{
#endif

/********************************************/
/*             �萔�E�}�N���錾             */
/********************************************/
#define TASK_DELETE_REQ  (1 << 0)				// �폜�\��
#define TASK_SLEEP_REQ   (1 << 1)				// �X���[�v�\��
#define TASK_SLEEP       (1 << 2)				// �X���[�v��
#define TASK_CREATE_REQ  (1 << 3)				// ���̃X�e�b�v�� MSG_CREATE �𓊂���
#define TASK_STEP_REQ    (1 << 4)				// ����X�e�b�v�\��
#define TASK_NOPAUSE     (1 << 5)				// �|�[�Y���Ȃ�

enum enmTASK_PRIO {
	TASK_PRI_NONE = -1,							// �����p�v���C�I���e�B�錾

	TASK_PRI_SYS = 1,							// �ŏ��Ɏ��s�����^�X�N

	TASK_PRI_01,								// �A�v���P�[�V�����ŗ��p
	TASK_PRI_02,								// �A�v���P�[�V�����̎q�^�X�N�����p
	TASK_PRI_03,
	TASK_PRI_04,
	TASK_PRI_05,								// �G�f�B�^�����p

	TASK_PRI_99,								// �Ō�Ɏ��s�����^�X�N

	//-------------
	TASK_PRI_NUM,
};

#ifdef DEBUG
	#define TASK_INFO(body, info)  PRINTF("---- %s %s ----\n", TaskGetId(body), info)
#else
	#define TASK_INFO(body, info)  __noop
#endif


/********************************************/
/*                �\���̐錾                */
/********************************************/
typedef struct _sTaskBody sTaskBody;

// �R�[���o�b�N�֐�
typedef int (*TASK_FUNC)(sTaskBody *body, int msg, int lParam, int rParam);


/********************************************/
/*              �O���[�o���ϐ�              */
/********************************************/


/********************************************/
/*              �O���[�o���֐�              */
/********************************************/
// ������
extern void TaskInit(int num);
// �I��
extern void TaskFin(void);

// �^�X�N�X�V����
extern void TaskUpdate(int msg, BOOL pause);

// �^�X�N�쐬
extern sTaskBody *TaskCreate(char *name, int prio, TASK_FUNC func, int lParam, int rParam);
// �^�X�N�쐬(MSG_CREATE �� TaskUpdate() ���ő��M)
extern sTaskBody *TaskEntry(char *name, int prio, TASK_FUNC func, int lParam, int rParam);
// �^�X�N�擾
extern sTaskBody *TaskGetBody(sTaskBody *body, int prio);
// �^�X�N�擾(���O�Œ��ׂ�)
extern sTaskBody *TaskGetBodyFromName(sTaskBody *body, char *id_str);
// �^�X�N���𒲂ׂ�
extern int TaskGetNum(int prio);

// �ėp�f�[�^�|�C���^�̎擾
extern void *TaskGetVar(sTaskBody *body, int size, int area);

// ���b�Z�[�W���M
extern int TaskPostMsg(sTaskBody *body, int msg, int lParam, int rParam);
// �S�^�X�N�փ��b�Z�[�W���M
extern void TaskPostMsgAll(int prio, int msg, int lParam, int rParam);
// �q�^�X�N�փ��b�Z�[�W���M
extern void TaskPostMsgChild(sTaskBody *parent, int msg, int lParam, int rParam);

// �^�X�N�폜(�\��)
extern void TaskDeleteReq(sTaskBody *body);
// �^�X�N�폜�\��
extern void TaskDeleteAllReq(int prio);
// �S�^�X�N�폜(�������s)
extern void TaskDeleteAll(int prio);
// �A�v���P�[�V�����^�X�N�폜(�������s)
extern void TaskDeleteAppAll(void);

// �^�X�N�̃X���[�v�\��
extern void TaskSleepReq(sTaskBody *body, int time);
// �^�X�N�̃X���[�v(�������s)
extern void TaskSleep(sTaskBody *body, int time);
// �^�X�N�̃X���[�v����
extern void TaskAwakeReq(sTaskBody *body, int time);
// �^�X�N�̃X���[�v����(�������s)
extern void TaskAwake(sTaskBody *body);
// �^�X�N���X���[�v��������
extern BOOL TaskIsSleep(sTaskBody *body);

// �^�X�N���|�[�Y�ɉe������邩�ݒ�
extern void TaskSetNoPause(sTaskBody *body, BOOL nopause);
// �^�X�N���|�[�Y�ɉe������邩����
extern BOOL TaskIsNoPause(sTaskBody *body);

// ���ʎq�擾
extern char *TaskGetId(sTaskBody *body);

// �e�^�X�N�ݒ�
extern void TaskSetParent(sTaskBody *body, sTaskBody *parent);
// �e�^�X�N�擾
extern sTaskBody *TaskGetParent(sTaskBody *body);
// �q�^�X�N���
extern sTaskBody *TaskEnumChild(sTaskBody *parent, sTaskBody *prev);


#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================
