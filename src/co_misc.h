
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
// �p�x�P�ʕϊ�
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


// �F�Z�b�g
extern void SetRGBA(sRGBA *dst, REAL r, REAL g, REAL b, REAL a);

// �x�N�g���̒l���Z�b�g
extern void SetV2d(FVector2 *dst, REAL x, REAL y);
// �x�N�g���̉��Z(dst = s0 + s1)
extern void AddV2d(FVector2 *dst, FVector2 *s0, FVector2 *s1);
// �x�N�g���̌��Z(dst = s0 - s1)
extern void SubV2d(FVector2 *dst, FVector2 *s0, FVector2 *s1);
// �x�N�g���̃R�s�[
extern void CopyV2d(FVector2 *dst, FVector2 *src);
// �x�N�g���̃X�P�[�����O
extern void ScaleV2d(FVector2 *out, FVector2 *in, REAL scale);
// �x�N�g���̒l���Z�b�g
extern void SetV3d(FVector3 *dst, REAL x, REAL y, REAL z);
// �x�N�g���̉��Z(dst = s0 + s1)
extern void AddV3d(FVector3 *dst, FVector3 *s0, FVector3 *s1);
// �x�N�g���̌��Z(dst = s0 - s1)
extern void SubV3d(FVector3 *dst, FVector3 *s0, FVector3 *s1);
// �x�N�g���̃R�s�[
extern void CopyV3d(FVector3 *dst, FVector3 *src);
// �x�N�g���̃X�P�[�����O
extern void ScaleV3d(FVector3 *out, FVector3 *in, REAL scale);
// �l���܂Ƃ߂Đݒ�
extern void SetV4d(FVector4 *dst, REAL x, REAL y, REAL z, REAL w);

// ���͒l�� mix�`max �Ȃ�A���̂܂ܒl��Ԃ��A�͈͊O�Ȃ� min or max ��Ԃ�
extern int limit(int x, int min_value, int max_value);
// �w��l�Ő؂�グ
extern int ceilingvalue(int a, int b);
// �؂�グ�Ĉ�ԋ߂��ׂ���l�����߂�
extern int int2pow(int value);
// �r�b�g�����J�E���g����
extern int countBitValue(u_int value);
// ��ԉ��ʂ̃r�b�g�����߂�
extern int getLowBitValue(u_int value);

// sin(r) �̃e�[�u��������
extern void SinTblInit(void);
// sin(r)�����߂�
extern int SinI(int r);
// cos(r)�����߂�
extern int CosI(int r);

// �E�C���h�E�Y�p�̃t�@�C�������쐬
extern void MakeWinFileName(char *dst, char *src);

// �l�����͈͓�������
extern BOOL MathIsRange(REAL val, REAL val1, REAL val2);
// �_�ƃ{�b�N�X�̓����
extern BOOL MathBoxCrossPoint(int x, int y, sBox *box);
// �_�Ƒ��p�`�̓����
extern BOOL MathPolygonCrossPoint(REAL x, REAL y, FVector2 *vtx);
// ���ƃ{�b�N�X�̓����
extern BOOL MathBoxCrossLine(sLine *line, sBox *box);
// �Q�̃{�b�N�X�̌�������
extern BOOL MathBoxCrossBox(sBox *box1, sBox *box2);
// �����̌�������
extern BOOL MathCheckCrossLine(sLine *l1, sLine *l2);
// �Q�����̌�_�����߂�
extern BOOL MathCrossLine(IVector2 *res, sLine *line1, sLine *line2);
// �_�Ɛ����̋��������߂�
extern REAL MathDistancePointLine(IVector2 *pos, sLine *line);

// ���W�̉�]
extern void MathRotateXY(FVector2 *pos, REAL angle);
// �Q�_�Ԃ̋���
extern REAL MathDistance(FVector2 *_x, FVector2 *_y);
// �P�ʃx�N�g�������߂�
extern BOOL MathNormalize(FVector2 *res, FVector2 *vct);
// �x�N�g�����X�J���{����
extern void MathScalar(FVector2 *vct, REAL scale);
// �x�N�g���̃X�J���l�����߂�
extern REAL MathLength(FVector2 *vct);
// ���ς����߂�
extern REAL MathGetDotProduct(FVector2 *v1, FVector2 *v2);
// �����̖@���x�N�g�������߂�
extern void MathGetLineNormal(FVector2 *res, FVector2 *v1, FVector2 *v2);
// �x�N�g�����m�̊p�x�����߂�
extern REAL MathVctAngle(FVector2 *v1, FVector2 *v2);
// �x�N�g��(0.0, 1.0)�ƔC�Ӄx�N�g���Ƃ̊p�x�����߂�
extern REAL MathVctAngleY(FVector2 *v1);
// �x�N�g��(1.0, 0.0)�ƔC�Ӄx�N�g���Ƃ̊p�x�����߂�
extern REAL MathVctAngleX(FVector2 *v1);
// �w��p�x�����̃x�N�g���v�Z
extern void MathCalcVector(FVector2 *vec, REAL angle, REAL length);

// �p�x���|�΂���{�΂̊Ԃɐ��K��
extern REAL NormalAngle(REAL ang);
// �Q�̊p�̍������߂�
extern REAL DifAngle(REAL a, REAL b);
// �p�x���|180�x����{180�x�̊Ԃɐ��K�� (DEGREE)
extern REAL NormalAngleDeg(REAL ang);
// �Q�̊p�̍������߂�(DEGREE)
extern REAL DifAngleDeg(REAL a, REAL b);

// �p�����[�^�ɂ��Ȑ�����
extern void MathSupershapes(FVector2 *res, REAL m, REAL n1, REAL n2, REAL n3, REAL phi);

// �p�^�[���ԍ�����UV�����߂�
extern BOOL PatCalcUV(FVector2 *res, int pat, int tex_w, int tex_h, int pat_w, int pat_h);

// �ȈՐڐG����
extern BOOL MathCrossBox(FVector2 *src, float src_radius, FVector2 *dst, float dst_radius);


#ifdef __cplusplus
}
#endif
//==============================================================
#endif
//==============================================================
