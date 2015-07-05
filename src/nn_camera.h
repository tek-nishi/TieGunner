//
// ÉJÉÅÉâèàóù
//

//==============================================================
#ifndef NN_CAMERA_H
#define NN_CAMERA_H
//==============================================================

#include "co_common.h"

#ifdef __cplusplus
extern              "C"
{
#endif

extern void CameraCreate(void);
extern FVector2 *CameraGetPos(void);
extern void CameraSetPos(FVector2 *pos);
extern FVector2 *CameraGetVct(void);
extern void CameraSetVct(FVector2 *vct);
extern void CameraSetDecay(float decay, int time);
extern float CameraGetDecay(void);

#ifdef __cplusplus
}
#endif

//==============================================================
#endif
//==============================================================
