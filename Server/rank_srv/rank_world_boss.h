#ifndef __RANK_WORLD_BOSS_H__
#define __RANK_WORLD_BOSS_H__
#include <stdio.h>
#include <stdint.h>
#include <set>
#include <map>
#include "comm_define.h"
#include "redis_util.h"


//#define MAX_WORD_BOSS_NUMM 10

#define MAX_WORLD_BOSS_NUM 100 //允许最大世界boss数
#define MAX_WORLD_BOSS_REALTINE_RANK_NUM 10 //世界boss实时排行前端最大显示数目
#define MAX_WORLD_BOSS_LASTROUND_RANK_NUM 5 //世界boss上轮排行前端最大显示数目
#define MAX_WORLD_BOSS_SHANGBANG_NUM 100 //世界boss最大上榜数
#define MAX_WORLD_BOSS_REWARD_ITEM_NUM 100 //世界boss奖励最大物品种类

extern std::set<uint64_t> world_boss_id;
extern char cur_world_boss_key[64]; //当前世界boss数据
extern char befor_world_boss_key[64]; //上轮世界boss数据
extern char tou_mu_world_boss_reward_num[64]; //头目类型世界boss玩家领奖信息
extern char shou_ling_world_boss_reward_num[64]; //首领类型世界boss玩家领奖信息
extern std::map<uint64_t, std::string> world_boss_rank_keys;
extern uint32_t tou_mu_parame_id; //头领限制领奖次数在参数表里面的id
extern uint32_t shou_ling_parame_id; //首领限制领奖次数在参数表里面的id
int init_rank_world_boss_id();
void init_world_boss_rank_key_map();
char *get_world_boss_rank_key(uint64_t rank_type);
int init_cur_world_boss_info();
//获取当前时间，单位、秒
uint64_t rank_srv_get_now_time();
//判断前后两个时间是否跨过某个时间点
//param1 前一个时间点 1970 年 1 月 1 日（00:00:00 GMT）以来的秒数
//param2 后一个时间点 1970 年 1 月 1 日（00:00:00 GMT）以来的秒数
//param3 - param4中间的是否跨过的时间点 年/月/日/时/分/秒 小于0表示用当前时间
//return 1:跨越了 0:没跨越 <0:出错
int judge_the_time_span(uint64_t befor_time, uint64_t behind_time, int year ,int month, int day, int hour, int min, int second);


//世界boss奖励表id
enum WorldBossRewardTableId
{
	WORLD_BOSS_FIRST_GEAR_REWARD = 510200001, //世界boss排行榜第1名奖励表id
	WORLD_BOSS_SECOND_GEAR_REWARD = 510200002, //世界boss排行榜第2名奖励表id
	WORLD_BOSS_THIRD_GEAR_REWARD = 510200003, //世界boss排行榜第3名奖励表id
	WORLD_BOSS_FOURTH_GEAR_REWARD = 510200004, //世界boss排行榜4-10名奖励表id
	WORLD_BOSS_FIFTH_GEAR_REWARD = 510200005, //世界boss排行榜11-30名奖励表id
	WORLD_BOSS_SIXTH_GEAR_REWARD = 510200006, //世界boss排行榜31-50名奖励表id
	WORLD_BOSS_SEVENTH_GEAR_REWARD = 510200007, //世界boss排行榜51-70名奖励表id
	WORLD_BOSS_EIGHTH_GEAR_REWARD = 510200008, //世界boss排行榜71-100名奖励表id
	WORLD_BOSS_NINTH_GEAR_REWARD = 510200999, //世界boss排行榜参与奖奖励表id
};
#endif
