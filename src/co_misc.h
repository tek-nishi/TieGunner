
//==============================================================
#ifndef CO_MISC_H
#define CO_MISC_H
//==============================================================

#include "co_common.h"

#ifdef __cplusplus
extern              "C"
{
#endif

//------------------------
// 角度単位変換
//------------------------
#define	ANG2RAD(angle)	((angle) * PI / 180.0f)
#define	RAD2ANG(angle)	((angle) * 180.0f / PI)


extern const FVector2 FVec2Zero;
extern const FVector2 FVec2One;
extern const FVector3 FVec3One;
extern const FVector4 FVec4One;

extern const sRGBA RGBAClear;
extern const sRGBA RGBABlack;
extern const sRGBA RGBAWhite;


// 色セット
extern void SetRGBA(sRGBA *dst, REAL r, REAL g, REAL b, REAL a);

// ベクトルの値をセット
extern void SetV2d(FVector2 *dst, REAL x, REAL y);
// ベクトルの加算(dst = s0 + s1)
extern void AddV2d(FVector2 *dst, FVector2 *s0, FVector2 *s1);
// ベクトルの減算(dst = s0 - s1)
extern void SubV2d(FVector2 *dst, FVector2 *s0, FVector2 *s1);
// ベクトルのコピー
extern void CopyV2d(FVector2 *dst, FVector2 *src);
// ベクトルのスケーリング
extern void ScaleV2d(FVector2 *out, FVector2 *in, REAL scale);
// ベクトルの値をセット
extern void SetV3d(FVector3 *dst, REAL x, REAL y, REAL z);
// ベクトルの加算(dst = s0 + s1)
extern void AddV3d(FVector3 *dst, FVector3 *s0, FVector3 *s1);
// ベクトルの減算(dst = s0 - s1)
extern void SubV3d(FVector3 *dst, FVector3 *s0, FVector3 *s1);
// ベクトルのコピー
extern void CopyV3d(FVector3 *dst, FVector3 *src);
// ベクトルのスケーリング
extern void ScaleV3d(FVector3 *out, FVector3 *in, REAL scale);
// 値をまとめて設定
extern void SetV4d(FVector4 *dst, REAL x, REAL y, REAL z, REAL w);

// 入力値が mix～max なら、そのまま値を返し、範囲外なら min or max を返す
extern int limit(int x, int min_value, int max_value);
// 指定値で切り上げ
extern int ceilingvalue(int a, int b);
// 切り上げて一番近いべき乗値を求める
extern int int2pow(int value);
// ビット数をカウントする
extern int countBitValue(u_int value);
// 一番下位のビットを求める
extern int getLowBitValue(u_int value);

// sin(r) のテーブル初期化
extern void SinTblInit(void);
// sin(r)を求める
extern int SinI(int r);
// cos(r)を求める
extern int CosI(int r);

// ウインドウズ用のファイル名を作成
extern void MakeWinFileName(char *dst, char *src);

// 値が一定範囲内か判定
extern BOOL MathIsRange(REAL val, REAL val1, REAL val2);
// 点とボックスの内包判定
extern BOOL MathBoxCrossPoint(int x, int y, sBox *box);
// 点と多角形の内包判定
extern BOOL MathPolygonCrossPoint(REAL x, REAL y, FVector2 *vtx);
// 線とボックスの内包判定
extern BOOL MathBoxCrossLine(sLine *line, sBox *box);
// ２つのボックスの交差判定
extern BOOL MathBoxCrossBox(sBox *box1, sBox *box2);
// 線分の交差判定
extern BOOL MathCheckCrossLine(sLine *l1, sLine *l2);
// ２直線の交点を求める
extern BOOL MathCrossLine(IVector2 *res, sLine *line1, sLine *line2);
// 点と線分の距離を求める
extern REAL MathDistancePointLine(IVector2 *pos, sLine *line);

// 座標の回転
extern void MathRotateXY(FVector2 *pos, REAL angle);
// ２点間の距離
extern REAL MathDistance(FVector2 *_x, FVector2 *_y);
// 単位ベクトルを求める
extern BOOL MathNormalize(FVector2 *res, FVector2 *vct);
// ベクトルをスカラ倍する
extern void MathScalar(FVector2 *vct, REAL scale);
// ベクトルのスカラ値を求める
extern REAL MathLength(FVector2 *vct);
// 内積を求める
extern REAL MathGetDotProduct(FVector2 *v1, FVector2 *v2);
// 線分の法線ベクトルを求める
extern void MathGetLineNormal(FVector2 *res, FVector2 *v1, FVector2 *v2);
// ベクトル同士の角度を求める
extern REAL MathVctAngle(FVector2 *v1, FVector2 *v2);
// ベクトル(0.0, 1.0)と任意ベクトルとの角度を求める
extern REAL MathVctAngleY(FVector2 *v1);
// ベクトル(1.0, 0.0)と任意ベクトルとの角度を求める
extern REAL MathVctAngleX(FVector2 *v1);
// 指定角度方向のベクトル計算
extern void MathCalcVector(FVector2 *vec, REAL angle, REAL length);

// 角度を－πから＋πの間に正規化
extern REAL NormalAngle(REAL ang);
// ２つの角の差を求める
extern REAL DifAngle(REAL a, REAL b);
// 角度を－180度から＋180度の間に正規化 (DEGREE)
extern REAL NormalAngleDeg(REAL ang);
// ２つの角の差を求める(DEGREE)
extern REAL DifAngleDeg(REAL a, REAL b);

// パラメータによる曲線生成
extern void MathSupershapes(FVector2 *res, REAL m, REAL n1, REAL n2, REAL n3, REAL phi);

// パターン番号からUVを求める
extern BOOL PatCalcUV(FVector2 *res, int pat, int tex_w, int tex_h, int pat_w, int pat_h);

// 簡易接触判定
extern BOOL MathCrossBox(FVector2 *src, float src_radius, FVector2 *dst, float dst_radius);


#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================
