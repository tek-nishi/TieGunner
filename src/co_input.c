//
// 入力管理
//

#include "co_input.h"
#include "co_debug.h"
#include "co_memory.h"
#include "co_misc.h"
#include "co_zlib.h"


#define D_CLICK_TIME	  30						// ダブルクリックとみなす間隔
#define INPUT_RECORD_SIZE (FRAME_RATE * 2)			// 初期記録バッファサイズ

#define KEY_INPUT_KINDNUM 256
#define KEY_INPUT_BUFFER  8							/* 1フレーム内で処理できるキー同時押し */

/* #define MOUSE_MIN_POSITION  -0x7fffffff			// 最小値 */
#define MOUSE_MIN_POSITION  0


typedef struct {
	u_char key_push[KEY_INPUT_BUFFER];
	int key_push_num;
	u_char key_pull[KEY_INPUT_BUFFER];
	int key_pull_num;

	u_char last_key_push;
} InputKeyBuffer;

typedef struct {
	int m_x;										// コールバックから渡された座標
	int m_y;
	BOOL m_left;
	BOOL m_middle;
	BOOL m_right;

	int x;										// マウスの座標
	int y;
	u_int btn_p;								// ボタンの状態
	u_int btn_td;
	u_int btn_tu;

	int d_left_count;							// 左ダブルクリック用のカウンタ
	IVector2 l_pos;								// 左ダブルクリック用の位置

	InputKeyBuffer keyBuffer[2];
	int key_page;
	BOOL key_press[KEY_INPUT_KINDNUM];

	BOOL exec;									// TRUE = 処理中
	BOOL btnExec;
	BOOL record;								// TRUE = 記録中
	BOOL rec_datamax;
	BOOL playback;								// TRUE = 再生中
	BOOL playback_fin;
} sInput;


typedef struct {
	u_char key;
	BOOL push;
} RecKey;

typedef struct {
	int x, y;
	u_int btn_p;
	u_int btn_td;
	u_int btn_tu;

	RecKey recKey[KEY_INPUT_BUFFER * 2];
	int recNum;
} sInputRecord;


static sInput gInput[INPUT_WORK_NUM];
static sInputRecord *record;
static int record_num;
static int record_ofs;
static int record_max;
static int playback_ofs;


static void recordKeyPush(sInput *input, u_char key)
{
	input->key_press[key] = TRUE;

	InputKeyBuffer *keyBuffer = input->keyBuffer + input->key_page;
	keyBuffer->last_key_push = key;
	if(keyBuffer->key_push_num < KEY_INPUT_BUFFER)
	{
		keyBuffer->key_push[keyBuffer->key_push_num] = key;
		keyBuffer->key_push_num += 1;
//			PRINTF("%x:%d:%d\n", key, keyBuffer->key_push_num, g.time);
	}
}

static void recordKeyPull(sInput *input, u_char key)
{
	input->key_press[key] = FALSE;

	InputKeyBuffer *keyBuffer = input->keyBuffer + input->key_page;
	if(keyBuffer->key_pull_num < KEY_INPUT_BUFFER)
	{
		keyBuffer->key_pull[keyBuffer->key_pull_num] = key;
		keyBuffer->key_pull_num += 1;
//			PRINTF("%x:%d:%d\n", key, keyBuffer->key_pull_num, g.time);
	}
}

static void recordKeyInput(u_char key, BOOL push)
{
	sInput *input = &gInput[INP_CH0];				/* 記録を採るチャンネルは固定 */
	if(input->record && !input->rec_datamax && !g.stop)
	{
		sInputRecord *rec = record + record_ofs;
		RecKey *recKey = rec->recKey + rec->recNum;
		recKey->key = key;
		recKey->push = push;
		rec->recNum += 1;
		ASSERT(rec->recNum <= KEY_INPUT_BUFFER * 2);
	}
}

static void clearAppInput(sInput *input)
{
	input->btn_td = 0;
	input->btn_tu = 0;
	input->btn_p = 0;
	input->d_left_count = 0;

	input->keyBuffer[0].key_push_num = 0;
	input->keyBuffer[0].key_pull_num = 0;
	ZEROMEMORY(input->key_press, sizeof(BOOL) * KEY_INPUT_KINDNUM);
}


