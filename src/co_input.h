
/*

	GLUTで提供されている入力から、ゲーム用の入力情報を生成します。
	InputGetKey() で返されるASCIIコードですが、TAB や ESC とかも
	ASCIIコードで返します。
	
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
	MOUSE_LEFT	 = (1 << 0),						// マウス左ボタン
	MOUSE_MIDDLE = (1 << 1),						// マウス中央ボタン
	MOUSE_RIGHT	 = (1 << 2),						// マウス右ボタン
	MOUSE_D_LEFT = (1 << 3),						// マウス左ボタンダブルクリック
};

enum {
	ASCII_ENTER = 13,								// リターンキー
	ASCII_BS	= 8,								// BSキー
	ASCII_DEL	= 127,								// DELキー
	ASCII_ESC	= 27,								// ESCキー
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


// 初期化
extern void InputInit(void);
// 終了
extern void InputFin(void);
// 更新
extern void InputUpdate(void);
// アプリケーション用の更新
extern void InputAppUpdate(void);

// ボタンのプレス値を取得
extern BOOL InputGetBtnP(u_int btn);
// ボタンのトリガ値を取得
extern BOOL InputGetBtnTD(u_int btn);
// ボタンのトリガ値を取得
extern BOOL InputGetBtnTU(u_int btn);
// マウスのX値を取得
extern int InputGetMouseX(void);
// マウスのY値を取得
extern int InputGetMouseY(void);
// ダブルクリック判定を更新
extern void InputFlashMouseClick(void);

extern u_char InputGetKey(void);
extern BOOL InputGetKeyPress(u_char key);
extern BOOL InputGetKeyPush(u_char key);
extern BOOL InputGetKeyPull(u_char key);
extern void InputSetKeyRepeat(BOOL repeat);

// 動作状態を変更
extern void InputSetAppExec(int ch, BOOL exec);
extern BOOL InputIsAppExec(int ch);
extern void InputSetAppBtnExec(int ch, BOOL exec);
extern BOOL InputIsAppBtnExec(int ch);

// ボタンのプレス値を取得
extern BOOL InputGetAppBtnP(int ch, u_int btn);
// ボタンのトリガ値を取得
extern BOOL InputGetAppBtnTD(int ch, u_int btn);
// ボタンのトリガ値を取得
extern BOOL InputGetAppBtnTU(int ch, u_int btn);
// マウスのX値を取得
extern int InputGetAppMouseX(int ch);
// マウスのY値を取得
extern int InputGetAppMouseY(int ch);

extern u_char InputGetAppKey(int ch);
extern BOOL InputGetAppKeyPress(int ch, u_char key);
extern BOOL InputGetAppKeyPush(int ch, u_char key);
extern BOOL InputGetAppKeyPull(int ch, u_char key);

// 記録開始
extern void InputRecordStart(int rec_max);
// 記録停止
extern void InputRecordStop(void);
// 記録領域の初期化
extern void InputRecordClean(void);
// 記録中か判別
extern BOOL InputRecordIsExec(void);

// ファイル書き出し
extern void InputRecordWrite(sFILE *fp);
// ファイル読み込み
extern void InputRecordRead(sFILE *fp);

extern int InputGetPlayBackFrame(void);

// 再生開始
extern void InputPlayBackStart(void);
// 再生停止
extern void InputPlayBackStop(void);

extern BOOL InputIsPlayBack(void);
extern BOOL InputPlayBackIsExec(void);

/* 入力データをmallocして返却 */
extern u_char *InputCreateRecData(int mem_area);
extern void InputSetRecData(u_char *ptr);

#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================

