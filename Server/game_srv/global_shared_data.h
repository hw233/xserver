#ifndef GLOBAL_SHARED_DATA_H
#define GLOBAL_SHARED_DATA_H

#include <stdint.h>

#define MAX_SERVER_LEVEL_REWARD_NUM 100
struct ServerLevelTable;
struct ServerLevelInfo
{
	uint32_t level_id;
	uint64_t break_reward[MAX_SERVER_LEVEL_REWARD_NUM];
	uint32_t break_goal;
	uint32_t break_num;
	bool     breaking;
	ServerLevelTable *config;
};

struct global_shared_data
{
	uint32_t g_team_id;
	ServerLevelInfo server_level;
};
extern struct global_shared_data *global_shared_data;
int init_global_shared_data(unsigned long key);
int resume_global_shared_data(unsigned long key);

#endif /* GLOBAL_SHARED_DATA_H */
