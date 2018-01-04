#ifndef __ACTIVITY_UTIL_H__
#define __ACTIVITY_UTIL_H__

#include <stdint.h>
#include "server_proto.h"

#define MAX_TIME_LIMIT_ACTIVITY_NUM 50
#define MAX_ZHANLIDAREN_REWARD_NUM 10
#define MAX_SHIDAMENZONG_REWARD_NUM 10

extern uint32_t sg_server_id;

enum TimeLimitActivityType
{
	ACTIVITY_SHIDAMENZONG = 1, //十大门宗
	ACTIVITY_ZHANLIDAREN = 2, //战力达人
	ACTIVITY_END
};

enum TimeLimitActivityState
{
	TLAS_INIT = 1,
	TLAS_BEGIN = 2,
	TLAS_END = 8,
};

struct ZhanlidarenGift
{
	uint32_t id;
	uint32_t num;
};

struct ActivityInfo
{
	uint32_t activity_id;
	uint32_t begin_time;
	uint32_t end_time;
	uint32_t state; //活动状态
	union
	{
		struct
		{
			ZhanlidarenGift gift_get[MAX_ZHANLIDAREN_REWARD_NUM];
		} zhanlidaren;
		struct
		{
			uint32_t reward_time; //发奖时间，为0表示奖励发放成功，不为0每隔5分钟请求一次发奖
		} shidamenzong;
	} data;
};

struct ActivityDetail
{
	uint32_t activity_id;
	union
	{
		struct 
		{
			uint32_t reward[MAX_ZHANLIDAREN_REWARD_NUM];
		} zhanlidaren;
	} matter;
};

struct ActivityPlayer
{
	uint64_t player_id;
	ActivityDetail details[MAX_TIME_LIMIT_ACTIVITY_NUM];
};


void load_activity_db(void);
int save_activity_info(ActivityInfo *info);

ActivityInfo *get_activity_info(uint32_t activity_id);
ActivityPlayer *get_activity_player(uint64_t player_id);

int get_time_limit_activity_control(uint32_t type, uint32_t param1, uint32_t param2, uint32_t *begin_ts, uint32_t *end_ts);
int get_time_limit_activity_type(uint32_t activity_id);
int begin_activity(ActivityInfo *info);
int end_activity(ActivityInfo *info, uint32_t now);

void notify_activity_info_to_client(ActivityPlayer *player, EXTERN_DATA *extern_data);

int get_zhanlidaren_gift_reward_num(ActivityInfo *info, uint32_t gift_id);
int add_zhanlidaren_gift_reward_num(ActivityInfo *info, uint32_t gift_id);
int sub_zhanlidaren_gift_reward_num(ActivityInfo *info, uint32_t gift_id);
void broadcast_zhanlidaren_gift_num_change(ActivityInfo *info, uint32_t gift_id);

bool get_player_zhanlidaren_gift_is_get(ActivityPlayer *player, uint32_t activity_id, uint32_t gift_id); 
int mark_player_zhanlidaren_gift_get(ActivityPlayer *player, uint32_t activity_id, uint32_t gift_id);
int unmark_player_zhanlidaren_gift_get(ActivityPlayer *player, uint32_t activity_id, uint32_t gift_id);

#endif
