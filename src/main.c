//
// アプリケーション起動・終了
//

#include "co_common.h"
#include "co_os.h"
#include "co_memory.h"
#include "co_file.h"
#include "co_debug.h"
#include "co_random.h"
#include "co_texture.h"
#include "co_graph.h"
#include "co_font.h"
#include "co_input.h"
#include "co_param.h"
#include "co_task.h"
#include "co_obj.h"
#include "co_misc.h"
#include "co_stack.h"
#include "co_sound.h"
#include "nn_main.h"


#define TASK_PROC_MAX  64

GLOBAL_COMMON g;									/* TODO:別の方法は無いものか… */

static BOOL first_init	   = TRUE;
static BOOL core_rand_init = FALSE;
static BOOL core_req_init  = TRUE;
static int	intarval_time;
static BOOL intarval_flag;
/* static int time_exec; */


#if 0
/* 画面の同期が取れているか調べる */
static BOOL analyzeDispSync(void)
{
	int time_current = glutGet(GLUT_ELAPSED_TIME);

	/* FIXME:何回か更新してようやくタイマで計測できる */
	glClearColor(0, 0, 0, 0);
	for(int i = 0; i < 5; i += 1)
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glutSwapBuffers();
	}

	time_exec = glutGet(GLUT_ELAPSED_TIME) - time_current;

	return time_exec >= UPDATE_INTERVAL;
}
#endif

static void globalWorkInit(BOOL first)
{
	IVector2 disp_size = g.disp_size;
	IVector2 disp_ofs = g.disp_ofs;
/* 	IVector2 start_pos = g.start_pos; */
	/* ↑resizeCallback()内で初期化済 */

#ifdef DEBUG
	u_int debug_flag = g.debug_flag;
	int slow = g.slow;
#endif
	
	ZEROMEMORY(&g, sizeof(GLOBAL_COMMON));

#ifdef DEBUG
	if(!first)
	{
		g.debug_flag = debug_flag;
		g.slow = slow;
		g.slow_intvl = g.slow;
	}
#endif

	g.width  = WINDOW_WIDTH;
	g.height = WINDOW_HEIGHT;

	g.bg_col = RGBAClear;

	g.disp_size = disp_size;
	g.disp_ofs = disp_ofs;
/* 	g.start_pos = start_pos; */
	
/* 	g.step_loop = 1; */
}

static void systemInit(void)
{
	SYSINFO("---- First Init. Start. ----\n");

	OsInit();
#ifdef DEBUG										/* デバッグ時はソフトリセット時に乱数の初期化をしない */
	if(!core_rand_init)
#endif
	{
		RandomInit();
		core_rand_init = TRUE;
	}
	SinTblInit();
	MemInit();
	FsInit();
	FsMountImage(IMAGE_FILE);
	GrpInit();
	TexInit();
	FontInit();
	InputInit();
	ParamInit();
	TaskInit(TASK_PROC_MAX);
	ObjInit();
	SndInit();
	g.msgarg_stack =StkCreate();
	g.display_list = glGenLists(2);
	
	SYSINFO("---- First Init. Fin. ----\n");
}

static void systemFin(void)
{
	SYSINFO("---- Exit Process Start. ----\n");

	glDeleteLists(g.display_list, 2);
	StkKill((sSTACK *)g.msgarg_stack);
	SndFin();
	TaskFin();
	ParamFin();
	InputFin();
	FontFin();
	TexFin();
	GrpFin();
	FsFin();
	MemFin();
	OsFin();
	
	SYSINFO("---- Exit Process Fin. ----\n");
}

