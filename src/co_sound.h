
//==============================================================
#ifndef CO_SOUND_H
#define CO_SOUND_H
//==============================================================

#include "co_common.h"
#if defined (_MSC_VER)
#pragma comment (lib, "openal32.lib")
#endif

#ifdef __cplusplus
extern              "C"
{
#endif

typedef struct _SndObj SndObj;


extern void SndInit(void);
extern void SndFin(void);

extern BOOL SndIsActive(void);
extern void SndPlay(char *id_str, float gain);
extern void SndStop(char *id_str);
extern void SndStopAll(void);

extern void SndSetMasterGain(float gain);
extern float SndGetMasterGain(void);

extern SndObj *SndReadWAV(char *fname);
//extern SndObj *SndReadAAC(char *fname);
//extern SndObj *SndReadMP3(char *fname);
extern void SndDestroy(SndObj *obj);
extern char *SndObjGetName(SndObj *obj);


#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================

