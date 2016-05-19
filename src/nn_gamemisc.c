//
// アプリケーション汎用処理
//

#include "nn_gamemisc.h"
#include "co_graph.h"
#include "co_texture.h"
#include "co_misc.h"
#include "co_debug.h"
#include "co_fileutil.h"
#include "co_stack.h"
#include "nn_main.h"
#include "nn_camera.h"
#include "nn_weapon.h"
#include "nn_bullet.h"
#include "nn_bullet.h"
#include "nn_bomb.h"
#include "nn_lazer.h"
#include "nn_thunder.h"
#include "nn_extra.h"
#include "nn_player.h"


void SpriteSetup(Sprite *spr, char *name, sParam *param)
{
	char id_str[ID_MAXLEN];
	FVector2 *value;

	sprintf(id_str, "%s.uv", name);
	value = ParamGetFVec2(param, id_str);
	ASSERT(value);
	spr->uv = *value;
	
	sprintf(id_str, "%s.size", name);
	value = ParamGetFVec2(param, id_str);
	ASSERT(value);
	spr->size = *value;

	sprintf(id_str, "%s.pos", name);
	value = ParamGetFVec2(param, id_str);
	ASSERT(value);
	spr->pos = *value;

	spr->center = FVec2Zero;
	sprintf(id_str, "%s.center", name);
	if(ParamIsExists(param, id_str))
	{
		spr->center = *ParamGetFVec2(param, id_str);
	}

	spr->scale = FVec2One;
	spr->col = RGBAWhite;
	spr->blend = GRP_BLEND_NORMAL;

	spr->rot = 0.0f;
	spr->prio = PRIO_COCKPIT;
	sprintf(id_str, "%s.disp", name);
	spr->disp = ParamIsExists(param, id_str) ? ParamGetReal(param, id_str) : TRUE;
}

void SpriteDraw(Sprite *spr, sTexture *texture)
{
	if(!spr->disp) return;
	
	sGRPOBJ *grp = GRPOBJ_QUAD(spr->prio);
	GrpSetPos(grp, spr->pos.x, spr->pos.y);
	GrpSetRot(grp, spr->rot);
	GrpSetSize(grp, spr->size.x, spr->size.y);
	GrpSetUV(grp, spr->uv.x, spr->uv.y);
	GrpSetScale(grp, spr->scale.x, spr->scale.y);
	GrpSetCenter(grp, spr->center.x, spr->center.y);
	GrpSetRGBA(grp, spr->col.red, spr->col.green, spr->col.blue, spr->col.alpha);   
	GrpSetTexture(grp, texture);
	GrpSetBlendMode(grp, spr->blend);
	GrpSetFilter(grp, TRUE);
}

BOOL FieldClip(FVector2 *pos, float range)
{
	FVector2 *camera = CameraGetPos();

	sBox box;
	box.inf.x = (float)(-WINDOW_WIDTH / 2) - range;
	box.inf.y = (float)(-WINDOW_HEIGHT / 2) - range;
	box.sup.x = (float)(WINDOW_WIDTH / 2) + range;
	box.sup.y = (float)(WINDOW_HEIGHT / 2) + range;

//	PRINTF("%f %f\n", pos->x - camera->x, pos->y - camera->y);
	
	return MathBoxCrossPoint(pos->x - camera->x, pos->y - camera->y, &box) ? FALSE : TRUE;
}

sParam *ReadGameSettings(void)
{
	char *fname;

	if(MmFileCheck(SETTINGS_FILE)) fname = SETTINGS_FILE;
	else                           fname = PATH_DATA "/" SETTINGS_FILE;

	return ParamRead(fname);
}

void WriteGameSettings(sParam *settings)
{
	ParamWrite(SETTINGS_FILE, settings);
}

int CreateShot(sOBJ *obj, int type, int level, float powofs, char *id_str, u_int target_type, FVector2 *pos, FVector2 *vct, float dir, sParam *param)
{
	int shot_int = 30;
	switch(type)
	{
	case WEAPON_KIND_NONE:
	case WEAPON_KIND_SHOT:
		{
			shot_int = BulletCreate(id_str, pos, vct, dir, target_type, param, level, powofs);
		}
		break;

	case WEAPON_KIND_BOMB:
		{
			shot_int = BombCreate(id_str, pos, vct, dir, target_type, param, level, powofs);
		}
		break;

	case WEAPON_KIND_LAZER:
		{
			shot_int = LazerCreate(id_str, pos, vct, dir, target_type, param, level, powofs);
		}
		break;

	case WEAPON_KIND_THUNDER:
		{
			shot_int = ThunderCreate(id_str, obj, target_type, param, level, powofs);
		}
		break;

	case WEAPON_KIND_EXTRA:
		{
			shot_int = ExtraCreate(id_str, pos, vct, dir, target_type, param, level, powofs);
		}
		break;
	}
	return shot_int;
}

/* パワーを内部数値へ変換 */
int PowerConvertInterValue(float power)
{
	return power * 256.0f;
}

/* パワーを通常数値へ変換 */
int PowerConvertOuterValue(int power)
{
	return (power + 255) / 256;						/* 切り上げ */
}

/* パワーを減らす。 返り値:整数値の減った値 */
int PowerCalcDamage(int *power, int damage)
{
	int a = *power;
	int b = a - damage;
	*power = b;

	return ((a + 255) / 256) - ((b + 255) / 256);
}

int PowerAddValue(int power, int value)
{
	return power + value * 256;
}

void DmgHitColor(sRGBA *res, int timer, sParam *param)
{
	const FVector4 *dmg_col = ParamGetFVec4(param, (timer & 0x2) ? "dmg_col1" : "dmg_col2");
	SetRGBA(res, dmg_col->x, dmg_col->y, dmg_col->z, dmg_col->w);
}

/* フィールドを無限ループする処理 */
BOOL GameFieldRange(FVector2 *pos)
{
	sParam *param = MainGetParam();
	FVector2 *p_pos = PlayerGetPos();
	BOOL wrap = FALSE;

	if(param && p_pos)
	{
		float dx = pos->x - p_pos->x;
		float dy = pos->y - p_pos->y;
		float field_range = ParamGetReal(param, "field_range");
		if(dx > field_range)
		{
			pos->x -= field_range * 2.0f;
			wrap = TRUE;
		}
		else
		if(dx < -field_range)
		{
			pos->x += field_range * 2.0f;
			wrap = TRUE;
		}
		if(dy > field_range)
		{
			pos->y -= field_range * 2.0f;
			wrap = TRUE;
		}
		else
		if(dy < -field_range)
		{
			pos->y += field_range * 2.0f;
			wrap = TRUE;
		}
	}
	return wrap;
}

/* ダメージ関連 */
sOBJ *ShotGetDamageSender(void)
{
	return (sOBJ *)StkRefFrameP(0);
}

FVector2 *ShotGetDamagePos(void)
{
	return (FVector2 *)StkRefFrameP(1);
}

FVector2 *ShotGetDamageVct(void)
{
	return (FVector2 *)StkRefFrameP(2);
}

float ShotGetDamageRadius(void)
{
	return StkRefFrameF(3);
}

int ShotGetDamagePower(void)
{
	return StkRefFrameI(4);
}

int ShotGetLostWeapon(void)
{
	return StkRefFrameI(5);
}
