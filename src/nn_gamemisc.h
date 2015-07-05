//
// アプリケーション汎用処理
//

//==============================================================
#ifndef NN_GAMEMISC_H
#define NN_GAMEMISC_H
//==============================================================

#include "co_common.h"
#include "co_param.h"
#include "co_obj.h"

#ifdef __cplusplus
extern              "C"
{
#endif

typedef struct {
	FVector2 pos;
	float rot;
	FVector2 size;
	FVector2 uv;
	FVector2 scale;
	FVector2 center;
	sRGBA col;
	int blend;
	int prio;
	BOOL disp;
} Sprite;

extern void SpriteSetup(Sprite *spr, char *name, sParam *param);
extern void SpriteDraw(Sprite *spr, sTexture *texture);

extern BOOL FieldClip(FVector2 *pos, float range);

extern sParam *ReadGameSettings(void);
extern void WriteGameSettings(sParam *settings);

extern int CreateShot(sOBJ *obj, int type, int level, float powofs, char *id_str, u_int target_type, FVector2 *pos, FVector2 *vct, float dir, sParam *param);
extern int PowerConvertInterValue(float power);
extern int PowerConvertOuterValue(int power);
extern int PowerCalcDamage(int *power, int damage);
extern int PowerAddValue(int power, int value);
extern void DmgHitColor(sRGBA *res, int timer, sParam *param);
extern BOOL GameFieldRange(FVector2 *pos);

extern sOBJ *ShotGetDamageSender(void);
extern FVector2 *ShotGetDamagePos(void);
extern FVector2 *ShotGetDamageVct(void);
extern float ShotGetDamageRadius(void);
extern int ShotGetDamagePower(void);
extern int ShotGetLostWeapon(void);


#ifdef __cplusplus
}
#endif

//==============================================================
#endif
//==============================================================