/*

  glutコールバック関数

*/
static void mouseBtnCallback(int button, int state, int x, int y)
{
	//---------------------------------------------------------------
	// FIXME: この単純な方法だと、短い間隔で DOWN→UP と変化した時に
	//        入力をとりこぼす場合があります
	//---------------------------------------------------------------
	switch(button)
	{
		case GLUT_LEFT_BUTTON:
		{
			gInput[INP_SYS].m_left = (state == GLUT_DOWN) ? TRUE : FALSE;
		}
		break;

		case GLUT_MIDDLE_BUTTON:
		{
			gInput[INP_SYS].m_middle = (state == GLUT_DOWN) ? TRUE : FALSE;
		}
		break;

		case GLUT_RIGHT_BUTTON:
		{
			gInput[INP_SYS].m_right = (state == GLUT_DOWN) ? TRUE : FALSE;
		}
		break;
	}
}

static void mousePosCallback(int x, int y)
{
	gInput[INP_SYS].m_x = x;
	gInput[INP_SYS].m_y = y;
}

static void keyPushCallback(u_char key, int x, int y)
{
//	PRINTF("PUSH:%x\n", key);
	
	if(key < 0x80)
	{
		recordKeyPush(&gInput[INP_SYS], key);
		recordKeyInput(key, TRUE);
	}
}

static void keyPullCallback(u_char key, int x, int y)
{
//	PRINTF("PULL:%x\n", key);

	if(key < 0x80)
	{
		recordKeyPull(&gInput[INP_SYS], key);
		recordKeyInput(key, FALSE);
	}
}

static void keyExPushCallback(int key, int x, int y)
{
	recordKeyPush(&gInput[INP_SYS], key + 0x80);
	recordKeyInput(key + 0x80, TRUE);
}

static void keyExPullCallback(int key, int x, int y)
{
	recordKeyPull(&gInput[INP_SYS], key + 0x80);
	recordKeyInput(key + 0x80, FALSE);
}


/*

 各種サブルーチン 

*/
static REAL mouseMoveLength(IVector2 *pos)
{
	REAL x;
	REAL y;

	x = gInput[INP_SYS].x - pos->x;
	y = gInput[INP_SYS].y - pos->y;

	return sqrtf(x * x + y * y);
}

static void mouseMakeBtnInfo(sInput *input, u_int btn)
{
	input->btn_td &= ~MOUSE_D_LEFT;

	input->btn_td = btn & ~(input->btn_p);
	input->btn_tu = ~btn & input->btn_p;
	input->btn_p = btn;

	// 左ダブルクリック判定
	//----------------------
	if(input->btn_td & MOUSE_LEFT)
	{
		if((input->d_left_count > 0) && (mouseMoveLength(&input->l_pos) < 2))
		{
			input->btn_td &= ~MOUSE_LEFT;
			input->btn_td |= MOUSE_D_LEFT;
			input->d_left_count = 0;
		}
		else
		{
			input->d_left_count = D_CLICK_TIME;
			input->l_pos.x = input->x;
			input->l_pos.y = input->y;
		}
	}
	if(input->d_left_count > 0)  input->d_left_count -= 1;
}

static void updateMouseInput(void)
{
	sInput *input;
	u_int btn = 0;

	input = &gInput[INP_SYS];
#if 1
	input->x = (float)(input->m_x - g.disp_ofs.x) * ((float)g.width / (float)(g.disp_size.x - (g.disp_ofs.x * 2)));
	input->y = (float)(input->m_y - g.disp_ofs.y) * ((float)g.height / (float)(g.disp_size.y - (g.disp_ofs.y * 2))); 
#else
	gInput[0].x = gInput[0].m_x - (g.disp_size.x - g.width) / 2;
	gInput[0].y = gInput[0].m_y - (g.disp_size.y - g.height) / 2;
#endif

	if(input->m_left)		btn |= MOUSE_LEFT;
	if(input->m_middle)		btn |= MOUSE_MIDDLE;
	if(input->m_right)		btn |= MOUSE_RIGHT;

	mouseMakeBtnInfo(input, btn);
}


