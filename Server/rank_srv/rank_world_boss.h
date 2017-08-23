#ifndef __RANK_WORD_BOSS_H__
#define __RANK_WORD_BOSS_H__
#include <stdio.h>
#include <stdint.h>
#include <set>
#include "comm_define.h"

//#define MAX_WORD_BOSS_NUMM 10

struct word_boss_info{
	uint64_t id;
	char name[MAX_PLAYER_NAME_LEN + 1];    //最后一击玩家名字
};


extern std::set<uint64_t> world_boss_id;
int init_rank_world_boss_id();

#endif
