
/*

	GLUT�Œ񋟂���Ă�����͂���A�Q�[���p�̓��͏��𐶐����܂��B
	InputGetKey() �ŕԂ����ASCII�R�[�h�ł����ATAB �� ESC �Ƃ���
	ASCII�R�[�h�ŕԂ��܂��B
	
*/

//==============================================================
#ifndef CO_INPUT_H
#define CO_INPUT_H
//==============================================================

#include "co_common.h"
#include "co_file.h"

#ifdef __cplusplus
extern              "C"
{
#endif


enum {
	MOUSE_LEFT	 = (1 << 0),						// �}�E�X���{�^��
	MOUSE_MIDDLE = (1 << 1),						// �}�E�X�����{�^��
	MOUSE_RIGHT	 = (1 << 2),						// �}�E�X�E�{�^��
	MOUSE_D_LEFT = (1 << 3),						// �}�E�X���{�^���_�u���N���b�N
};

enum {
	ASCII_ENTER = 13,								// ���^�[���L�[
	ASCII_BS	= 8,								// BS�L�[
	ASCII_DEL	= 127,								// DEL�L�[
	ASCII_ESC	= 27,								// ESC�L�[
};

enum {
	INP_SYS,
	INP_CH0,
	INP_CH1,

	INPUT_WORK_NUM
};

enum {
	INP_KEY_F1		  = GLUT_KEY_F1 + 0x80,
	INP_KEY_F2		  = GLUT_KEY_F2 + 0x80,
	INP_KEY_F3		  = GLUT_KEY_F3 + 0x80,
	INP_KEY_F4		  = GLUT_KEY_F4 + 0x80,
	INP_KEY_F5		  = GLUT_KEY_F5 + 0x80,
	INP_KEY_F6		  = GLUT_KEY_F6 + 0x80,
	INP_KEY_F7		  = GLUT_KEY_F7 + 0x80,
	INP_KEY_F8		  = GLUT_KEY_F8 + 0x80,
	INP_KEY_F9		  = GLUT_KEY_F9 + 0x80,
	INP_KEY_F10		  = GLUT_KEY_F10 + 0x80,
	INP_KEY_F11		  = GLUT_KEY_F11 + 0x80,
	INP_KEY_F12		  = GLUT_KEY_F12 + 0x80,
	INP_KEY_UP		  = GLUT_KEY_UP + 0x80,
	INP_KEY_RIGHT	  = GLUT_KEY_RIGHT + 0x80,
	INP_KEY_DOWN	  = GLUT_KEY_DOWN + 0x80,
	INP_KEY_PAGE_UP	  = GLUT_KEY_PAGE_UP + 0x80,
	INP_KEY_PAGE_DOWN = GLUT_KEY_PAGE_DOWN + 0x80,
	INP_KEY_HOME	  = GLUT_KEY_HOME + 0x80,
	INP_KEY_END		  = GLUT_KEY_END + 0x80,
	INP_KEY_INSERT	  = GLUT_KEY_INSERT + 0x80,
};


// ������
extern void InputInit(void);
// �I��
extern void InputFin(void);
// �X�V
extern void InputUpdate(void);
// �A�v���P�[�V�����p�̍X�V
extern void InputAppUpdate(void);

// �{�^���̃v���X�l���擾
extern BOOL InputGetBtnP(u_int btn);
// �{�^���̃g���K�l���擾
extern BOOL InputGetBtnTD(u_int btn);
// �{�^���̃g���K�l���擾
extern BOOL InputGetBtnTU(u_int btn);
// �}�E�X��X�l���擾
extern int InputGetMouseX(void);
// �}�E�X��Y�l���擾
extern int InputGetMouseY(void);
// �_�u���N���b�N������X�V
extern void InputFlashMouseClick(void);

extern u_char InputGetKey(void);
extern BOOL InputGetKeyPress(u_char key);
extern BOOL InputGetKeyPush(u_char key);
extern BOOL InputGetKeyPull(u_char key);
extern void InputSetKeyRepeat(BOOL repeat);

// �����Ԃ�ύX
extern void InputSetAppExec(int ch, BOOL exec);
extern BOOL InputIsAppExec(int ch);
extern void InputSetAppBtnExec(int ch, BOOL exec);
extern BOOL InputIsAppBtnExec(int ch);

// �{�^���̃v���X�l���擾
extern BOOL InputGetAppBtnP(int ch, u_int btn);
// �{�^���̃g���K�l���擾
extern BOOL InputGetAppBtnTD(int ch, u_int btn);
// �{�^���̃g���K�l���擾
extern BOOL InputGetAppBtnTU(int ch, u_int btn);
// �}�E�X��X�l���擾
extern int InputGetAppMouseX(int ch);
// �}�E�X��Y�l���擾
extern int InputGetAppMouseY(int ch);

extern u_char InputGetAppKey(int ch);
extern BOOL InputGetAppKeyPress(int ch, u_char key);
extern BOOL InputGetAppKeyPush(int ch, u_char key);
extern BOOL InputGetAppKeyPull(int ch, u_char key);

// �L�^�J�n
extern void InputRecordStart(int rec_max);
// �L�^��~
extern void InputRecordStop(void);
// �L�^�̈�̏�����
extern void InputRecordClean(void);
// �L�^��������
extern BOOL InputRecordIsExec(void);

// �t�@�C�������o��
extern void InputRecordWrite(sFILE *fp);
// �t�@�C���ǂݍ���
extern void InputRecordRead(sFILE *fp);

extern int InputGetPlayBackFrame(void);

// �Đ��J�n
extern void InputPlayBackStart(void);
// �Đ���~
extern void InputPlayBackStop(void);

extern BOOL InputIsPlayBack(void);
extern BOOL InputPlayBackIsExec(void);

/* ���̓f�[�^��malloc���ĕԋp */
extern u_char *InputCreateRecData(int mem_area);
extern void InputSetRecData(u_char *ptr);

#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================