static void recordInput(sInput *input)
{
	sInputRecord *rec = record + record_ofs;

	rec->x = input->x;
	rec->y = input->y;
	rec->btn_p = input->btn_p;
	rec->btn_td = input->btn_td;
	rec->btn_tu = input->btn_tu;
	
	if(++record_ofs == record_num)
	{
		if(record_num >= record_max)
		{
			input->rec_datamax = TRUE;
			PRINTF("InputRecord Datamax\n");
			/* 打ち止め */
		}
		else
		{
			record_num *= 2;
			if((record_max > 0) && (record_max < record_num)) record_num = record_max;
		
			record = (sInputRecord *)fsRealloc(record, sizeof(sInputRecord) * record_num, "record");
		}
	}
	rec = record + record_ofs;
	rec->recNum = 0;
}


static void playbackInput(sInput *input)
{
	sInputRecord *rec = record + playback_ofs;

	if(input->exec && !input->playback_fin)
	{
		input->x = rec->x;
		input->y = rec->y;
		if(input->btnExec)
		{
			input->btn_p = rec->btn_p;
			input->btn_td = rec->btn_td;
			input->btn_tu = rec->btn_tu;

			input->keyBuffer[0].key_push_num = 0;
			input->keyBuffer[0].key_pull_num = 0;
		
			for(int i = 0; i < rec->recNum; i += 1)
			{
				u_char key = (rec->recKey + i)->key;
				if((rec->recKey + i)->push)
				{
					recordKeyPush(input, key);
				}
				else
				{
					recordKeyPull(input, key);
				}
			}
		}
	}
	if(playback_ofs < record_num)
	{
		if(++playback_ofs == record_num)
		{
			input->playback_fin = TRUE;
			clearAppInput(input);
		}
	}
}

static void updateAppInput(sInput *input)
{
	if(input->playback)
	{
		playbackInput(input);
	}
	else
	{
		if(!input->exec)
		{
/* 			input->x = MOUSE_MIN_POSITION; */
/* 			input->y = MOUSE_MIN_POSITION; */
		}
		else
		{
			input->x = gInput[INP_SYS].x;
			input->y = gInput[INP_SYS].y;

			if(input->btnExec)
			{
				mouseMakeBtnInfo(input, gInput[INP_SYS].btn_p);
				input->keyBuffer[0] = gInput[INP_SYS].keyBuffer[gInput[INP_SYS].key_page ^ 1];
			}
		}

		if(input->record && !input->rec_datamax)
		{
			recordInput(input);
		}
	}
}


static BOOL inputGetBtnPress(int ch, u_int btn)
{
	return (gInput[ch].btn_p & btn) ? TRUE : FALSE;
}

static BOOL inputGetBtnPush(int ch, u_int btn)
{
	return (gInput[ch].btn_td & btn) ? TRUE : FALSE;
}

static BOOL inputGetBtnPull(int ch, u_int btn)
{
	return (gInput[ch].btn_tu & btn) ? TRUE : FALSE;
}

static int inputGetMouseX(int ch)
{
	return gInput[ch].x;
}

static int inputGetMouseY(int ch)
{
	return gInput[ch].y;
}


void InputInit(void)
{
	int i;

	ZEROMEMORY(gInput, sizeof(sInput) * INPUT_WORK_NUM);

	gInput[INP_SYS].exec = TRUE;

	for(i = 0; i < INPUT_WORK_NUM; ++i)
	{
		gInput[i].m_x = MOUSE_MIN_POSITION;
		gInput[i].m_y = MOUSE_MIN_POSITION;
		gInput[i].x = MOUSE_MIN_POSITION;
		gInput[i].y = MOUSE_MIN_POSITION;
	}

	record = NULL;

	glutMouseFunc(mouseBtnCallback);
	glutMotionFunc(mousePosCallback);
	glutPassiveMotionFunc(mousePosCallback);

	glutKeyboardFunc(keyPushCallback);
	glutKeyboardUpFunc(keyPullCallback);
	glutSpecialFunc(keyExPushCallback);
	glutSpecialUpFunc(keyExPullCallback);

	SYSINFO(".... input initialize");
}

