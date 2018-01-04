#ifndef GUILD_BATTLE_MANAGER_H
#define GUILD_BATTLE_MANAGER_H

#include <stdint.h>
#include "player.h"

#define GUILD_BATTLE_PLAYER_NUM (1)

class guild_wait_raid_struct;

//帮战预赛战斗结果
enum
{
	GBR_WIN = 1,
	GBR_LOSE = 2,
	GBR_DRAW = 3,
	GBR_AUTO_WIN = 4,
};

enum
{
	GBFLS_MISS = 0,
	GBFLS_GETTING = 1,
	GBFLS_GOTTEN = 2,
};

struct GuildBattleFightPlayerRewardInfo
{
	uint64_t player_id;
	uint64_t team_id;
	uint32_t result;
	uint32_t score;
	uint32_t treasure;
	uint32_t donation;
};

struct GuildBattleFightGuildRewardInfo
{
	uint32_t guild_id;
	std::vector<GuildBattleFightPlayerRewardInfo> players;
};

void get_rand_born_pos1(float *pos_x, float *pos_z, float *direct);
void get_rand_born_pos2(float *pos_x, float *pos_z, float *direct);

void get_final_rand_born_pos1(float *pos_x, float *pos_z, float *direct);
void get_final_rand_born_pos2(float *pos_x, float *pos_z, float *direct);
void get_final_rand_born_pos3(float *pos_x, float *pos_z, float *direct);
void get_final_rand_born_pos4(float *pos_x, float *pos_z, float *direct);

void guild_battle_manager_on_tick();
int add_to_guild_battle_waiting(player_struct *player);
int del_from_guild_battle_waiting(player_struct *player);  //玩家离开准备场景，下线

int start_guild_battle_match();
int start_final_guild_battle_match();

void print_waiting_player(struct evbuffer *buffer);
//int on_team_info_changed(player_struct *player);  队伍信息变更
// TODO: 加入队伍的时候要判断是否加入的是该场景的队伍和玩家是否是该场景的玩家
// TODO: 加入队伍的时候，要把人从单人队列移出
// TODO: 移出队伍的时候，要把人加入单人队列(如果在线)

//int guild_raid_on_player_add_team(player_struct *player, Team *team);
//int guild_raid_on_player_leave_team(player_struct *player, Team *team);

ProtoGuildInfo *get_guild_summary(uint32_t guild_id);

void broadcast_guild_battle_call_notify(std::vector<uint64_t> &playerIds, uint64_t caller_id = 0, char *caller_name = NULL);
void start_guild_battle_activity();
void start_final_guild_battle_activity();
bool is_guild_battle_opening(); //活动是否开启
bool is_guild_battle_settling(); //活动是否在结算时间
bool player_can_return_guild_battle(player_struct *player);
int player_can_participate_guild_battle(player_struct *player);
void check_guild_participate_num(uint32_t guild_id);
uint32_t set_guild_call_cd(uint32_t guild_id);
uint32_t get_guild_call_cd(uint32_t guild_id);
int pack_guild_wait_info(uint32_t guild_id, uint8_t *out_data);
char *get_guild_name(uint32_t guild_id);
uint32_t get_cur_round_end_time();
void get_guild_battle_fight_reward(player_struct *player, int result, uint32_t &score, uint32_t &treasure, uint32_t &donation, uint32_t kill_num = 0, uint32_t dead_num = 0, uint32_t monster_num = 0, uint32_t boss_damage = 0, uint32_t boss_num = 0);
void notify_guild_battle_fight_reward(player_struct *player, int result, uint32_t score, uint32_t treasure, uint32_t donation);
void send_guild_battle_fight_reward_to_guildsrv(std::map<uint32_t, GuildBattleFightGuildRewardInfo> &reward_map);
void insert_guild_battle_fight_reward_map(std::map<uint32_t, GuildBattleFightGuildRewardInfo> &reward_map, player_struct *player, int result, uint32_t kill_num = 0, uint32_t dead_num = 0, uint32_t monster_num = 0, uint32_t boss_damage = 0, uint32_t boss_num = 0);
void get_guild_raid_reward(raid_struct *raid, int win_team, std::map<uint32_t, GuildBattleFightGuildRewardInfo> &reward_map);
void get_guild_final_raid_reward(raid_struct *raid, std::map<uint32_t, GuildBattleFightGuildRewardInfo> &reward_map);
void set_final_guild_id(uint32_t *guild_id, uint32_t num);
void add_final_guild_id(uint32_t guild_id);
void init_guild_battle_manager();

#endif /* GUILD_BATTLE_MANAGER_H */
