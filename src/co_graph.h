
/*

	prio の値はマイナス値でも設定可。値が小さいほど前面に表示されます。
	また、同じプライオリティ同士では、後から定義した方が前面になります。

*/

//==============================================================
#ifndef CO_GRAPH_H
#define CO_GRAPH_H
//==============================================================

#include "co_common.h"
#include "co_texture.h"

#ifdef __cplusplus
extern              "C"
{
#endif

/********************************************/
/*             定数・マクロ宣言             */
/********************************************/
enum enmGRP_TYPE {
	GRP_POINT,									// 点
	GRP_LINE,									// ライン
	GRP_BOX,									// ボックス(枠のみ)
	GRP_QUAD,									// 四角

	GRP_CIRCLE,									// 円(枠のみ)
	GRP_FILLCIRCLE,								// 塗りつぶし円
	GRP_TRIANGLE,								// 三角ポリゴン
	GRP_FAN,									// 扇
	GRP_FILLDONUT,								// 塗りつぶしドーナツ
};

enum enmGRP_BLEND {
	GRP_BLEND_NORMAL,							// 通常のブレンディング
	GRP_BLEND_ADD,								// 加算半透明
	GRP_BLEND_REV,								// 反転表示
	GRP_BLEND_XOR,								// XOR
	GRP_BLEND_MUL,								// 乗算
	GRP_BLEND_SCREEN,							// スクリーン合成
};


/********************************************/
/*                構造体宣言                */
/********************************************/
typedef struct _sGRPOBJ sGRPOBJ;


/********************************************/
/*              グローバル変数              */
/********************************************/


/********************************************/
/*              グローバル関数              */
/********************************************/
// 初期化
extern void GrpInit(void);
// 終了
extern void GrpFin(void);

// セットアップ
extern void GrpSetup(void);
// 描画
extern void GrpDraw(void);

// オブジェクトを生成
extern sGRPOBJ *GrpCreate(int type, int prio);
// GRP_POINTで作成
#define GRPOBJ_POINT(prio)       GrpCreate(GRP_POINT, prio)
// GRP_LINEで作成
#define GRPOBJ_LINE(prio)        GrpCreate(GRP_LINE, prio)
// GRP_BOXで作成
#define GRPOBJ_BOX(prio)         GrpCreate(GRP_BOX, prio)
// GRP_QUADで作成
#define GRPOBJ_QUAD(prio)        GrpCreate(GRP_QUAD, prio)
// GRP_CIRCLEで作成
#define GRPOBJ_CIRCLE(prio)      GrpCreate(GRP_CIRCLE, prio)
// GRP_FILLCIRCLEで作成
#define GRPOBJ_FILLCIRCLE(prio)  GrpCreate(GRP_FILLCIRCLE, prio)
// GRP_TRIANGLEで作成
#define GRPOBJ_TRIANGLE(prio)    GrpCreate(GRP_TRIANGLE, prio)
#define GRPOBJ_FAN(prio)         GrpCreate(GRP_FAN, prio)
#define GRPOBJ_FILLDONUT(prio)   GrpCreate(GRP_FILLDONUT, prio)

// 色設定
extern void GrpSetRGBA(sGRPOBJ *obj, REAL red, REAL green, REAL blue, REAL alpha);
// 色設定
extern void GrpSetRGBA4(sGRPOBJ *obj, sRGBA *c1, sRGBA *c2, sRGBA *c3, sRGBA *c4);
// 座標設定
extern void GrpSetPos(sGRPOBJ *obj, REAL x, REAL y);
// サイズ設定
extern void GrpSetSize(sGRPOBJ *obj, REAL w, REAL h);
// ドーナツのサイズを設定
extern void GrpSetDonutSize(sGRPOBJ *obj, REAL w1, REAL h1, REAL w2, REAL h2);
// ラインの描画数を設定
extern void GrpSetLineNum(sGRPOBJ *obj, int num);
// 扇形の角度を設定
extern void GrpSetFanAngle(sGRPOBJ *obj, float angle);
// 座標設定(４頂点)
extern void GrpSetVtx(sGRPOBJ *obj, FVector2 *v1, FVector2 *v2, FVector2 *v3, FVector2 *v4);
// UV座標設定
extern void GrpSetUV(sGRPOBJ *obj, REAL x, REAL y);
// UV座標設定(４頂点まとめて設定)
extern void GrpSetUV4(sGRPOBJ *obj, FVector2 *v1, FVector2 *v2, FVector2 *v3, FVector2 *v4);
// 点や線の描画サイズ設定
extern void GrpSetDrawSize(sGRPOBJ *obj, REAL size);
// スケーリング、回転中心を設定
extern void GrpSetCenter(sGRPOBJ *obj, REAL x, REAL y);
// スケール値を設定
extern void GrpSetScale(sGRPOBJ *obj, REAL x, REAL y);
// 回転角を設定
extern void GrpSetRot(sGRPOBJ *obj, REAL r);
// フリップ設定
extern void GrpSetFlip(sGRPOBJ *obj, BOOL flip_h, BOOL flip_v);
// ブレンディングモード設定
extern void GrpSetBlendMode(sGRPOBJ *obj, int mode);
// テクスチャを設定
extern void GrpSetTexture(sGRPOBJ *obj, sTexture *tex);
// エイリアシングモードを変更
extern void GrpSetSmooth(sGRPOBJ *obj, BOOL smooth);
// エイリアシングモードを取得
extern BOOL GrpGetSmooth(sGRPOBJ *obj);
// フィルタリングモードを変更
extern void GrpSetFilter(sGRPOBJ *obj, BOOL filter);
// フィルタリングモードを取得
extern BOOL GrpGetFilter(sGRPOBJ *obj, BOOL filter);

#ifdef DEBUG
extern int GrpGetDrawPrimNum(void);
#else
#define GrpGetDrawPrimNum()
#endif


#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================

