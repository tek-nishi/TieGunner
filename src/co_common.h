
//==============================================================
#ifndef CO_COMMON_H
#define CO_COMMON_H
//==============================================================

/*

	DEBUG			�f�o�b�O�@�\��L���ɂ��ăR���p�C��(�����Ő錾����I)

	__APPLE__       OSX�ŃR���p�C��(�����I�ɐ錾����Ă���)
	_MSC_VER		VC�ŃR���p�C��(�����I�ɐ錾����Ă���)
	__GNUC__		GCC�ŃR���p�C��(�����I�ɐ錾����Ă���)

*/

#ifdef DEBUG
	#define CRTDBG_MAP_ALLOC					// ���������[�N��ǐՂ���
#endif


//------------------------------
// ANSI�Ƃ�
//------------------------------
#include <ctype.h>
#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/stat.h>

#if defined (__GNUC__)
	#include <dirent.h>
#endif

#if defined (_MSC_VER) || defined (WIN32)
#include <windows.h>								/* ���ǁc ntohl()��htonl()���g���� */
#pragma comment (lib, "Ws2_32.lib")
#endif

//------------------------------
// �O�����C�u����
//------------------------------
#if defined (__APPLE__)
#include <GLUT/glut.h>
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <GL/glut.h>
#include <AL/al.h>
#include <AL/alc.h>
#endif

#include <zlib.h>
#include <png.h>
#ifndef png_infopp_NULL 
#define png_infopp_NULL NULL
#endif

#include "co_types.h"
#include "co_message.h"


#ifdef __cplusplus
extern              "C"
{
#endif

#define PROJECT_NAME    "TieGunner"					// �^�C�g���o�[�ɕ\�������v���_�N�g��
#define VERSION_NUMBER  "1.01"						// ���o�[�W�����ԍ�
#define BUILD_NUMBER    "142"						/* �r���h�ԍ� */

#define WINDOW_WIDTH   512
#define WINDOW_HEIGHT  512

#define FRAME_RATE		60							/* �P�b������̃��[�v�� */
#define UPDATE_INTERVAL (1000 / FRAME_RATE)			// �X�V�Ԋu(ms)

#define FNAME_MAXLEN   256							// �p�X���̍ő咷
#define ID_MAXLEN      16							// ID���̍ő咷

#define PATH_DEVELOP "devdata"						// �J�����[�g�p�X
#define PATH_DATA	 PATH_DEVELOP					// �J���f�[�^(�O���t�B�b�N�E�T�E���h�Ȃǂ̌��f�[�^)
#define PATH_BACKUP  PATH_DEVELOP"/backup"
#define IMAGE_FILE	 "files.dat"


//------------------------------
//	�V�X�e���J���[��`(HTML ����)
//------------------------------
enum tagSYS_COLOR {
	COL_BLACK,									// ��
	COL_GLAY,									// �D�F
	COL_SILVER,									// ���邢�D�F
	COL_WHITE,									// ��

	COL_RED,									// ��
	COL_YELLOW,									// ���F
	COL_LIME,									// ��
	COL_AQUA,									// ���F
	COL_BLUE,									// ��
	COL_FUCHSIA,								// ��

	COL_MAROON,									// �Â���
	COL_OLIVE,									// �Â����F
	COL_GREEN,									// �Â���
	COL_TEAL,									// �Â����F
	COL_NAVY,									// �Â���
	COL_PURPLE,									// �Â���

	// ---------------------
	COL_MAX
};


//------------------------------
//	�f�o�b�O�t���O
//------------------------------
enum enumDEBUG_FLAG {
	DBG_FLAG_01 = (1 << 0),
	DBG_FLAG_02 = (1 << 1),
	DBG_FLAG_03 = (1 << 2),
	DBG_FLAG_04 = (1 << 3),
	DBG_FLAG_05 = (1 << 4),
	DBG_FLAG_06 = (1 << 5),
	DBG_FLAG_07 = (1 << 6),
	DBG_FLAG_08 = (1 << 7),
	DBG_FLAG_09 = (1 << 8),
	DBG_FLAG_10 = (1 << 9),
	DBG_FLAG_11 = (1 << 10),
	DBG_FLAG_12 = (1 << 11),
	DBG_FLAG_13 = (1 << 12),
	DBG_FLAG_14 = (1 << 13),
	DBG_FLAG_15 = (1 << 14),
	DBG_FLAG_16 = (1 << 15),
	DBG_FLAG_17 = (1 << 16),
	DBG_FLAG_18 = (1 << 17),
	DBG_FLAG_19 = (1 << 18),
	DBG_FLAG_20 = (1 << 19),
	DBG_FLAG_21 = (1 << 20),
	DBG_FLAG_22 = (1 << 21),
	DBG_FLAG_23 = (1 << 22),
	DBG_FLAG_24 = (1 << 23),

	// ---------------------
	DBG_FLAG_NONE = 0,
	DBG_FLAG_ALL  = (~0)
};


//------------------------------
// �}�N���Ŏ���
//------------------------------
#ifdef DEBUG
	#define EXIT(val)	while(1)
#else
	#define EXIT(val)
#endif


//------------------------------------------
// �O���[�o���ϐ�
// ���v���O�����N��/�\�t�g�E�F�A���Z�b�g���ɏ�����
//------------------------------------------
typedef struct {
	int width, height;							// �t���[���o�b�t�@�̃T�C�Y
	IVector2 disp_size;							// �\���T�C�Y
	IVector2 disp_ofs;
	sRGBA bg_col;								// �w�i�F

	GLuint display_list;
	GLuint display_page;

	u_int time;									// �Q�[�����J�n����Ă���̌o�ߎ��� ( 2^32�t���ڂ� 0 �ɖ߂� )
	BOOL stop;									// TRUE: �S�Ă̓����������X�g�b�v��
	int slow;									// �Q�[���̐i�s���X���[�ɂ���Ԋu(0 = 100%)
	int slow_intvl;								// �X���[�̃t���[���Ԋu
	int step_loop;								// �X�e�b�v��(1�`)

	BOOL softreset;								// TRUE: �\�t�g���Z�b�g�v��
	BOOL app_exit;								// TRUE: �A�v���P�[�V�����I��
	BOOL window_reset;							/* TRUE: �T�C�Y������ */

	void *msgarg_stack;							// ���b�Z�[�W�p�X�^�b�N(��L���� void �^)

#ifdef DEBUG
	u_int debug_flag;
#endif
} GLOBAL_COMMON;

extern GLOBAL_COMMON g;								/* main.c�ɂĒ�` */

#ifdef __cplusplus
}
#endif

//==============================================================
#endif
//==============================================================

