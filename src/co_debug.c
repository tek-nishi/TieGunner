
#include "co_debug.h"
#include "co_os.h"

//==========================================================================
#if defined DEBUG
//==========================================================================

#define BUFFER_LENGTH  1024							/* FIXME:可変引数のバッファサイズ問題 */


void DbgPrint(char *fmt, ...)
{
	va_list arg;
	char work[BUFFER_LENGTH];

	// 可変長引数の取得
	//------------------
	va_start(arg, fmt);
	vsprintf(work, fmt, arg);
	va_end(arg);

	OsPrintf(work);
}

void DbgSysInfo(char *fmt, ...)
{
	va_list arg;
	char work[BUFFER_LENGTH];

	// 可変長引数の取得
	//------------------
	va_start(arg, fmt);
	vsprintf(work, fmt, arg);
	va_end(arg);
	strcat(work, "\n");

	OsPrintf(work);
}

void AssertPrint(BOOL test, char *fmt, ...)
{
	if(!test)
	{
		va_list arg;
		char work[BUFFER_LENGTH];

		// 可変長引数の取得
		//------------------
		va_start(arg, fmt);
		vsprintf(work, fmt, arg);
		va_end(arg);
		strcat(work, "\n");

		OsPrintf(work);
		abort();
	}
}

//==========================================================================
#endif
//==========================================================================

