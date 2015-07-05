
//==============================================================
#ifndef CO_TYPES_H
#define CO_TYPES_H
//==============================================================

#ifndef TRUE
	#define TRUE  1
#endif

#ifndef FALSE
	#define FALSE  0
#endif

#ifndef REAL
	typedef float	REAL;						// �����s���ɂ���ĕύX����
#endif

#ifndef BOOL
	typedef int  	BOOL;
#endif

#if defined (PI)
    #undef PI
#endif
#define PI  ((REAL)3.1415925026e+00)		// �~����


#define SLASH()	/

// �z��̗v�f�����擾
//--------------------
#define elemsof(a)  ((int)(sizeof(a) / sizeof((a)[0])))

// �\���̗̂v�f�̐擪����̃I�t�Z�b�g���擾
//--------------------
#define FIELD_OFS(p_type, field)  ((unsigned int)&(((p_type)NULL)->field))

// ������������
//--------------------
#define	ZEROMEMORY(p, s)     memset((p), (0), (s))
#define	FILLMEMORY(p, s, v)  memset((p), (v), (s))

// �ő�E�ŏ�
//--------------------
#ifndef min
#define min(x, y) (((x) > (y)) ? (y) : (x))
#endif

#ifndef max
#define max(x, y) (((x) < (y)) ? (y) : (x))
#endif

// TODO: sin()�Ƃ� cos()�Ƃ��̒u��

typedef unsigned char   u_char;
typedef unsigned short  u_short;
typedef unsigned int    u_int;

typedef struct {
	int x, y;
} IVector2;

typedef struct {
	int x, y, z;
} IVector3;

typedef struct {
	int x, y, z, w;
} IVector4;

typedef struct {
	REAL x, y;
} FVector2;

typedef struct {
	REAL x, y, z;
} FVector3;

typedef struct {
	REAL x, y, z, w;
} FVector4;

typedef struct {
	IVector2 start;								// �n�_
	IVector2 end;								// �I�_
} sLine;

typedef struct {
	IVector2 inf;								// min �l
	IVector2 sup;								// max �l
} sBox;

typedef struct {
	REAL red;
	REAL green;
	REAL blue;
	REAL alpha;
} sRGBA;

//------------
// RWG �̖��c
//------------
typedef char	 RwChar;							// ������
typedef char	 RwInt8;							// signed 8bit
typedef u_char	 RwUInt8;							// unsigned 8bit
typedef int		 RwInt32;							// signed 32bit
typedef u_int	 RwUInt32;							// unsigned 32bit
typedef BOOL	 RwBool;							// boolean
typedef REAL	 RwReal;							// �P���x����
typedef FVector2 RwV2d;								// �x�N�g��
typedef FVector3 RwV3d;								// �x�N�g��
typedef FVector4 RwV4d;								// �x�N�g��

//==============================================================
#endif
//==============================================================
