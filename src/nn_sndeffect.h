//
// ÉTÉEÉìÉhä«óù
//

//==============================================================
#ifndef NN_SNDEFFECT_H
#define NN_SNDEFFECT_H
//==============================================================

#include "co_common.h"

#ifdef __cplusplus
extern              "C"
{
#endif


extern void SndEffectStart(void);
extern void SndEffectReq(char *id_str, int delay, float gain);
extern void SndEffectStopAll(BOOL audio_stop);


#ifdef __cplusplus
}
#endif

//==============================================================
#endif
//==============================================================