void InputFin(void)
{
	glutMouseFunc(NULL);
	glutMotionFunc(NULL);
	glutPassiveMotionFunc(NULL);

	glutKeyboardFunc(NULL);
	glutKeyboardUpFunc(NULL);
	glutSpecialFunc(NULL);
	glutSpecialUpFunc(NULL);

	FreeWork(record);

	SYSINFO(".... input finish");
}

void InputUpdate(void)
{
	updateMouseInput();

	gInput[INP_SYS].key_page ^= 1;
//	PRINTF("%d:%d\n", gInput[INP_SYS].keyBuffer[gInput[INP_SYS].key_page ^ 1].key_push_num, g.time);

	InputKeyBuffer *keyBuffer = &gInput[INP_SYS].keyBuffer[gInput[INP_SYS].key_page];
	keyBuffer->last_key_push = 0;
	keyBuffer->key_push_num = 0;
	keyBuffer->key_pull_num = 0;
}

void InputAppUpdate(void)
{
	updateAppInput(&gInput[INP_CH0]);
	updateAppInput(&gInput[INP_CH1]);
}


BOOL InputGetBtnP(u_int btn)
{
	return inputGetBtnPress(INP_SYS, btn);
}

BOOL InputGetBtnTD(u_int btn)
{
	return inputGetBtnPush(INP_SYS, btn);
}

BOOL InputGetBtnTU(u_int btn)
{
	return inputGetBtnPull(INP_SYS, btn);
}

int InputGetMouseX(void)
{
	return inputGetMouseX(INP_SYS);
}

int InputGetMouseY(void)
{
	return inputGetMouseY(INP_SYS);
}

void InputFlashMouseClick(void)
{
	gInput[0].d_left_count = 0;
}


u_char InputGetKey(void)
{
	InputKeyBuffer *keyBuffer = &gInput[0].keyBuffer[gInput[0].key_page ^ 1];
	return keyBuffer->last_key_push;
}

BOOL InputGetKeyPress(u_char key)
{
	return gInput[0].key_press[key];
}

BOOL InputGetKeyPush(u_char key)
{
	InputKeyBuffer *keyBuffer = &gInput[0].keyBuffer[gInput[0].key_page ^ 1];

	int i;
	for(i = 0; i < keyBuffer->key_push_num; i += 1)
	{
		if(keyBuffer->key_push[i] == key) break;
	}
	
	return i < keyBuffer->key_push_num;
}

BOOL InputGetKeyPull(u_char key)
{
	InputKeyBuffer *keyBuffer = &gInput[0].keyBuffer[gInput[0].key_page ^ 1];

	int i;
	for(i = 0; i < keyBuffer->key_pull_num; i += 1)
	{
		if(keyBuffer->key_pull[i] == key) break;
	}
	
	return i < keyBuffer->key_pull_num;
}

void InputSetKeyRepeat(BOOL repeat)
{
	glutIgnoreKeyRepeat(repeat ? 0 : 1);
//	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
//	glutSetKeyRepeat(repeat ? GLUT_KEY_REPEAT_DEFAULT : GLUT_KEY_REPEAT_OFF);
}


void InputSetAppExec(int ch, BOOL exec)
{
/* 	PRINTF("InputSetAppExec:%d %d\n", ch, exec); */
	gInput[ch].exec = exec;
	if(!exec)
	{
		clearAppInput(&gInput[ch]);
	}
}

BOOL InputIsAppExec(int ch)
{
	return gInput[ch].exec;
}

void InputSetAppBtnExec(int ch, BOOL exec)
{
	gInput[ch].btnExec = exec;
	if(!exec)
	{
		clearAppInput(&gInput[ch]);
	}
}

BOOL InputIsAppBtnExec(int ch)
{
	return gInput[ch].btnExec;
}

BOOL InputGetAppBtnP(int ch, u_int btn)
{
	return inputGetBtnPress(ch, btn);
}

BOOL InputGetAppBtnTD(int ch, u_int btn)
{
	return inputGetBtnPush(ch, btn);
}

BOOL InputGetAppBtnTU(int ch, u_int btn)
{
	return inputGetBtnPull(ch, btn);
}

int InputGetAppMouseX(int ch)
{
	return inputGetMouseX(ch);
}

int InputGetAppMouseY(int ch)
{
	return inputGetMouseY(ch);
}

