#ifndef GLOBAL_PARAM_H
#define GLOBAL_PARAM_H
#include <map>
#include <vector>
#include <list>
#include <string>
#include <set>
#include <stdint.h>
#include "comm_define.h"
#include "mem_pool.h"
#include "minheap.h"

#define MAX_PARTNER_AI_INTERFACE 20
#define MAX_MONSTER_AI_INTERFACE 50
#define MAX_RAID_AI_INTERFACE 30

class Collect;
typedef std::map<uint32_t, Collect *> COLLECT_MAP;

class Team;
class cash_truck_struct;
typedef std::map<uint32_t, Team *> TEAM_MAP;
typedef std::map<uint64_t, int> ALL_ROLE_CONTAIN;
typedef std::set<uint64_t> ROLE_IN_TARGET;
typedef std::map<int, ROLE_IN_TARGET > TARGET_ROLE_CONTAIN;
typedef std::vector<uint64_t> TEAM_CONTAIN;

extern const char g_tmp_name[];

struct STChengJie
{
	uint32_t id;
	uint64_t pid; //target
	uint64_t investor;//
	uint32_t fail; //失败次数
	uint32_t shuangjin; //
	uint32_t exp; //
	uint32_t courage;
	uint64_t timeOut;
	uint64_t taskTime;
	uint64_t acceptCd;
	uint64_t step;
	bool complete;
	char declaration[256 + 1]; //悬赏留言
	bool anonymous;
	STChengJie() { memset(this, 0, sizeof(STChengJie)); }
}; 
struct ChengjieCd 
{
	uint32_t level;
	uint64_t cd;
};
typedef std::map<uint32_t, STChengJie> CHENGJIE_CONTAIN;
typedef std::vector<uint32_t> CHENGJIE_VECTOR;
typedef std::map<uint64_t, uint64_t> CHENGJIE_TARGET; //目标 接任务者
typedef std::map<uint64_t , ChengjieCd> CHENGJIE_ROLE_LEVEL;

enum UNIT_FIGHT_TYPE
{
	UNIT_FIGHT_TYPE_NONE,  //中立
	UNIT_FIGHT_TYPE_ENEMY,  //敌对
	UNIT_FIGHT_TYPE_FRIEND,  //友好
	UNIT_FIGHT_TYPE_MYSELF,  //自身
	UNIT_FIGHT_TYPE_ZHENYING,  //阵营
};
#define MAX_PK_TYPE 21
extern UNIT_FIGHT_TYPE pk_type_to_fight_type[MAX_PK_TYPE][MAX_PK_TYPE];

class ZhenyingBattle;
typedef std::map<uint64_t, ZhenyingBattle *> PRIVATE_BATTLE_T;

#endif /* GLOBAL_PARAM_H */
