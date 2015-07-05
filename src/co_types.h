
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
	typedef float	REAL;						// ※実行環境によって変更する
#endif

#ifndef BOOL
	typedef int  	BOOL;
#endif

#if defined (PI)
    #undef PI
#endif
#define PI  ((REAL)3.1415925026e+00)		// 円周率


#define SLASH()	/

// 配列の要素数を取得
//--------------------
#define elemsof(a)  ((int)(sizeof(a) / sizeof((a)[0])))

// 構造体の要素の先頭からのオフセットを取得
//--------------------
#define FIELD_OFS(p_type, field)  ((unsigned int)&(((p_type)NULL)->field))

// メモリ初期化
//--------------------
#define	ZEROMEMORY(p, s)     memset((p), (0), (s))
#define	FILLMEMORY(p, s, v)  memset((p), (v), (s))

// 最大・最小
//--------------------
#ifndef min
#define min(x, y) (((x) > (y)) ? (y) : (x))
#endif

#ifndef max
#define max(x, y) (((x) < (y)) ? (y) : (x))
#endif

// TODO: sin()とか cos()とかの置換

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
	IVector2 start;								// 始点
	IVector2 end;								// 終点
} sLine;

typedef struct {
	IVector2 inf;								// min 値
	IVector2 sup;								// max 値
} sBox;

typedef struct {
	REAL red;
	REAL green;
	REAL blue;
	REAL alpha;
} sRGBA;

//------------
// RWG の名残
//------------
typedef char	 RwChar;							// 文字列
typedef char	 RwInt8;							// signed 8bit
typedef u_char	 RwUInt8;							// unsigned 8bit
typedef int		 RwInt32;							// signed 32bit
typedef u_int	 RwUInt32;							// unsigned 32bit
typedef BOOL	 RwBool;							// boolean
typedef REAL	 RwReal;							// 単精度実数
typedef FVector2 RwV2d;								// ベクトル
typedef FVector3 RwV3d;								// ベクトル
typedef FVector4 RwV4d;								// ベクトル

//==============================================================
#endif
//==============================================================