u_char InputGetAppKey(int ch)
{
	InputKeyBuffer *keyBuffer = &gInput[ch].keyBuffer[0];
	return keyBuffer->last_key_push;
}

BOOL InputGetAppKeyPress(int ch, u_char key)
{
	return gInput[ch].key_press[key];
}

BOOL InputGetAppKeyPush(int ch, u_char key)
{
	InputKeyBuffer *keyBuffer = &gInput[ch].keyBuffer[0];

	int i;
	for(i = 0; i < keyBuffer->key_push_num; i += 1)
	{
		if(keyBuffer->key_push[i] == key) break;
	}
	
	return i < keyBuffer->key_push_num;
}

BOOL InputGetAppKeyPull(int ch, u_char key)
{
	InputKeyBuffer *keyBuffer = &gInput[ch].keyBuffer[0];

	int i;
	for(i = 0; i < keyBuffer->key_pull_num; i += 1)
	{
		if(keyBuffer->key_pull[i] == key) break;
	}
	
	return i < keyBuffer->key_pull_num;
}


void InputRecordStart(int rec_max)
{
	gInput[INP_CH0].record = TRUE;
	gInput[INP_CH0].playback = FALSE;
	gInput[INP_CH0].rec_datamax = FALSE;

	for(int i = 0; i < INPUT_WORK_NUM; i += 1)
	{
		ZEROMEMORY(gInput[i].key_press, sizeof(gInput[i].key_press));
	}

	FreeWork(record);

	record_num = INPUT_RECORD_SIZE;
	record_ofs = 0;
	record_max = rec_max;							/* 0:無限 */
	record = (sInputRecord *)fsMalloc(sizeof(sInputRecord) * record_num, "record");
	record->recNum = 0;
}

void InputRecordStop(void)
{
	gInput[INP_CH0].record = FALSE;
}

void InputRecordClean(void)
{
	FreeWork(record);
	record = NULL;
	gInput[INP_CH0].record = FALSE;
	gInput[INP_CH0].playback = FALSE;
}

BOOL InputRecordIsExec(void)
{
	return gInput[INP_CH0].record;
}


typedef struct {
	u_char *ptr;
	int ofs;
	int size;
} FileBuffer;

static void putCharValue(FileBuffer *buffer, u_char value)
{
	u_char *ptr = buffer->ptr;
	int ofs = buffer->ofs;
	int size = buffer->size;
	
	if((ofs + 1) >= size)
	{
		size = size * 2;
		ptr = (u_char *)fsRealloc(ptr, size, "byteValue");
		buffer->ptr = ptr;
		buffer->size = size;
	}
	*(ptr + ofs) = value;
	ofs += 1;
	buffer->ofs = ofs;
}

static void putIntValue(FileBuffer *buffer, u_int value)
{
	u_char *ptr = buffer->ptr;
	int ofs = buffer->ofs;
	int size = buffer->size;

	if((ofs + 4) >= size)
	{
		size = size * 2;
		ptr = (u_char *)fsRealloc(ptr, size, "byteValue");
		buffer->ptr = ptr;
		buffer->size = size;
	}
	*(u_int *)(ptr + ofs) = htonl(value);
	ofs += sizeof(u_int);
	buffer->ofs = ofs;
}

static u_char getCharValue(FileBuffer *buffer)
{
	u_char *ptr = buffer->ptr + buffer->ofs;
	buffer->ofs += 1;
	
	return *ptr;
}

static u_int getIntValue(FileBuffer *buffer)
{
	u_int *ptr = (u_int *)(buffer->ptr + buffer->ofs);
	buffer->ofs += 4;
	
	return ntohl(*ptr);
}