static void displayCallback(void)
{
/* 	int time_current; */

	if(core_req_init)
	{
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		glutSwapBuffers();

		globalWorkInit(first_init);
		systemInit();
		MainExec();									/* ゲームメイン実行開始 */
		
		first_init = FALSE;
		core_req_init = FALSE;
	}

/* 	time_current = glutGet(GLUT_ELAPSED_TIME); */

	if(g.slow_intvl > 0)
	{
		if(g.slow_intvl -= 1) return;
		g.slow_intvl = g.slow;
	}

	glNewList(g.display_list + g.display_page, GL_COMPILE);
	glClearColor(g.bg_col.red, g.bg_col.green, g.bg_col.blue, g.bg_col.alpha);
	glClear(GL_COLOR_BUFFER_BIT);
	GrpSetup();

//	for(int i = 0; i < 4; i += 1)
	{
		InputUpdate();
		if(!g.stop) InputAppUpdate();

		TaskUpdate(MSG_PREPROC, g.stop);
		TaskUpdate(MSG_STEP, g.stop);
		TaskUpdate(MSG_UPDATE, g.stop);
		g.time += 1;
	}
	TaskUpdate(MSG_DRAW, FALSE);
#ifdef DEBUG
	FontPrintF(512 - 30, 0, 0, "%d:%d", intarval_flag, intarval_time);
#endif

	GrpDraw();
	glEndList();
	glCallList(g.display_list + g.display_page);
	g.display_page ^= 1;
	glutSwapBuffers();

/* 	time_exec = glutGet(GLUT_ELAPSED_TIME) - time_current; */

	if(g.app_exit)
	{
		exit(0);
	}
	else
	if(g.softreset)
	{
		core_req_init = TRUE;
		systemFin();
	}
	else
	if(g.window_reset)
	{
		g.window_reset = FALSE;
//		glutPositionWindow(g.start_pos.x, g.start_pos.y);
		glutReshapeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);
	}
}

static void timerCallback(int value)
{
	glutTimerFunc(intarval_time, timerCallback, 0);
	glutPostRedisplay();
}

static void resizeCallback(int w, int h)
{
#if 1
	int width, height;

#if 1
	/* 縦横比を維持したサイズ変更 */
	width = w < h ? w : h;
	height = h < w ? h : w;
#else
	/* ウインドウサイズに合わせたサイズ変更 */
	width = w;
	height = h;
#endif

	int x_ofs = (w - width) / 2;
	int y_ofs = (h - height) / 2;
	g.disp_ofs.x = x_ofs;
	g.disp_ofs.y = y_ofs;
		
	glViewport(x_ofs, y_ofs, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -1.0, 1.0);
#else
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	int ofs_x;
	int ofs_y;
	ofs_x = (w - WINDOW_WIDTH) / 2;
	ofs_y = (h - WINDOW_HEIGHT) / 2;
	glOrtho(0 - ofs_x, w - ofs_x, h - ofs_y, 0 - ofs_y, -1.0, 1.0);
#endif

	g.disp_size.x = w;
	g.disp_size.y = h;
}

static void mainFin(void)
{
	systemFin();
}


int main(int argc, char **argv)
{
	int x = -1;
	int y = -1;

	
	glutInit(&argc, argv);

	int w = glutGet(GLUT_SCREEN_WIDTH);
	int h = glutGet(GLUT_SCREEN_HEIGHT);
	if(w > 0)
	{
		x = (w - (WINDOW_WIDTH + 8)) / 2;		// 若干のりしろを付けておく
		if(x < 0)	x = -1;
	}
	if(h > 0)
	{
		y = (h - (WINDOW_HEIGHT + 16)) / 2;		// 若干のりしろを付けておく
		if(y < 0)	y = -1;
	}
/* 	g.start_pos.x = x; */
/* 	g.start_pos.y = y; */

	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(x, y);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutCreateWindow(PROJECT_NAME "  Ver " VERSION_NUMBER "." BUILD_NUMBER);

	{
		BOOL res = OsIsVsyncSwap();
		if(res) res = OsToggleVsyncSwap(1);
		intarval_time = res ? UPDATE_INTERVAL - 1 : UPDATE_INTERVAL;
		intarval_flag = res;
	}

	glutDisplayFunc(displayCallback);
	glutReshapeFunc(resizeCallback);
	glutTimerFunc(intarval_time, timerCallback, 0);
	atexit(mainFin);

	glutMainLoop();

	return 0;
}
