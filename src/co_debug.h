
//==============================================================
#ifndef CO_DEBUG_H
#define CO_DEBUG_H
//==============================================================

#include "co_common.h"

#ifdef __cplusplus
extern              "C"
{
#endif

/********************************************/
/*             �萔�E�}�N���錾             */
/********************************************/
// printf()
#ifdef DEBUG

	#if defined (__GNUC__)
        #define PRINTF(...)		DbgPrint(__VA_ARGS__)
        #define SYSINFO(...)		DbgSysInfo(__VA_ARGS__)
	#elif defined (_MSC_VER)
		#define PRINTF		DbgPrint
		#define SYSINFO		DbgSysInfo
	#endif

#else

	#if defined (__GNUC__)
		#define PRINTF(...)
		#define SYSINFO(...)
	#elif defined (_MSC_VER)
		#define PRINTF   __noop
		#define SYSINFO  __noop
	#endif

#endif

// assert()
#if defined (__GNUC__)
	#define ASSERT(test)  assert(test)
#elif defined (_MSC_VER)
	#define ASSERT(test)  assert(test)
#endif

// �Ǝ��g�� assert()
#if defined (__GNUC__)
    #define ASSERTM(test, ...)  AssertPrint(test, __VA_ARGS__)
    #define ASSERTW(test, ...)  if(!test) DbgPrint(__VA_ARGS__)
#elif defined (_MSC_VER)
	#define ASSERTM  AssertPrint
	#define ASSERTW  DbgPrint
#endif


/********************************************/
/*                �\���̐錾                */
/********************************************/


/********************************************/
/*              �O���[�o���ϐ�              */
/********************************************/


/********************************************/
/*              �O���[�o���֐�              */
/********************************************/
// printf�n
extern void DbgPrint(char *fmt, ...);
extern void DbgSysInfo(char *fmt, ...);

extern void AssertPrint(BOOL test, char *fmt, ...);

#ifdef __cplusplus
}
#endif

//==============================================================
#endif
//==============================================================