void InputRecordWrite(sFILE *fp)
{
	if(record && record_ofs)
	{
		FileBuffer buffer;
		buffer.ptr = (u_char *)fsMalloc(256, "byteValue");
		buffer.ofs = 0;
		buffer.size = 256;

		sInputRecord *ptr = record;
		for(int i = 0; i < record_ofs; i += 1)
		{
			putIntValue(&buffer, (u_int)ptr->x);
			putIntValue(&buffer, (u_int)ptr->y);
			putIntValue(&buffer, ptr->btn_p);
			putIntValue(&buffer, ptr->btn_td);
			putIntValue(&buffer, ptr->btn_tu);

			RecKey *recKey = ptr->recKey;
			for(int h = 0; h < KEY_INPUT_BUFFER * 2; h += 1)
			{
				putCharValue(&buffer, recKey->key);
				putCharValue(&buffer, recKey->push);
				recKey += 1;
			}
			putIntValue(&buffer, ptr->recNum);
			ptr += 1;
		}
		
		u_int header;
		header = htonl(record_ofs);
		FsWrite(fp, &header, sizeof(u_int));

		void *zptr = ZlibEncode(buffer.ptr, buffer.ofs);
		FsWrite(fp, zptr, ZlibEncodeSize(zptr));
		Free(zptr);
		Free(buffer.ptr);
	}
}

void InputRecordRead(sFILE *fp)
{
	gInput[INP_CH0].record = FALSE;
	gInput[INP_CH0].playback = FALSE;

	u_int header;
	FsRead(fp, &header, sizeof(u_int));
	record_num = ntohl(header);
	record_ofs = record_num;

	FreeWork(record);
	record = (sInputRecord *)fsMalloc(sizeof(sInputRecord) * record_num, "record");
	ASSERT(record);

	FileBuffer buffer;
	size_t size = FsGetSize(fp) - 4;
	void *zptr = fsMalloc(size, "zlib");
	ASSERT(zptr);
	FsRead(fp, zptr, size);
	buffer.ptr = (u_char *)ZlibDecode(zptr, ZlibEncodeSize(zptr));
	buffer.ofs = 0;
	Free(zptr);

	sInputRecord *ptr = record;
	for(int i = 0; i < record_ofs; i += 1)
	{
		ptr->x = (int)getIntValue(&buffer);
		ptr->y = (int)getIntValue(&buffer);
		ptr->btn_p = getIntValue(&buffer);
		ptr->btn_td = getIntValue(&buffer);
		ptr->btn_tu = getIntValue(&buffer);

		RecKey *recKey = ptr->recKey;
		for(int h = 0; h < KEY_INPUT_BUFFER * 2; h += 1)
		{
			recKey->key = getCharValue(&buffer);
			recKey->push = (BOOL)getCharValue(&buffer);
			recKey += 1;
		}
		ptr->recNum = getIntValue(&buffer);
		ptr += 1;
	}
	Free(buffer.ptr);
}

int InputGetPlayBackFrame(void)
{
	return record_num;
}

void InputPlayBackStart(void)
{
	gInput[INP_CH0].record = FALSE;
	gInput[INP_CH0].playback = TRUE;
	gInput[INP_CH0].playback_fin = FALSE;
	clearAppInput(&gInput[INP_CH0]);

	playback_ofs = 0;
}

void InputPlayBackStop(void)
{
	gInput[INP_CH0].playback = FALSE;
}

BOOL InputIsPlayBack(void)
{
	return gInput[INP_CH0].playback;
}

BOOL InputPlayBackIsExec(void)
{
	return gInput[INP_CH0].playback_fin;
}

u_char *InputCreateRecData(int mem_area)
{
	u_char *ptr = NULL;

	if(record != NULL)
	{
		ptr = (u_char *)MemMalloc(mem_area, 4 + sizeof(sInputRecord) * record_ofs, "record");
		ASSERT(ptr);
		*(u_int *)ptr = htonl(record_ofs);
		memcpy(ptr + sizeof(u_int), record, sizeof(sInputRecord) * record_ofs);
		/* FIXME:頭の４バイトにデータサイズを書きこんでいる…が、他に良い実装を思い付かず */
	}
	
	return ptr;
}

void InputSetRecData(u_char *ptr)
{
	gInput[INP_CH0].record = FALSE;
	gInput[INP_CH0].playback = FALSE;

	FreeWork(record);
	
	record_num = ntohl(*(u_int *)ptr);
	record = (sInputRecord *)fsMalloc(sizeof(sInputRecord) * record_num, "record");
	memcpy(record, ptr + 4, sizeof(sInputRecord) * record_num);
	/* FIXME:頭の４バイトがデータサイズになっている…が、他に良い実装を思い付かず */
}
