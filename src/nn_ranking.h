//
// ƒ‰ƒ“ƒLƒ“ƒO‰æ–Ê
//

//==============================================================
#ifndef NN_RANKING_H
#define NN_RANKING_H
//==============================================================

#include "co_common.h"

#ifdef __cplusplus
extern              "C"
{
#endif

typedef struct {
	int score;
	int boss;
	int weapon;
} RankingInfo;


extern void RankingStart(BOOL playend, int score, int boss, int weapon);
extern void RankingTopScore(RankingInfo *res);


#ifdef __cplusplus
}
#endif

//==============================================================
#endif
//==============================================================
