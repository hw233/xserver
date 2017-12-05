#include "guild_battle_manager.h"
#include "player_manager.h"
#include "msgid.h"
#include "uuid.h"
#include "raid_manager.h"
#include "../proto/pvp_raid.pb-c.h"
#include "../proto/raid.pb-c.h"
#include "../proto/scene_transfer.pb-c.h"
#include "../proto/guild_battle.pb-c.h"
#include "lua_config.h"
#include "team.h"
#include <set>
#include <vector>
#include <assert.h>
#include "guild_wait_raid.h"
#include "guild_wait_raid_manager.h"
#include "app_data_statis.h"
#include "error_code.h"
#include <math.h>
#include <algorithm>

#define GUILD_RAID_ID (sg_guild_raid_param1[0])
#define GUILD_RAID_FINAL_ID (sg_guild_raid_final_param1[0])

#define GUILD_BATTLE_PRELIMINARY_ACTIVITY_ID 330100024
#define GUILD_BATTLE_FINAL_ACTIVITY_ID 330100025

// static std::vector<player_struct *> guild_battle_manager_cant_matched_player;   //不能组成队伍的人
// static std::set<player_struct *> guild_battle_manager_waiting_player;   //个人
// static std::set<Team *> guild_battle_manager_waiting_team;  //队伍，保证队伍里面都是同一个工会
// static std::multimap<uint32_t, struct matched_team *> guild_battle_manager_matched_team; //组合好的队伍

// static uint32_t guild_battle_manager_action_tick = 0;
// static uint32_t guild_battle_manager_action_state = 0;
// static uint32_t guild_battle_manager_action_round = 0;
// static uint32_t guild_battle_manager_action_act = 0;

// static std::map<uint32_t, uint32_t> guild_battle_manager_guild_participate;
// static std::map<uint32_t, uint32_t> guild_battle_manager_guild_call_cd;
// static std::set<uint32_t> guild_battle_manager_final_guild_id; //参加决赛的帮会

extern std::map<uint32_t, ProtoGuildInfo> guild_summary_map;

extern std::vector<player_struct *> guild_battle_manager_cant_matched_player;   //不能组成队伍的人
extern std::set<player_struct *> guild_battle_manager_waiting_player;   //个人
extern std::set<Team *> guild_battle_manager_waiting_team;  //队伍，保证队伍里面都是同一个工会
extern std::multimap<uint32_t, struct matched_team *> guild_battle_manager_matched_team; //组合好的队伍

extern uint32_t guild_battle_manager_activity_start_ts;
extern uint32_t guild_battle_manager_action_tick;
extern uint32_t guild_battle_manager_action_state;
extern uint32_t guild_battle_manager_action_round;
extern uint32_t guild_battle_manager_action_act;

extern std::map<uint32_t, uint32_t> guild_battle_manager_guild_participate;
extern std::map<uint32_t, uint32_t> guild_battle_manager_guild_call_cd;
extern std::set<uint32_t> guild_battle_manager_final_guild_id; //参加决赛的帮会

extern uint32_t guild_battle_manager_final_list_state;
extern uint32_t guild_battle_manager_final_list_tick;

extern std::set<uint64_t> guild_battle_manager_participate_players; //参加帮会战的玩家

enum
{
	GBS_BEGIN = 1,
	GBS_WAIT = 2,
	GBS_BATTLE = 3,
	GBS_SETTLE = 4,
};

uint32_t get_wait_time();
void cant_matched_player_round_finished();
void on_guild_battle_raid_time_out(); //时间到，副本结算
bool guild_battle_is_final();
static void check_guild_wait_award();
static void get_guild_battle_next_activity_begin_time(uint32_t &act_id, uint32_t &begin_time);
static uint32_t calcu_player_final_score(uint32_t kill_num, uint32_t monster_num, uint32_t boss_damage, uint32_t boss_num);
static int start_guild_battle(struct matched_team *team1, struct matched_team *team2);
static int start_final_guild_battle(struct matched_team *team1, struct matched_team *team2, struct matched_team *team3, struct matched_team *team4);
struct matched_team
{
	uint32_t yongwu; //勇武值
	player_struct *team_player[GUILD_BATTLE_PLAYER_NUM];	
};

static bool do_not_remove_team_member = false;

int del_from_guild_battle_waiting(player_struct *player)
{
	guild_battle_manager_waiting_player.erase(player);

	if (!do_not_remove_team_member && player->m_team)// && guild_battle_manager_waiting_team.find(player->m_team) != guild_battle_manager_waiting_team.end())
	{
		player->m_team->RemoveMember(*player);
		// if (player->m_team->GetMemberSize() == 0)
		// {
		// 	guild_battle_manager_waiting_team.erase(player->m_team);
		// }
	}
	return (0);
}

int add_to_guild_battle_waiting(player_struct *player)
{
	if (player->data->guild_id == 0)
	{
		LOG_ERR("%s: player[%lu] do not in guild", __FUNCTION__, player->get_uuid());
		return (-1);
	}

		// 检查是否已经加入了队列

//	if (!player->m_team)
	{
		guild_battle_manager_waiting_player.insert(player);
		player->guild_battle_wait_award_time = time_helper::get_cached_time() / 1000;
		return (0);
	}

		// 检查是否都是一个工会的
	// for (int i = 0; i < player->m_team->GetMemberSize(); ++i)
	// {
	// 	player_struct *t_player = player_manager::get_player_by_id(player->m_team->m_data->m_mem[i].id);
	// 	if (!t_player || !t_player->is_online())
	// 	{
	// 		LOG_ERR("%s: player[%lu] not online", __FUNCTION__, player->m_team->m_data->m_mem[i].id);
	// 		return (-10);
	// 	}
	// 	if (t_player->data->guild_id != player->data->guild_id)
	// 	{
	// 		LOG_ERR("%s: player[%lu] guild not same", __FUNCTION__, player->m_team->m_data->m_mem[i].id);
	// 		return (-20);			
	// 	}
	// }

	// 	// TODO: 是否需要检查队长?
	// guild_battle_manager_waiting_team.insert(player->m_team);
	return (0);
}

// int guild_raid_on_player_add_team(player_struct *player, Team *team)
// {
// 	assert(guild_battle_manager_waiting_team.find(team) != guild_battle_manager_waiting_team.end());
// 	assert(guild_battle_manager_waiting_player.find(player) != guild_battle_manager_waiting_player.end());
// 	guild_battle_manager_waiting_player.erase(player);
// 	return (0);
// }

// int guild_raid_on_player_leave_team(player_struct *player, Team *team)
// {
// 	assert(guild_battle_manager_waiting_team.find(team) != guild_battle_manager_waiting_team.end());
// 	assert(guild_battle_manager_waiting_player.find(player) == guild_battle_manager_waiting_player.end());
// 	if (player->is_online())
// 		guild_battle_manager_waiting_player.insert(player);	
// 	return (0);	
// }

static void clean_choosed_ite(std::set<Team *>::iterator *t1, int n_choosed_guild_ite, std::set<player_struct *>::iterator *t2, int n_choosed_single_ite)
{
	for (int i = 0; i < n_choosed_guild_ite; ++i)
	{
		guild_battle_manager_waiting_team.erase(t1[i]);
	}
	for (int i = 0; i < n_choosed_single_ite; ++i)
	{
		guild_battle_manager_waiting_player.erase(t2[i]);
	}
}

static void add_choosed_to_cantmatch(std::set<Team *>::iterator *t1, int n_choosed_guild_ite, std::set<player_struct *>::iterator *t2, int n_choosed_single_ite)
{
	for (int i = 0; i < n_choosed_guild_ite; ++i)
	{
		Team *team = *(t1[i]);
		for (int i = 0; i < team->GetMemberSize(); ++i)
		{
			player_struct *t_player = player_manager::get_player_by_id(team->m_data->m_mem[i].id);
			if (!t_player || !t_player->is_online())
			{
				LOG_ERR("%s: player[%lu] not online", __FUNCTION__, team->m_data->m_mem[i].id);
				continue;
			}
			guild_battle_manager_cant_matched_player.push_back(t_player);
		}
	}
	for (int i = 0; i < n_choosed_single_ite; ++i)
	{
		guild_battle_manager_cant_matched_player.push_back(*(t2[i]));
	}
}

static void add_matched_team(std::set<Team *>::iterator *t1, int n_choosed_guild_ite, std::set<player_struct *>::iterator *t2, int n_choosed_single_ite)
{
	struct matched_team *t = (struct matched_team *)malloc(sizeof(struct matched_team));
	if (!t)
	{
		LOG_ERR("%s: malloc failed", __FUNCTION__);
		return;
	}

	LOG_INFO("%s: t = %p", __FUNCTION__, t);
	
	int index = 0;

	for (int i = 0; i < n_choosed_guild_ite; ++i)
	{
		Team *team = *(t1[i]);
		assert(team);
		for (int i = 0; i < team->GetMemberSize(); ++i)
		{
			player_struct *player = player_manager::get_player_by_id(team->m_data->m_mem[i].id);
			assert(player);
			t->team_player[index] = player;
				// 勇武值
			t->yongwu += player->get_attr(PLAYER_ATTR_BRAVE);
			++index;
		}
	}

	for (int i = 0; i < n_choosed_single_ite; ++i)
	{
		player_struct *player = *(t2[i]);
		t->team_player[index] = player;
			// 勇武值
		t->yongwu += player->get_attr(PLAYER_ATTR_BRAVE);
		++index;
	}
	
	guild_battle_manager_matched_team.insert(std::make_pair(t->yongwu, t));
	clean_choosed_ite(t1, n_choosed_guild_ite, t2, n_choosed_single_ite);
}

static void move_waiting_player_to_team()
{
	std::set<Team *>::iterator choosed_guild_ite[GUILD_BATTLE_PLAYER_NUM];
	std::set<player_struct *>::iterator choosed_single_ite[GUILD_BATTLE_PLAYER_NUM];	
	int n_choosed_guild_ite = 0;
	int n_choosed_single_ite = 0;	
	int player_num = 0;

	uint64_t guild_id = 0;
	for (std::set<Team *>::iterator ite = guild_battle_manager_waiting_team.begin(); ite != guild_battle_manager_waiting_team.end(); ++ite)
	{
		Team *team = (*ite);
		// if (!team)
		// {
		// 	LOG_ERR("%s: can not find team[%lu] bug?", __FUNCTION__, *ite);
		// 	continue;
		// }
		if (guild_id == 0)
		{
			player_struct *leader = team->GetLead();
			if (!leader)
			{
				LOG_ERR("%s: can not find team leader[%p] bug?", __FUNCTION__, *ite);
				continue;			
			}
			guild_id = leader->data->guild_id;
			if (guild_id == 0)
			{
				LOG_ERR("%s: can not find guild[%p] bug?", __FUNCTION__, *ite);
				continue;							
			}
		}
		else
		{
			player_struct *leader = team->GetLead();
			if (!leader)
			{
				LOG_ERR("%s: can not find team leader[%p] bug?", __FUNCTION__, *ite);
				continue;			
			}
			uint64_t cur_guild_id = leader->data->guild_id;
			if (cur_guild_id == 0)
			{
				LOG_ERR("%s: can not find guild[%p] bug?", __FUNCTION__, *ite);
				continue;							
			}
			if (cur_guild_id != guild_id)
				continue;
		}

		int mem_size = team->GetMemberSize(); 
		if (mem_size + player_num > GUILD_BATTLE_PLAYER_NUM)
			continue;
		
		choosed_guild_ite[n_choosed_guild_ite] = ite;
		++n_choosed_guild_ite;
		player_num += mem_size;

		if (player_num == GUILD_BATTLE_PLAYER_NUM)
		{
				// 匹配到一个队伍
			add_matched_team(choosed_guild_ite, n_choosed_guild_ite, choosed_single_ite, n_choosed_single_ite);
			return;
		}
	}

	for (std::set<player_struct *>::iterator ite = guild_battle_manager_waiting_player.begin(); ite != guild_battle_manager_waiting_player.end(); ++ite)
	{
		if (guild_id == 0)
		{
			guild_id = (*ite)->data->guild_id;
			if (guild_id == 0)
			{
				LOG_ERR("%s: can not find guild[%lu] bug?", __FUNCTION__, (*ite)->get_uuid());
				continue;							
			}
		}
		else
		{
			uint64_t cur_guild_id = (*ite)->data->guild_id;
			if (cur_guild_id == 0)
			{
				LOG_ERR("%s: can not find guild[%lu] bug?", __FUNCTION__, (*ite)->get_uuid());
				continue;											
			}
			if (cur_guild_id != guild_id)
				continue;			
		}

		choosed_single_ite[n_choosed_single_ite] = ite;
		++n_choosed_single_ite;
		++player_num;
		if (player_num == GUILD_BATTLE_PLAYER_NUM)
		{
				// 匹配到一个队伍
			add_matched_team(choosed_guild_ite, n_choosed_guild_ite, choosed_single_ite, n_choosed_single_ite);
			return;			
		}
	}

		//没有匹配到，排除掉第一个
	if (n_choosed_guild_ite)
	{
		add_choosed_to_cantmatch(choosed_guild_ite, 1, NULL, 0);
		clean_choosed_ite(choosed_guild_ite, 1, NULL, 0);
	}
	else
	{
		assert(n_choosed_single_ite > 0);
		add_choosed_to_cantmatch(NULL, 0, choosed_single_ite, 1);		
		clean_choosed_ite(NULL, 0, choosed_single_ite, 1);		
	}
}

static Team *create_team_from_matched_team(struct matched_team *team)
{
	if (!team)
		return NULL;
	for (int i = 0; i < GUILD_BATTLE_PLAYER_NUM; ++i)
	{
		if (team->team_player[i]->m_team)
		{
			Team::DestroyTeam(team->team_player[i]->m_team);
		}
	}
	
	Team *t = Team::CreateTeam(team->team_player, GUILD_BATTLE_PLAYER_NUM);
	return t;
}

static void try_match_guild_battle()
{
	guild_battle_manager_matched_team.clear();
	guild_battle_manager_cant_matched_player.clear();
	
//	guild_battle_manager_matched_team = last_matched_team;
//	last_matched_team.clear();
		//首先组成各种队伍
	while (!guild_battle_manager_waiting_player.empty() || !guild_battle_manager_waiting_team.empty())
	{
		move_waiting_player_to_team();
	}

		//接下来两两匹配
	while (!guild_battle_manager_matched_team.empty())
	{
		std::multimap<uint32_t, struct matched_team *>::iterator ite = guild_battle_manager_matched_team.begin();		
		uint32_t guild_id = ite->second->team_player[0]->data->guild_id;
		assert(guild_id > 0);
		std::multimap<uint32_t, struct matched_team *>::iterator cur_ite = ite;
		++cur_ite;
		bool start_battle = false;
		for (; cur_ite != guild_battle_manager_matched_team.end(); ++cur_ite)
		{
			uint32_t cur_guild_id = cur_ite->second->team_player[0]->data->guild_id;
			if (cur_guild_id == guild_id)
				continue;
				// 匹配到了
			start_guild_battle(ite->second, cur_ite->second);
			start_battle = true;

			LOG_INFO("%s: free_matched_team = %p %p", __FUNCTION__, cur_ite->second, ite->second);
			free(cur_ite->second);
			guild_battle_manager_matched_team.erase(cur_ite);
			free(ite->second);
			guild_battle_manager_matched_team.erase(ite);
			break;
		}

		if (!start_battle)
		{
			// 没有匹配到，删掉ite再来，加入cant_matched_player
			LOG_INFO("%s: %p not match, free_matched_team", __FUNCTION__, ite->second);
			struct matched_team *team = ite->second;
			for (int i = 0; i < GUILD_BATTLE_PLAYER_NUM; ++i)
			{
				guild_battle_manager_cant_matched_player.push_back(team->team_player[i]);
			}
			free(team);			
			guild_battle_manager_matched_team.erase(ite);
		}
	}
}

static void try_match_final_guild_battle()
{
	guild_battle_manager_matched_team.clear();
//	guild_battle_manager_matched_team = last_matched_team;
//	last_matched_team.clear();
		//首先组成各种队伍
	while (!guild_battle_manager_waiting_player.empty() || !guild_battle_manager_waiting_team.empty())
	{
		move_waiting_player_to_team();
	}

		//接下来4个一起匹配
	while (!guild_battle_manager_matched_team.empty())
	{
		std::multimap<uint32_t, struct matched_team *>::iterator ite[4];		
		uint32_t guild_id[4];
		int found_team_num = 1;
		
		ite[0] = guild_battle_manager_matched_team.begin();
		guild_id[0] = ite[0]->second->team_player[0]->data->guild_id;
		assert(guild_id[0] > 0);
		
		std::multimap<uint32_t, struct matched_team *>::iterator cur_ite = ite[0];
		++cur_ite;
		bool start_battle = false;
		for (; cur_ite != guild_battle_manager_matched_team.end(); ++cur_ite)
		{
			uint32_t cur_guild_id = cur_ite->second->team_player[0]->data->guild_id;
			bool same_team = false;
			for (int i = 0; i < found_team_num; ++i)
			{
				if (cur_guild_id == guild_id[i])
				{
					same_team = true;
					break;
				}
			}
			if (same_team)
				continue;
			
				// 匹配到了
			ite[found_team_num] = cur_ite;
			guild_id[found_team_num] = cur_guild_id;
			++found_team_num;

				//足够4个队伍了
			if (found_team_num == 4)
			{
				// start_final_guild_battle(ite[0]->second, ite[1]->second, ite[2]->second, ite[3]->second);

				// LOG_INFO("%s: free_matched_team = %p %p %p %p", __FUNCTION__, ite[0]->second, ite[1]->second, ite[2]->second, ite[3]->second);
				
				// start_battle = true;				

				// for (int i = 0; i < found_team_num; ++i)
				// {
				// 	free(ite[i]->second);
				// 	guild_battle_manager_matched_team.erase(ite[i]);
				// }
				break;				
			}
		}

			//如果够两个队伍，那也开始
		if (found_team_num >= 2)
		{
			struct matched_team *team[4];
			for (int i = 0; i < 4; ++i)
				team[i] = NULL;
			for (int i = 0; i < found_team_num; ++i)
			{
				team[i] = ite[i]->second;
			}
			
			start_final_guild_battle(team[0], team[1], team[2], team[3]);
			LOG_INFO("%s: free_matched_team = %p %p %p %p", __FUNCTION__, team[0], team[1], team[2], team[3]);
				
			start_battle = true;				

			for (int i = 0; i < found_team_num; ++i)
			{
				free(ite[i]->second);
				guild_battle_manager_matched_team.erase(ite[i]);
			}
		}
		

		if (!start_battle)
		{
			// 没有匹配到，删掉ite再来，重新加回队列
			// 那肯定只有一个队伍
			assert(found_team_num == 1);
			LOG_INFO("%s: %p not match, free_matched_team", __FUNCTION__, ite[0]->second);
			struct matched_team *team = ite[0]->second;
			for (int i = 0; i < GUILD_BATTLE_PLAYER_NUM; ++i)
			{
				guild_battle_manager_cant_matched_player.push_back(team->team_player[i]);
			}
			free(team);			
			guild_battle_manager_matched_team.erase(ite[0]);
		}
	}
}

static int create_guild_battle_team(raid_struct *raid, struct matched_team *team)
{
//	player_struct *player[GUILD_BATTLE_PLAYER_NUM];
//	player[0] = team->team_player[0];
	if (!team)
		return 0;

	Team *t = create_team_from_matched_team(team);
	if (t == NULL)
	{
		return -1;
	}
//	raid->data->delete_team1 = true;
	raid->data->team_id = t->GetId();
	raid->m_raid_team  = t;
	t->m_data->m_raid_uuid = raid->data->uuid;
	return (0);
}

static int create_guild_battle_team2(raid_struct *raid, struct matched_team *team)
{
//	player_struct *player[GUILD_BATTLE_PLAYER_NUM];
//	player[0] = team->team_player[0];
	if (!team)
		return 0;

	Team *t = create_team_from_matched_team(team);
//	raid->data->delete_team1 = true;
	raid->data->team2_id = t->GetId();
	raid->m_raid_team2  = t;
	t->m_data->m_raid_uuid = raid->data->uuid;
	return (0);
}

static int create_guild_battle_team3(raid_struct *raid, struct matched_team *team)
{
//	player_struct *player[GUILD_BATTLE_PLAYER_NUM];
//	player[0] = team->team_player[0];
	if (!team)
		return 0;

	Team *t = create_team_from_matched_team(team);
//	raid->data->delete_team1 = true;
	raid->data->team3_id = t->GetId();
	raid->m_raid_team3  = t;
	t->m_data->m_raid_uuid = raid->data->uuid;
	return (0);
}

static int create_guild_battle_team4(raid_struct *raid, struct matched_team *team)
{
//	player_struct *player[GUILD_BATTLE_PLAYER_NUM];
//	player[0] = team->team_player[0];
	if (!team)
		return 0;

	Team *t = create_team_from_matched_team(team);
//	raid->data->delete_team1 = true;
	raid->data->team4_id = t->GetId();
	raid->m_raid_team4  = t;
	t->m_data->m_raid_uuid = raid->data->uuid;
	return (0);
}


void get_rand_born_pos1(float *pos_x, float *pos_z, float *direct)
{
	float rand1 = (random() % (sg_guild_raid_param1[5] * 2) - sg_guild_raid_param1[5]) / 100.0;
	*pos_x = sg_guild_raid_param1[1] + rand1;
	rand1 = (random() % (sg_guild_raid_param1[5] * 2) - sg_guild_raid_param1[5]) / 100.0;		
	*pos_z = sg_guild_raid_param1[3] + rand1;
	*direct = sg_guild_raid_param1[4];
}
void get_rand_born_pos2(float *pos_x, float *pos_z, float *direct)
{
	float rand1 = (random() % (sg_guild_raid_param2[5] * 2) - sg_guild_raid_param2[5]) / 100.0;
	*pos_x = sg_guild_raid_param2[1] + rand1;
	rand1 = (random() % (sg_guild_raid_param2[5] * 2) - sg_guild_raid_param2[5]) / 100.0;		
	*pos_z = sg_guild_raid_param2[3] + rand1;
	*direct = sg_guild_raid_param2[4];
}

void get_final_rand_born_pos1(float *pos_x, float *pos_z, float *direct)
{
	float rand1 = (random() % (sg_guild_raid_final_param1[5] * 2) - sg_guild_raid_final_param1[5]) / 100.0;
	*pos_x = sg_guild_raid_final_param1[1] + rand1;
	rand1 = (random() % (sg_guild_raid_final_param1[5] * 2) - sg_guild_raid_final_param1[5]) / 100.0;		
	*pos_z = sg_guild_raid_final_param1[3] + rand1;
	*direct = sg_guild_raid_final_param1[4];
}
void get_final_rand_born_pos2(float *pos_x, float *pos_z, float *direct)
{
	float rand1 = (random() % (sg_guild_raid_final_param2[5] * 2) - sg_guild_raid_final_param2[5]) / 100.0;
	*pos_x = sg_guild_raid_final_param2[1] + rand1;
	rand1 = (random() % (sg_guild_raid_final_param2[5] * 2) - sg_guild_raid_final_param2[5]) / 100.0;		
	*pos_z = sg_guild_raid_final_param2[3] + rand1;
	*direct = sg_guild_raid_final_param2[4];
}
void get_final_rand_born_pos3(float *pos_x, float *pos_z, float *direct)
{
	float rand1 = (random() % (sg_guild_raid_final_param3[5] * 2) - sg_guild_raid_final_param3[5]) / 100.0;
	*pos_x = sg_guild_raid_final_param3[1] + rand1;
	rand1 = (random() % (sg_guild_raid_final_param3[5] * 2) - sg_guild_raid_final_param3[5]) / 100.0;		
	*pos_z = sg_guild_raid_final_param3[3] + rand1;
	*direct = sg_guild_raid_final_param3[4];
}
void get_final_rand_born_pos4(float *pos_x, float *pos_z, float *direct)
{
	float rand1 = (random() % (sg_guild_raid_final_param4[5] * 2) - sg_guild_raid_final_param4[5]) / 100.0;
	*pos_x = sg_guild_raid_final_param4[1] + rand1;
	rand1 = (random() % (sg_guild_raid_final_param4[5] * 2) - sg_guild_raid_final_param4[5]) / 100.0;		
	*pos_z = sg_guild_raid_final_param4[3] + rand1;
	*direct = sg_guild_raid_final_param4[4];
}


static int start_guild_battle(struct matched_team *team1, struct matched_team *team2)
{
	assert(team1);
	assert(team2);	
	
	raid_struct *raid = raid_manager::create_raid(GUILD_RAID_ID, NULL);
	if (!raid)
	{
		LOG_ERR("%s: create raid failed", __FUNCTION__);
		return (-10);
	}

	do_not_remove_team_member = true;	

	raid->GUILD_DATA.guild_id[0] = team1->team_player[0]->data->guild_id;
	raid->GUILD_DATA.guild_id[1] = team2->team_player[0]->data->guild_id;

	create_guild_battle_team(raid, team1);
	create_guild_battle_team2(raid, team2);	

	player_struct *player;
//	EXTERN_DATA extern_data;
//	EnterRaidNotify notify;
//	SceneTransferAnswer resp;
//	enter_raid_notify__init(&notify);
//	notify.raid_id = GUILD_RAID_ID;
	for (int i = 0; i < GUILD_BATTLE_PLAYER_NUM; ++i)
	{
		player = team1->team_player[i];
		assert(player);

		// raid_struct *t_raid = player->get_raid();
		// if (t_raid)
		// {
		// 	player->set_out_raid_pos_and_clear_scene();
		// 	t_raid->delete_player_from_scene(player);
		// 	t_raid->on_player_leave_raid(player);
		// }
		
// 		raid->m_player[i] = player;
// //		memset(&raid->data->player_info[i], 0, sizeof(raid->data->player_info[i]));
// //		raid->data->player_info[i].player_id = player->get_uuid();

// 		raid->set_player_info(player, &raid->data->player_info[i]);

// 		player->set_enter_raid_pos_and_scene(raid);

// 		// float rand1 = (random() % (sg_3v3_pvp_raid_param1[5] * 2) - sg_3v3_pvp_raid_param1[5]) / 100.0;
// 		// float pos_x = sg_3v3_pvp_raid_param1[1] + rand1;
// 		// rand1 = (random() % (sg_3v3_pvp_raid_param1[5] * 2) - sg_3v3_pvp_raid_param1[5]) / 100.0;		
// 		// float pos_z = sg_3v3_pvp_raid_param1[3] + rand1;
 		float pos_x, pos_z, direct;
 		get_rand_born_pos1(&pos_x, &pos_z, &direct);
// 		player->set_pos(pos_x, pos_z);

// 		extern_data.player_id = player->get_uuid();
// 		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ENTER_RAID_NOTIFY, enter_raid_notify__pack, notify);

// 		player->send_scene_transfer(sg_guild_raid_param1[4], pos_x, sg_guild_raid_param1[2],
// 			pos_z, GUILD_RAID_ID, 0);

// 		raid->on_player_enter_raid(player);

		raid->player_enter_raid_impl(player, i, pos_x, pos_z, direct);

		player = team2->team_player[i];
		assert(player);
		// t_raid = player->get_raid();
		// if (t_raid)
		// {
		// 	player->set_out_raid_pos_and_clear_scene();
		// 	t_raid->delete_player_from_scene(player);
		// 	t_raid->on_player_leave_raid(player);
		// }
		
// 		raid->m_player2[i] = player;
// //		memset(&raid->data->player_info2[i], 0, sizeof(raid->data->player_info[i]));
// //		raid->data->player_info2[i].player_id = player->get_uuid();
// 		raid->set_player_info(player, &raid->data->player_info2[i]);
		
// 		player->set_enter_raid_pos_and_scene(raid);

 		get_rand_born_pos2(&pos_x, &pos_z, &direct);
// 		player->set_pos(pos_x, pos_z);		
// //		player->set_pos(sg_3v3_pvp_raid_param2[1], sg_3v3_pvp_raid_param2[3]);

// 		extern_data.player_id = player->get_uuid();
// 		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ENTER_RAID_NOTIFY, enter_raid_notify__pack, notify);

// 		player->send_scene_transfer(sg_guild_raid_param2[4], pos_x, sg_guild_raid_param2[2],
// 			pos_z, GUILD_RAID_ID, 0);

// 		raid->on_player_enter_raid(player);
		raid->player_enter_raid_impl(player, i + MAX_TEAM_MEM, pos_x, pos_z, direct);		
	}

	do_not_remove_team_member = false;	

	return (0);
}

static int start_final_guild_battle(struct matched_team *team1, struct matched_team *team2, struct matched_team *team3, struct matched_team *team4)
{
		//至少两个队伍
	assert(team1);
	assert(team2);
	
	raid_struct *raid = raid_manager::create_raid(GUILD_RAID_FINAL_ID, NULL);
	if (!raid)
	{
		LOG_ERR("%s: create raid failed", __FUNCTION__);
		return (-10);
	}

	raid->GUILD_FINAL_DATA.guild_id[0] = team1->team_player[0]->data->guild_id;
	raid->GUILD_FINAL_DATA.guild_id[1] = team2->team_player[0]->data->guild_id;
	if (team3)
	{
		raid->GUILD_FINAL_DATA.guild_id[2] = team3->team_player[0]->data->guild_id;		
	}
	if (team4)
	{
		raid->GUILD_FINAL_DATA.guild_id[3] = team4->team_player[0]->data->guild_id;		
	}

	player_struct *player;
//	EXTERN_DATA extern_data;
//	EnterRaidNotify notify;
//	SceneTransferAnswer resp;
//	enter_raid_notify__init(&notify);
//	notify.raid_id = GUILD_RAID_FINAL_ID;
	for (int i = 0; i < GUILD_BATTLE_PLAYER_NUM; ++i)
	{
		player = team1->team_player[i];
		assert(player);

		raid_struct *t_raid = player->get_raid();
		if (t_raid)
		{
			player->set_out_raid_pos_and_clear_scene();
			t_raid->delete_player_from_scene(player);
			t_raid->on_player_leave_raid(player);
		}
		
// 		raid->m_player[i] = player;
// //		memset(&raid->data->player_info[i], 0, sizeof(raid->data->player_info[i]));
// //		raid->data->player_info[i].player_id = player->get_uuid();
// 		raid->set_player_info(player, &raid->data->player_info[i]);		
// 		player->set_enter_raid_pos_and_scene(raid);

// 		// float rand1 = (random() % (sg_3v3_pvp_raid_param1[5] * 2) - sg_3v3_pvp_raid_param1[5]) / 100.0;
// 		// float pos_x = sg_3v3_pvp_raid_param1[1] + rand1;
// 		// rand1 = (random() % (sg_3v3_pvp_raid_param1[5] * 2) - sg_3v3_pvp_raid_param1[5]) / 100.0;		
// 		// float pos_z = sg_3v3_pvp_raid_param1[3] + rand1;
 		float pos_x, pos_z, direct;
 		get_final_rand_born_pos1(&pos_x, &pos_z, &direct);
// 		player->set_pos(pos_x, pos_z);

// 		extern_data.player_id = player->get_uuid();
// 		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ENTER_RAID_NOTIFY, enter_raid_notify__pack, notify);

// 		player->send_scene_transfer(sg_guild_raid_final_param1[4], pos_x, sg_guild_raid_param1[2],
// 			pos_z, GUILD_RAID_FINAL_ID, 0);

// 		raid->on_player_enter_raid(player);
		raid->player_enter_raid_impl(player, i, pos_x, pos_z, direct);

		player = team2->team_player[i];
		assert(player);

		t_raid = player->get_raid();
		if (t_raid)
		{
			player->set_out_raid_pos_and_clear_scene();
			t_raid->delete_player_from_scene(player);
			t_raid->on_player_leave_raid(player);
		}
		
// 		raid->m_player2[i] = player;
// //		memset(&raid->data->player_info2[i], 0, sizeof(raid->data->player_info[i]));
// //		raid->data->player_info2[i].player_id = player->get_uuid();
// 		raid->set_player_info(player, &raid->data->player_info2[i]);		
// 		player->set_enter_raid_pos_and_scene(raid);

 		get_final_rand_born_pos2(&pos_x, &pos_z, &direct);
// 		player->set_pos(pos_x, pos_z);		
// //		player->set_pos(sg_3v3_pvp_raid_param2[1], sg_3v3_pvp_raid_param2[3]);

// 		extern_data.player_id = player->get_uuid();
// 		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ENTER_RAID_NOTIFY, enter_raid_notify__pack, notify);

// 		player->send_scene_transfer(sg_guild_raid_final_param2[4], pos_x, sg_guild_raid_final_param2[2],
// 			pos_z, GUILD_RAID_FINAL_ID, 0);

// 		raid->on_player_enter_raid(player);
		raid->player_enter_raid_impl(player, i + MAX_TEAM_MEM, pos_x, pos_z, direct);		
	
		if (team3)
		{
			player = team3->team_player[i];
			assert(player);

			t_raid = player->get_raid();
			if (t_raid)
			{
				player->set_out_raid_pos_and_clear_scene();
				t_raid->delete_player_from_scene(player);
				t_raid->on_player_leave_raid(player);
			}
		
// 			raid->m_player3[i] = player;
// //		memset(&raid->data->player_info2[i], 0, sizeof(raid->data->player_info[i]));
// //		raid->data->player_info2[i].player_id = player->get_uuid();
// 			raid->set_player_info(player, &raid->data->player_info3[i]);		
// 			player->set_enter_raid_pos_and_scene(raid);

 			get_final_rand_born_pos3(&pos_x, &pos_z, &direct);
// 			player->set_pos(pos_x, pos_z);		
// //		player->set_pos(sg_3v3_pvp_raid_param2[1], sg_3v3_pvp_raid_param2[3]);

// 			extern_data.player_id = player->get_uuid();
// 			fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ENTER_RAID_NOTIFY, enter_raid_notify__pack, notify);

// 			player->send_scene_transfer(sg_guild_raid_final_param3[4], pos_x, sg_guild_raid_final_param3[2],
// 				pos_z, GUILD_RAID_FINAL_ID, 0);

// 			raid->on_player_enter_raid(player);
			raid->player_enter_raid_impl(player, i + MAX_TEAM_MEM + MAX_TEAM_MEM, pos_x, pos_z, direct);					
		}
		if (team4)
		{
			player = team4->team_player[i];
			assert(player);

			t_raid = player->get_raid();
			if (t_raid)
			{
				player->set_out_raid_pos_and_clear_scene();
				t_raid->delete_player_from_scene(player);
				t_raid->on_player_leave_raid(player);
			}
		
// 			raid->m_player4[i] = player;
// //		memset(&raid->data->player_info2[i], 0, sizeof(raid->data->player_info[i]));
// //		raid->data->player_info2[i].player_id = player->get_uuid();
// 			raid->set_player_info(player, &raid->data->player_info4[i]);		
// 			player->set_enter_raid_pos_and_scene(raid);

 			get_final_rand_born_pos4(&pos_x, &pos_z, &direct);
// 			player->set_pos(pos_x, pos_z);		
// //		player->set_pos(sg_3v3_pvp_raid_param2[1], sg_3v3_pvp_raid_param2[3]);

// 			extern_data.player_id = player->get_uuid();
// 			fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ENTER_RAID_NOTIFY, enter_raid_notify__pack, notify);

// 			player->send_scene_transfer(sg_guild_raid_final_param4[4], pos_x, sg_guild_raid_final_param4[2],
// 				pos_z, GUILD_RAID_FINAL_ID, 0);

// 			raid->on_player_enter_raid(player);
			raid->player_enter_raid_impl(player, i + MAX_TEAM_MEM + MAX_TEAM_MEM + MAX_TEAM_MEM, pos_x, pos_z, direct);								
		}
	}

	create_guild_battle_team(raid, team1);
	create_guild_battle_team2(raid, team2);
	if (team3)
	{
		create_guild_battle_team3(raid, team3);
	}
	if (team4)
	{
		create_guild_battle_team4(raid, team4);
	}
	return (0);
}

int start_final_guild_battle_match()
{
		// 把有队伍的玩家加入waiting_team
	guild_battle_manager_waiting_team.clear();
	std::set<player_struct *>::iterator next;
	for (std::set<player_struct *>::iterator ite = guild_battle_manager_waiting_player.begin(); ite != guild_battle_manager_waiting_player.end(); ite = next)
	{
		next = ite;
		++next;
		Team *team = (*ite)->m_team;
		
		if (!team)
			continue;
		// 检查是否都是一个工会的
#ifdef DEBUG
		for (int i = 0; i < team->GetMemberSize(); ++i)
		{
			player_struct *t_player = player_manager::get_player_by_id(team->m_data->m_mem[i].id);
			if (!t_player || !t_player->is_online())
			{
				LOG_ERR("%s: player[%lu] not online", __FUNCTION__, team->m_data->m_mem[i].id);
				continue;
			}
			if (t_player->data->guild_id != (*ite)->data->guild_id)
			{
				LOG_ERR("%s: player[%lu] guild not same", __FUNCTION__, team->m_data->m_mem[i].id);
				continue;
			}
		}
#endif
		guild_battle_manager_waiting_player.erase(ite);
		guild_battle_manager_waiting_team.insert(team);
	}
	
	try_match_final_guild_battle();

	//匹配不到的算胜利
	cant_matched_player_round_finished();

	for(std::vector<player_struct *>::iterator ite = guild_battle_manager_cant_matched_player.begin(); ite != guild_battle_manager_cant_matched_player.end(); ++ite)
	{
		guild_battle_manager_waiting_player.insert(*ite);
	}
	guild_battle_manager_cant_matched_player.clear();
	return (0);
}

int start_guild_battle_match()
{
		// 把有队伍的玩家加入waiting_team
	guild_battle_manager_waiting_team.clear();
	std::set<player_struct *>::iterator next;
	for (std::set<player_struct *>::iterator ite = guild_battle_manager_waiting_player.begin(); ite != guild_battle_manager_waiting_player.end(); ite = next)
	{
		next = ite;
		++next;
		Team *team = (*ite)->m_team;
		
		if (!team)
			continue;
		// 检查是否都是一个工会的
#ifdef DEBUG
		for (int i = 0; i < team->GetMemberSize(); ++i)
		{
			player_struct *t_player = player_manager::get_player_by_id(team->m_data->m_mem[i].id);
			if (!t_player || !t_player->is_online())
			{
				LOG_ERR("%s: player[%lu] not online", __FUNCTION__, team->m_data->m_mem[i].id);
				continue;
			}
			if (t_player->data->guild_id != (*ite)->data->guild_id)
			{
				LOG_ERR("%s: player[%lu] guild not same", __FUNCTION__, team->m_data->m_mem[i].id);
				continue;
			}
		}
#endif
		guild_battle_manager_waiting_player.erase(ite);
		guild_battle_manager_waiting_team.insert(team);
	}
	
	try_match_guild_battle();

	//匹配不到的算胜利
	cant_matched_player_round_finished();

	for(std::vector<player_struct *>::iterator ite = guild_battle_manager_cant_matched_player.begin(); ite != guild_battle_manager_cant_matched_player.end(); ++ite)
	{
		guild_battle_manager_waiting_player.insert(*ite);
	}
	guild_battle_manager_cant_matched_player.clear();
	return (0);
}

void print_waiting_player(struct evbuffer *buffer)
{
	for (std::set<player_struct *>::iterator ite = guild_battle_manager_waiting_player.begin(); ite != guild_battle_manager_waiting_player.end(); ++ite)
	{
		player_struct *player = (*ite);
		evbuffer_add_printf(buffer, "%lu: %s<br><br>\n", player->get_uuid(), player->get_name());
	}
}

void broadcast_guild_battle_call_notify(std::vector<uint64_t> &playerIds, uint64_t caller_id, char *caller_name)
{
	if (playerIds.size() == 0)
	{
		return;
	}

	GuildBattleCallNotify nty;
	guild_battle_call_notify__init(&nty);

	nty.activityid = guild_battle_manager_action_act;
	if (caller_id > 0 && caller_name != NULL)
	{
		nty.callerid = caller_id;
		nty.has_callerid = true;
		nty.callername = caller_name;
	}

	uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(MSG_ID_GUILD_BATTLE_CALL_NOTIFY, &nty, (pack_func)guild_battle_call_notify__pack);
	for (std::vector<uint64_t>::iterator iter = playerIds.begin(); iter != playerIds.end(); ++iter)
	{
		ppp = conn_node_gamesrv::broadcast_msg_add_players(*iter, ppp);
	}
	conn_node_gamesrv::broadcast_msg_send();
}

void call_all_guild_player()
{
	std::vector<uint64_t> playerIds;
	for (std::map<uint64_t, player_struct *>::iterator iter = player_manager_all_players_id.begin(); iter != player_manager_all_players_id.end(); ++iter)
	{
		if (get_entity_type(iter->first) == ENTITY_TYPE_AI_PLAYER)
			continue;
		
		player_struct *player = iter->second;
		if (!player->is_online())
		{
			continue;
		}
		if (player_can_participate_guild_battle(player) != 0)
		{
			continue;
		}
		if (player->is_in_raid())
		{
			continue;
		}
		if (!player->is_alive())
		{
			continue;
		}
		playerIds.push_back(iter->first);
	}

	broadcast_guild_battle_call_notify(playerIds);
}

void broadcast_to_all_guild_wait(uint32_t msg_id, void *msg_data, pack_func func)
{
	std::set<uint64_t> playerIds;
	for (std::map<uint64_t, guild_wait_raid_struct *>::iterator iter = guild_wait_raid_manager_all_raid_id.begin(); iter != guild_wait_raid_manager_all_raid_id.end(); ++iter)
	{
		guild_wait_raid_struct *raid = iter->second;
		raid->get_wait_player(playerIds);
	}

	if (playerIds.size() == 0)
	{
		return;
	}

	uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(msg_id, msg_data, func);
	PROTO_HEAD_CONN_BROADCAST *head;
	head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;

	for (std::set<uint64_t>::iterator iter = playerIds.begin(); iter != playerIds.end(); ++iter)
	{
		conn_node_gamesrv::broadcast_msg_add_players(*iter, ppp);					
	}

	if (head->num_player_id > 0)
	{
//		head->len += sizeof(uint64_t) * head->num_player_id;
		conn_node_gamesrv::broadcast_msg_send();
	}
}

void broadcast_wait_time_change()
{
	GuildBattleWaitInfoNotify nty;
	guild_battle_wait_info_notify__init(&nty);

	nty.state = guild_battle_manager_action_state;
	nty.has_state = true;
	nty.waittime = guild_battle_manager_action_tick;
	nty.has_waittime = true;

	broadcast_to_all_guild_wait(MSG_ID_GUILD_BATTLE_WAIT_INFO_NOTIFY, &nty, (pack_func)guild_battle_wait_info_notify__pack);
}

void broadcast_round_change()
{
	GuildBattleWaitInfoNotify nty;
	guild_battle_wait_info_notify__init(&nty);

	nty.round = guild_battle_manager_action_round;
	nty.has_round = true;

	broadcast_to_all_guild_wait(MSG_ID_GUILD_BATTLE_WAIT_INFO_NOTIFY, &nty, (pack_func)guild_battle_wait_info_notify__pack);
}

static uint32_t get_act_match_time()
{
	if (guild_battle_is_final())
	{
		return sg_guild_battle_final_match_time;
	}

	return sg_guild_battle_match_time;
}

static uint32_t get_act_fight_time()
{
	if (guild_battle_is_final())
	{
		return sg_guild_battle_final_fight_time;
	}

	return sg_guild_battle_fight_time;
}

static uint32_t get_act_settle_time()
{
	if (guild_battle_is_final())
	{
		return sg_guild_battle_final_settle_time;
	}

	return sg_guild_battle_settle_time;
}

static uint32_t get_act_round_num()
{
	if (guild_battle_is_final())
	{
		return sg_guild_battle_final_round_num;
	}

	return sg_guild_battle_round_num;
}

static void start_act_match()
{
	if (guild_battle_is_final())
	{
		start_final_guild_battle_match();
	}
	else
	{
		start_guild_battle_match();
	}
}

static void sync_guild_battle_begin()
{
	PROTO_HEAD *head = conn_node_base::get_send_buf(SERVER_PROTO_GUILD_BATTLE_BEGIN, 0);
	uint32_t *pData = (uint32_t*)(head->data);
	*pData++ = guild_battle_manager_action_act;
	head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + sizeof(uint32_t));
	if (conn_node_gamesrv::connecter.send_one_msg(head, 1) != (int)ENDION_FUNC_4(head->len))
	{
		LOG_ERR("[%s:%d] send to guildsrv failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
}

static void sync_guild_battle_end()
{
	PROTO_HEAD *head = conn_node_base::get_send_buf(SERVER_PROTO_GUILD_BATTLE_END, 0);
	uint32_t *pData = (uint32_t*)(head->data);
	*pData++ = guild_battle_manager_action_act;
	head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + sizeof(uint32_t));
	if (conn_node_gamesrv::connecter.send_one_msg(head, 1) != (int)ENDION_FUNC_4(head->len))
	{
		LOG_ERR("[%s:%d] send to guildsrv failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
}

static void sync_guild_battle_settle()
{
	PROTO_HEAD *head = conn_node_base::get_send_buf(SERVER_PROTO_GUILD_BATTLE_SETTLE, 0);
	PROTO_GUILD_BATTLE_SETTLE *proto = (PROTO_GUILD_BATTLE_SETTLE*)head;
	proto->activity_id = guild_battle_manager_action_act;
	proto->broadcast_num = 0;
	uint64_t *ppp = proto->broadcast_id;
	for (std::set<player_struct*>::iterator iter = guild_battle_manager_waiting_player.begin(); iter != guild_battle_manager_waiting_player.end(); ++iter)
	{
		*ppp++ = (*iter)->get_uuid();
		proto->broadcast_num++;
	}
	head->len = ENDION_FUNC_4(sizeof(PROTO_GUILD_BATTLE_SETTLE) + sizeof(uint64_t) * proto->broadcast_num);
	if (conn_node_gamesrv::connecter.send_one_msg(head, 1) != (int)ENDION_FUNC_4(head->len))
	{
		LOG_ERR("[%s:%d] send to guildsrv failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
}

static void get_final_list()
{
	uint32_t now = time_helper::get_cached_time() / 1000;
	//在决赛来临或者进行期间，如果没有决赛参赛名单，去gulid_srv获取
	if (!guild_battle_is_final())
	{
		return;
	}
	if (guild_battle_manager_final_list_state == GBFLS_GOTTEN)
	{
		return;
	}
	if (guild_battle_manager_final_list_state == GBFLS_GETTING && guild_battle_manager_final_list_tick + 5 >= now)
	{
		return;
	}

	PROTO_HEAD *head = conn_node_base::get_send_buf(SERVER_PROTO_GUILD_BATTLE_FINAL_LIST_REQUEST, 0);
	head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD));
	if (conn_node_gamesrv::connecter.send_one_msg(head, 1) != (int)ENDION_FUNC_4(head->len))
	{
		LOG_ERR("[%s:%d] send to guildsrv failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
	else
	{
		guild_battle_manager_final_list_state = GBFLS_GETTING;
		guild_battle_manager_final_list_tick = now;
	}
}

static void clear_all_guild_battle_player()
{
	/* 
	 * 活动结束，把所有在准备区和战斗副本里的玩家都拉到帮会领地
	 * 先踢战斗副本的玩家，防止玩家退回准备区导致的重复创建准备副本
	 */

	//把战斗副本的玩家踢出去，如果可以删除，删除副本
	for (std::map<uint64_t, raid_struct *>::iterator iter = raid_manager_all_raid_id.begin(); iter != raid_manager_all_raid_id.end(); )
	{
		raid_struct *raid = iter->second;
		++iter;
		if (!raid->is_guild_battle_fight_raid())
		{
			continue;
		}

		for (int i = 0; i < MAX_RAID_TEAM_NUM; ++i)
		{
			player_struct **team_players = NULL;
			if (i == 0)
			{
				team_players = raid->m_player;
			}
			else if (i == 1)
			{
				team_players = raid->m_player2;
			}
			else if (i == 2)
			{
				team_players = raid->m_player3;
			}
			else if (i == 3)
			{
				team_players = raid->m_player4;
			}

			if (!team_players)
			{
				continue;
			}

			for (int j = 0; j < MAX_TEAM_MEM; ++j)
			{
				player_struct *player = team_players[j];
				if (player)
				{
					raid->player_leave_raid(player);
				}
			}
		}

		if (raid->check_raid_need_delete())
		{
			raid_manager::delete_raid(raid);
		}
	}
	
	//把准备区副本的玩家踢出去，如果可以删除，删除副本
	for (std::map<uint64_t, guild_wait_raid_struct *>::iterator iter = guild_wait_raid_manager_all_raid_id.begin(); iter != guild_wait_raid_manager_all_raid_id.end(); )
	{
		guild_wait_raid_struct *raid = iter->second;
		++iter;
		std::set<uint64_t> player_ids;
		raid->get_wait_player(player_ids);
		for (std::set<uint64_t>::iterator iter = player_ids.begin(); iter != player_ids.end(); ++iter)
		{
			player_struct *player = player_manager::get_player_by_id(*iter);
			if (player)
			{
				raid->player_leave_raid(player);
			}
		}
		if (raid->check_raid_need_delete())
		{
			guild_wait_raid_manager::delete_guild_wait_raid(raid);
		}
	}
}

void run_activity_period()
{
	get_final_list();
	uint32_t now = time_helper::get_cached_time() / 1000;
	if (guild_battle_manager_action_tick == 0)
	{
		return;
	}

	if (now < guild_battle_manager_action_tick)
	{
		return;
	}

	switch (guild_battle_manager_action_state)
	{
		case GBS_BEGIN:
			{
				sync_guild_battle_begin();

				guild_battle_manager_activity_start_ts = now;
				guild_battle_manager_action_tick += get_act_match_time();
				guild_battle_manager_action_state = GBS_WAIT;
				guild_battle_manager_action_round = 1;

				//活动开始，清除所有数据
				guild_battle_manager_guild_participate.clear();
				guild_battle_manager_guild_call_cd.clear();
				guild_battle_manager_participate_players.clear();
				//通知所有帮会玩家活动开始
				call_all_guild_player();
			}
			break;
		case GBS_WAIT:
			{
				guild_battle_manager_action_tick += get_act_fight_time();
				guild_battle_manager_action_state = GBS_BATTLE;
				//等待时间到，进行匹配战斗
				start_act_match();
				//通知等待区玩家等待时间发生变化
				broadcast_wait_time_change();
			}
			break;
		case GBS_BATTLE:
			{
				if (guild_battle_manager_action_round == get_act_round_num()) 
				{
					guild_battle_manager_action_tick += get_act_settle_time();
					guild_battle_manager_action_state = GBS_SETTLE;
				}
				else
				{
					guild_battle_manager_action_tick += get_act_match_time();
					guild_battle_manager_action_state = GBS_WAIT;
					guild_battle_manager_action_round++;
					//通知等待区玩家回合变化
					broadcast_round_change();
				}

				//通知等待区玩家等待时间发生变化
				broadcast_wait_time_change();
				//战斗时间结束，没决出胜负的副本进行结算
				on_guild_battle_raid_time_out();

				//通知guild_srv进行活动结算
				if (guild_battle_manager_action_state == GBS_SETTLE) 
				{
					sync_guild_battle_settle();
				}
			}
			break;
		case GBS_SETTLE:
			{
				sync_guild_battle_end();
				if (guild_battle_is_final())
				{
					/* 决赛结束，清除参赛名单信息 */
					guild_battle_manager_final_guild_id.clear();
					guild_battle_manager_final_list_state = 0;
					guild_battle_manager_final_list_tick = 0;
				}

				get_guild_battle_next_activity_begin_time(guild_battle_manager_action_act, guild_battle_manager_action_tick);
				guild_battle_manager_action_state = GBS_BEGIN;
				guild_battle_manager_action_round = 0;

				clear_all_guild_battle_player();
			}
			break;
	}

	LOG_INFO("[%s:%d] enter state %u, tick:%u, round:%u", __FUNCTION__, __LINE__, guild_battle_manager_action_state, guild_battle_manager_action_tick, guild_battle_manager_action_round);
}

void guild_battle_manager_on_tick()
{
// 	if (1)
// 		try_match_guild_battle();
// 	else
// 		try_match_final_guild_battle();	
	check_guild_wait_award(); //等待区奖励
	run_activity_period();
}

void start_guild_battle_activity()
{
	guild_battle_manager_action_tick = time_helper::get_cached_time() / 1000;
	guild_battle_manager_action_state = GBS_BEGIN;
	guild_battle_manager_action_act = GUILD_BATTLE_PRELIMINARY_ACTIVITY_ID;
}

void start_final_guild_battle_activity()
{
	guild_battle_manager_action_tick = time_helper::get_cached_time() / 1000;
	guild_battle_manager_action_state = GBS_BEGIN;
	guild_battle_manager_action_act = GUILD_BATTLE_FINAL_ACTIVITY_ID;
}

struct ActTimeCmpInfo
{
	uint32_t act_id;
	uint32_t begin_time;
};

bool act_time_cmp_func(const ActTimeCmpInfo &l, const ActTimeCmpInfo& r)
{
	if (l.begin_time <= r.begin_time)
	{
		return true;
	}

	return false;
}

static void get_guild_battle_next_activity_begin_time(uint32_t &act_id, uint32_t &begin_time)
{
	uint32_t now = time_helper::get_micro_time() / 1000 / 1000;
	std::vector<ActTimeCmpInfo> act_time_sort_list;
	std::vector<uint32_t> act_ids;
	act_ids.push_back(GUILD_BATTLE_PRELIMINARY_ACTIVITY_ID);
	act_ids.push_back(GUILD_BATTLE_FINAL_ACTIVITY_ID);
	for (std::vector<uint32_t>::iterator iter = act_ids.begin(); iter != act_ids.end(); ++iter)
	{
		EventCalendarTable *act_config = get_config_by_id(*iter, &activity_config);
		if (!act_config)
		{
			break;
		}

		ControlTable *ctrl_config = get_config_by_id(act_config->RelationID, &all_control_config);
		if (!ctrl_config)
		{
			break;
		}

		for (uint32_t i = 0; i < ctrl_config->n_OpenDay; ++i)
		{
			int wday = ctrl_config->OpenDay[i];
			for (uint32_t j = 0; j < ctrl_config->n_OpenTime; ++j)
			{
				int hour = ctrl_config->OpenTime[j] / 100;
				int minute = ctrl_config->OpenTime[j] % 100;

				ActTimeCmpInfo info;
				info.act_id = act_config->ID;
				info.begin_time = time_helper::get_next_timestamp_by_week_old(wday, hour, minute, now);
				act_time_sort_list.push_back(info);
			}
		}
	}

	if (act_time_sort_list.size() == 0)
	{
		return ;
	}

	std::sort(act_time_sort_list.begin(), act_time_sort_list.end(), act_time_cmp_func);
	ActTimeCmpInfo &info = *act_time_sort_list.begin();
	act_id = info.act_id;
	begin_time = info.begin_time;
}

bool is_guild_battle_opening()
{
	return (guild_battle_manager_action_state != GBS_BEGIN && guild_battle_manager_action_state != 0);
}

bool is_guild_battle_settling() //活动是否在结算时间
{
	return (guild_battle_manager_action_state == GBS_SETTLE);
}

bool player_can_return_guild_battle(player_struct *player)
{
	return (player->data->guild_battle_activity_time == guild_battle_manager_activity_start_ts);
}

int player_can_participate_guild_battle(player_struct *player)
{
	if (player->data->guild_id == 0)
	{
		return ERROR_ID_GUILD_PLAYER_NOT_JOIN;
	}

	if (guild_battle_is_final() && guild_battle_manager_final_guild_id.find(player->data->guild_id) == guild_battle_manager_final_guild_id.end())
	{
		return 190411003;
	}

	return 0;
}

ProtoGuildInfo *get_guild_summary(uint32_t guild_id)
{
	std::map<uint32_t, ProtoGuildInfo>::iterator iter = guild_summary_map.find(guild_id);
	if (iter != guild_summary_map.end())
	{
		return &iter->second;
	}

	return NULL;
}

uint32_t get_guild_participate_num(uint32_t guild_id)
{
	std::map<uint32_t, uint32_t>::iterator iter = guild_battle_manager_guild_participate.find(guild_id);
	if (iter != guild_battle_manager_guild_participate.end())
	{
		return iter->second;
	}
	
	return 0;
}

void check_guild_participate_num(uint32_t guild_id)
{
	guild_wait_raid_struct *raid = guild_wait_raid_manager::get_guild_wait_raid(guild_id);
	if (!raid)
	{
		return;
	}

	uint32_t old_num = get_guild_participate_num(guild_id);
	uint32_t cur_num = 0;
	cur_num += raid->get_cur_player_num();
	for (std::map<uint64_t, raid_struct *>::iterator iter = raid_manager_all_raid_id.begin(); iter != raid_manager_all_raid_id.end(); ++iter)
	{
		raid_struct *pRaid = iter->second;
		if (!pRaid->is_guild_battle_fight_raid())
		{
			continue;
		}

		for (int i = 0; i < MAX_TEAM_MEM; ++i)
		{
			player_struct *team1_player = pRaid->m_player[i];
			if (team1_player && team1_player->data->guild_id == guild_id)
			{
				cur_num++;
			}
			player_struct *team2_player = pRaid->m_player2[i];
			if (team2_player && team2_player->data->guild_id == guild_id)
			{
				cur_num++;
			}
		}
	}

	if (cur_num != old_num)
	{
		guild_battle_manager_guild_participate[guild_id] = cur_num;

		GuildBattleWaitInfoNotify nty;
		guild_battle_wait_info_notify__init(&nty);

		nty.participatenum = cur_num;
		nty.has_participatenum = true;

		raid->broadcast_to_raid(MSG_ID_GUILD_BATTLE_WAIT_INFO_NOTIFY, &nty, (pack_func)guild_battle_wait_info_notify__pack);
	}
}

uint32_t set_guild_call_cd(uint32_t guild_id)
{
	uint32_t now = time_helper::get_cached_time() / 1000;
	uint32_t tick = now + 60;
	guild_battle_manager_guild_call_cd[guild_id] = tick;

	guild_wait_raid_struct *raid = guild_wait_raid_manager::get_guild_wait_raid(guild_id);
	if (raid)
	{
		GuildBattleWaitInfoNotify nty;
		guild_battle_wait_info_notify__init(&nty);

		nty.callcd = tick;
		nty.has_callcd = true;

		raid->broadcast_to_raid(MSG_ID_GUILD_BATTLE_WAIT_INFO_NOTIFY, &nty, (pack_func)guild_battle_wait_info_notify__pack);
	}
	return tick;
}

uint32_t get_guild_call_cd(uint32_t guild_id)
{
	std::map<uint32_t, uint32_t>::iterator iter = guild_battle_manager_guild_participate.find(guild_id);
	if (iter != guild_battle_manager_guild_participate.end())
	{
		return iter->second;
	}

	return 0;
}

uint32_t get_wait_time()
{
	if (guild_battle_manager_action_state == GBS_WAIT)
	{
		return guild_battle_manager_action_tick;
	}
	else if (guild_battle_manager_action_state == GBS_BATTLE)
	{
		if (guild_battle_manager_action_round == get_act_round_num())
		{
			return guild_battle_manager_action_tick + get_act_settle_time();
		}
		else
		{
			return guild_battle_manager_action_tick + get_act_match_time();
		}
	}
	else if (guild_battle_manager_action_state == GBS_SETTLE)
	{
		return guild_battle_manager_action_tick;
	}
	return 0;
}

int pack_guild_wait_info(uint32_t guild_id, uint8_t *out_data)
{
	GuildBattleWaitInfoNotify nty;
	guild_battle_wait_info_notify__init(&nty);

	nty.participatenum = guild_battle_manager_guild_participate[guild_id];
	nty.has_participatenum = true;
	nty.round = guild_battle_manager_action_round;
	nty.has_round = true;
	nty.state = guild_battle_manager_action_state;
	nty.has_state = true;
	nty.waittime = guild_battle_manager_action_tick;
	nty.has_waittime = true;
	nty.callcd = get_guild_call_cd(guild_id);
	nty.has_callcd = true;
	nty.activityid = guild_battle_manager_action_act;
	nty.has_activityid = true;

	return guild_battle_wait_info_notify__pack(&nty, out_data);
}

char *get_guild_name(uint32_t guild_id)
{
	ProtoGuildInfo *info = get_guild_summary(guild_id);
	if (info)
	{
		return info->name;
	}
	return NULL;
}

uint32_t get_cur_round_end_time()
{
	if (guild_battle_manager_action_state == GBS_BATTLE)
	{
		return guild_battle_manager_action_tick;
	}
	return 0;
}

bool guild_battle_is_final()
{
	return (guild_battle_manager_action_act == (uint32_t)GUILD_BATTLE_FINAL_ACTIVITY_ID);
}

//获取战斗奖励
void get_guild_battle_fight_reward(player_struct *player, int result, uint32_t &score, uint32_t &treasure, uint32_t &donation, uint32_t kill_num, uint32_t dead_num, uint32_t monster_num, uint32_t boss_damage, uint32_t boss_num)
{
	if (!guild_battle_is_final())
	{ //预赛
		/*
		   帮战预赛奖励-不战而胜-1勇武值2积分3帮贡4帮会资金
		   帮战预赛奖励-胜-1勇武值2积分系数3帮贡4帮会资金系数
		   帮战预赛奖励-负-1勇武值2积分系数3帮贡4帮会资金系数
		   帮战预赛奖励-平-1勇武值2积分系数3帮贡4帮会资金系数
		   帮战预赛奖励-帮会资金-1杀人系数2死亡系数3基础系数
		   公式1：勇武值+帮贡直接填表
		   公式2：积分=勇武值+胜/负/平积分系数
		   公式3：帮会资金=胜负平系数*max（（杀人数*杀人系数-死亡数*死亡系数），基础系数）
		 */
		int brave = player->get_attr(PLAYER_ATTR_BRAVE);
		int brave_add = 0;
		int score_factor = 0;
		int donation_add = 0;
		int treasure_factor = 0;
		switch (result)
		{
			case GBR_WIN:
				{
					brave_add = sg_guild_battle_fight_win_reward[0];
					score_factor = sg_guild_battle_fight_win_reward[1];
					donation_add = sg_guild_battle_fight_win_reward[2];
					treasure_factor = sg_guild_battle_fight_win_reward[3];
				}
				break;
			case GBR_LOSE:
				{
					brave_add = sg_guild_battle_fight_lose_reward[0];
					score_factor = sg_guild_battle_fight_lose_reward[1];
					donation_add = sg_guild_battle_fight_lose_reward[2];
					treasure_factor = sg_guild_battle_fight_lose_reward[3];
				}
				break;
			case GBR_DRAW:
				{
					brave_add = sg_guild_battle_fight_draw_reward[0];
					score_factor = sg_guild_battle_fight_draw_reward[1];
					donation_add = sg_guild_battle_fight_draw_reward[2];
					treasure_factor = sg_guild_battle_fight_draw_reward[3];
				}
				break;
			case GBR_AUTO_WIN:
				{
					brave_add = sg_guild_battle_fight_auto_win_reward[0];
					score_factor = sg_guild_battle_fight_auto_win_reward[1];
					donation_add = sg_guild_battle_fight_auto_win_reward[2];
					treasure_factor = sg_guild_battle_fight_auto_win_reward[3];
				}
				break;
		}

		brave = std::max(brave + brave_add, 0);
		player->set_attr(PLAYER_ATTR_BRAVE, brave);
		donation = donation_add;

		if (result == GBR_AUTO_WIN)
		{
			score = score_factor;
			treasure = treasure_factor;
		}
		else
		{
			score = brave + score_factor;
			int kill_factor = sg_guild_battle_treasure_factor[0];
			int dead_factor = sg_guild_battle_treasure_factor[1];
			int base_factor = sg_guild_battle_treasure_factor[2];
			treasure = treasure_factor * std::max(((int)kill_num * kill_factor - (int)dead_num * dead_factor), base_factor);
		}
	}
	else
	{ //决赛
		/*
		 * 帮战决赛奖励-不战而胜-（1）勇武值（2）积分（3）帮贡（4）帮会资金
		 * 帮战决赛奖励-第1名-（1）勇武值（2）帮贡（3）帮会资金系数
		 * 帮战决赛奖励-第2名-（1）勇武值（2）帮贡（3）帮会资金系数
		 * 帮战决赛奖励-第3名-（1）勇武值（2）帮贡（3）帮会资金系数
		 * 帮战决赛奖励-第4名-（1）勇武值（2）帮贡（3）帮会资金系数
		 * 帮战决赛奖励-积分-（1）小怪系数（2）BOSS系数（3）杀人系数（4）boss击杀积分
		 * 勇武值+帮贡直接填表
		 * 积分公式：小怪数*小怪系数+对boss造成1%伤害*boss积分系数+杀人数*杀人系数+BOSS击杀积分
		 * 帮战决赛帮会资金基础系数
		 * 帮会资金公式：第1-4名系数*max（积分，基础系数）
		 */
		int brave = player->get_attr(PLAYER_ATTR_BRAVE);
		int brave_add = 0;
		int score_factor = 0;
		int donation_add = 0;
		int treasure_factor = 0;
		switch (result)
		{
			case 0:
				{
					brave_add = sg_guild_battle_final_fight_reward_0[0];
					score_factor = sg_guild_battle_final_fight_reward_0[1];
					donation_add = sg_guild_battle_final_fight_reward_0[2];
					treasure_factor = sg_guild_battle_final_fight_reward_0[3];
				}
				break;
			case 1:
				{
					brave_add = sg_guild_battle_final_fight_reward_1[0];
					donation_add = sg_guild_battle_final_fight_reward_1[1];
					treasure_factor = sg_guild_battle_final_fight_reward_1[2];
				}
				break;
			case 2:
				{
					brave_add = sg_guild_battle_final_fight_reward_2[0];
					donation_add = sg_guild_battle_final_fight_reward_2[1];
					treasure_factor = sg_guild_battle_final_fight_reward_2[2];
				}
				break;
			case 3:
				{
					brave_add = sg_guild_battle_final_fight_reward_3[0];
					donation_add = sg_guild_battle_final_fight_reward_3[1];
					treasure_factor = sg_guild_battle_final_fight_reward_3[2];
				}
				break;
			case 4:
				{
					brave_add = sg_guild_battle_final_fight_reward_4[0];
					donation_add = sg_guild_battle_final_fight_reward_4[1];
					treasure_factor = sg_guild_battle_final_fight_reward_4[2];
				}
				break;
		}

		brave = std::max(brave + brave_add, 0);
		player->set_attr(PLAYER_ATTR_BRAVE, brave);
		donation = donation_add;

		if (result == 0)
		{
			score = score_factor;
			treasure = treasure_factor;
		}
		else
		{
			score = calcu_player_final_score(kill_num, monster_num, boss_damage, boss_num);

			int base_factor = sg_guild_battle_final_treasure_factor[0];
			treasure = treasure_factor * std::max(score, (uint32_t)base_factor);
		}
	}
}

void notify_guild_battle_fight_reward(player_struct *player, int result, uint32_t score, uint32_t treasure, uint32_t donation)
{
	GuildBattleRoundFinishNotify nty;
	guild_battle_round_finish_notify__init(&nty);

	nty.result = result;
	nty.score = score;
	nty.guildtreasure = treasure;
	nty.guilddonation = donation;

	EXTERN_DATA ext_data;
	ext_data.player_id = player->get_uuid();

	fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_GUILD_BATTLE_ROUND_FINISH_NOTIFY, guild_battle_round_finish_notify__pack, nty);
}

void send_guild_battle_fight_reward_to_guildsrv(std::map<uint32_t, GuildBattleFightGuildRewardInfo> &reward_map)
{
	for (std::map<uint32_t, GuildBattleFightGuildRewardInfo>::iterator guild_iter = reward_map.begin(); guild_iter != reward_map.end(); ++guild_iter)
	{
		GuildBattleFightGuildRewardInfo &guild_reward = guild_iter->second;
		PROTO_GUILD_BATTLE_REWARD *req = (PROTO_GUILD_BATTLE_REWARD*)conn_node_base::get_send_buf(SERVER_PROTO_GUILD_BATTLE_REWARD, 0);
		PROTO_HEAD *head = &req->head;
		memset(head->data, 0, sizeof(PROTO_GUILD_BATTLE_REWARD) - sizeof(PROTO_HEAD));
		head->len = ENDION_FUNC_4(sizeof(PROTO_GUILD_BATTLE_REWARD));
		req->activity_id = guild_battle_manager_action_act;
		req->guild_id = guild_reward.guild_id;

		for (std::vector<GuildBattleFightPlayerRewardInfo>::iterator player_iter = guild_reward.players.begin(); player_iter != guild_reward.players.end() && req->player_num < MAX_GUILD_MEMBER_NUM; ++player_iter)
		{
			req->player_id[req->player_num] = player_iter->player_id;
			req->result[req->player_num] = player_iter->result;
			req->score[req->player_num] = player_iter->score;
			req->treasure[req->player_num] = player_iter->treasure;
			req->donation[req->player_num] = player_iter->donation;
			req->player_num++;
		}

		guild_wait_raid_struct *raid = guild_wait_raid_manager::get_guild_wait_raid(guild_reward.guild_id);
		if (raid)
		{
			std::set<uint64_t> playerIds;
			raid->get_wait_player(playerIds);
			for (std::set<uint64_t>::iterator wait_iter = playerIds.begin(); wait_iter != playerIds.end() && req->broadcast_num < MAX_GUILD_MEMBER_NUM; ++wait_iter)
			{
				req->broadcast_id[req->broadcast_num] = *wait_iter;
				req->broadcast_num++;
			}
		}

		if (conn_node_gamesrv::connecter.send_one_msg(head, 1) != (int)ENDION_FUNC_4(head->len))
		{
			LOG_ERR("[%s:%d] send to guildsrv failed err[%d]", __FUNCTION__, __LINE__, errno);
		}
	}
}

void insert_guild_battle_fight_reward_map(std::map<uint32_t, GuildBattleFightGuildRewardInfo> &reward_map, player_struct *player, int result, uint32_t kill_num, uint32_t dead_num, uint32_t monster_num, uint32_t boss_damage, uint32_t boss_num)
{
	std::map<uint32_t, GuildBattleFightGuildRewardInfo>::iterator guild_iter = reward_map.find(player->data->guild_id);
	if (guild_iter == reward_map.end())
	{
		GuildBattleFightGuildRewardInfo guild_info;
		guild_info.guild_id = player->data->guild_id;
		guild_iter = reward_map.insert(std::make_pair(guild_info.guild_id, guild_info)).first;
	}

	GuildBattleFightPlayerRewardInfo player_reward;
	player_reward.player_id = player->get_uuid();
	player_reward.result = result;
	get_guild_battle_fight_reward(player, result, player_reward.score, player_reward.treasure, player_reward.donation, kill_num, dead_num, monster_num, boss_damage, boss_num);
	if (!guild_battle_is_final() || result == 0)
	{
		notify_guild_battle_fight_reward(player, result, player_reward.score, player_reward.treasure, player_reward.donation);
	}

	guild_iter->second.players.push_back(player_reward);
	if (guild_battle_manager_participate_players.find(player_reward.player_id) == guild_battle_manager_participate_players.end())
	{
		guild_battle_manager_participate_players.insert(player_reward.player_id);
		player->add_achievement_progress(ACType_GUILD_BATTLE, 0, 0, 0, 1);
	}
}

void get_guild_raid_reward(raid_struct *raid, int win_team, std::map<uint32_t, GuildBattleFightGuildRewardInfo> &reward_map)
{
	int team1_result = 0;
	int team2_result = 0;
	if (win_team == 1)
	{
		team1_result = GBR_WIN;
		team2_result = GBR_LOSE;
	}
	else if (win_team == 2)
	{
		team1_result = GBR_LOSE;
		team2_result = GBR_WIN;
	}
	else
	{
		team1_result = GBR_DRAW;
		team2_result = GBR_DRAW;
	}

	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		{
			player_struct *player = raid->m_player[i];
			if (player)
			{
				uint32_t kill_num = raid->GUILD_DATA.kill_record[i];
				uint32_t dead_num = raid->data->player_info[i].dead_count;
				insert_guild_battle_fight_reward_map(reward_map, player, team1_result, kill_num, dead_num);
			}
		}

		{
			player_struct *player = raid->m_player2[i];
			if (player)
			{
				uint32_t kill_num = raid->GUILD_DATA.kill_record[i + MAX_TEAM_MEM];
				uint32_t dead_num = raid->data->player_info2[i].dead_count;
				insert_guild_battle_fight_reward_map(reward_map, player, team2_result, kill_num, dead_num);
			}
		}
	}
}

static uint32_t calcu_player_final_score(uint32_t kill_num, uint32_t monster_num, uint32_t boss_damage, uint32_t boss_num)
{
	int monster_factor = sg_guild_battle_final_score_factor[0];
	int boss_factor = sg_guild_battle_final_score_factor[1];
	int kill_factor2 = sg_guild_battle_final_score_factor[2];
	int kill_boss = sg_guild_battle_final_score_factor[3];
	return (monster_num * monster_factor + boss_damage * boss_factor + kill_num * kill_factor2 + boss_num * kill_boss);
}

struct FinalScoreCmpInfo
{
	uint32_t team_idx;
	uint32_t score;
};

bool final_score_cmp_func(const FinalScoreCmpInfo &l, const FinalScoreCmpInfo &r)
{
	if (l.score > r.score)
	{
		return true;
	}

	return false;
}

void get_guild_final_raid_reward(raid_struct *raid, std::map<uint32_t, GuildBattleFightGuildRewardInfo> &reward_map)
{
	std::vector<FinalScoreCmpInfo> final_score_sort_list;
	for (int j = 0; j < MAX_RAID_TEAM_NUM; ++j)
	{
		if (raid->GUILD_FINAL_DATA.guild_id[j] == 0)
		{
			continue;
		}

		player_struct **players = NULL;
		raid_player_info *raid_players = NULL;
		switch (j)
		{
			case 0:
				{
					players = raid->m_player;
					raid_players = raid->data->player_info;
				}
				break;
			case 1:
				{
					players = raid->m_player2;
					raid_players = raid->data->player_info2;
				}
				break;
			case 2:
				{
					players = raid->m_player3;
					raid_players = raid->data->player_info3;
				}
				break;
			case 3:
				{
					players = raid->m_player4;
					raid_players = raid->data->player_info4;
				}
				break;
			default:
				break;
		}
		if (players == NULL || raid_players == NULL)
		{
			break;
		}

		FinalScoreCmpInfo info;
		info.team_idx = j;
		info.score = 0;
		for (int i = 0; i < MAX_TEAM_MEM; ++i)
		{
			player_struct *player = players[i];
			if (player)
			{
				uint32_t kill_num = raid->GUILD_FINAL_DATA.kill_record[j * MAX_TEAM_MEM + i];
				uint32_t monster_num = raid->GUILD_FINAL_DATA.monster_record[j * MAX_TEAM_MEM + i];
				uint32_t boss_damage = raid->GUILD_FINAL_DATA.boss_record[j * MAX_TEAM_MEM + i];
				uint32_t boss_maxhp = raid->GUILD_FINAL_DATA.boss_maxhp;
				uint32_t boss_damage_percent = (boss_maxhp == 0 ? 0 : round((double)boss_damage / (double)boss_maxhp * 100));
				uint32_t boss_num = (raid->GUILD_FINAL_DATA.boss_killer == player->get_uuid() ? 1 : 0);
				uint32_t score = calcu_player_final_score(kill_num, monster_num, boss_damage_percent, boss_num);
				info.score += score;
			}
		}

		final_score_sort_list.push_back(info);
	}

	std::sort(final_score_sort_list.begin(), final_score_sort_list.end(), final_score_cmp_func);

	for (size_t k = 0; k < final_score_sort_list.size(); ++k)
	{
		FinalScoreCmpInfo &info = final_score_sort_list[k];
		player_struct **players = NULL;
		raid_player_info *raid_players = NULL;
		int j = info.team_idx;
		switch (j)
		{
			case 0:
				{
					players = raid->m_player;
					raid_players = raid->data->player_info;
				}
				break;
			case 1:
				{
					players = raid->m_player2;
					raid_players = raid->data->player_info2;
				}
				break;
			case 2:
				{
					players = raid->m_player3;
					raid_players = raid->data->player_info3;
				}
				break;
			case 3:
				{
					players = raid->m_player4;
					raid_players = raid->data->player_info4;
				}
				break;
			default:
				break;
		}
		if (players == NULL || raid_players == NULL)
		{
			break;
		}

		for (int i = 0; i < MAX_TEAM_MEM; ++i)
		{
			player_struct *player = players[i];
			raid_player_info &raid_player = raid_players[i];
			if (player)
			{
				uint32_t kill_num = raid->GUILD_FINAL_DATA.kill_record[j * MAX_TEAM_MEM + i];
				uint32_t dead_num = raid_player.dead_count;
				uint32_t monster_num = raid->GUILD_FINAL_DATA.monster_record[j * MAX_TEAM_MEM + i];
				uint32_t boss_damage = raid->GUILD_FINAL_DATA.boss_record[j * MAX_TEAM_MEM + i];
				uint32_t boss_maxhp = raid->GUILD_FINAL_DATA.boss_maxhp;
				uint32_t boss_damage_percent = (boss_maxhp == 0 ? 0 : round((double)boss_damage / (double)boss_maxhp * 100));
				insert_guild_battle_fight_reward_map(reward_map, player, k+1, kill_num, dead_num, monster_num, boss_damage_percent);
			}
		}
	}
}

void cant_matched_player_round_finished()
{
	std::map<uint32_t, GuildBattleFightGuildRewardInfo> reward_map;
	for(std::vector<player_struct *>::iterator ite = guild_battle_manager_cant_matched_player.begin(); ite != guild_battle_manager_cant_matched_player.end(); ++ite)
	{
		player_struct *player = *ite;
		if (!guild_battle_is_final())
		{
			insert_guild_battle_fight_reward_map(reward_map, player, GBR_AUTO_WIN);
		}
		else
		{
			insert_guild_battle_fight_reward_map(reward_map, player, 0);
		}
	}

	send_guild_battle_fight_reward_to_guildsrv(reward_map);
}

static void check_guild_wait_award()
{
	if (!is_guild_battle_opening())
	{
		return;
	}
	uint32_t now = time_helper::get_cached_time() / 1000;
	for (std::set<player_struct*>::iterator iter = guild_battle_manager_waiting_player.begin(); iter != guild_battle_manager_waiting_player.end(); ++iter)
	{
		player_struct *player = *iter;
		if (player)
		{
			if (player->guild_battle_wait_award_time + sg_guild_battle_wait_award_interval <= now)
			{
				player->guild_battle_wait_award_time += sg_guild_battle_wait_award_interval;
				player->add_item_list_as_much_as_possible(sg_guild_battle_wait_award, MAGIC_TYPE_GUILD_BATTLE_WAIT);
			}
		}
	}
}

static void get_guild_raid_team_kill_num(raid_struct *raid, int *kill1, int *kill2)
{
	int a = 0, b = 0;
	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		a += raid->GUILD_DATA.kill_record[i];
		b += raid->GUILD_DATA.kill_record[i + MAX_TEAM_MEM];
	}
	*kill1 = a;
	*kill2 = b;
}

void on_guild_battle_raid_time_out()
{
	std::map<uint32_t, GuildBattleFightGuildRewardInfo> reward_map;
	for (std::map<uint64_t, raid_struct *>::iterator iter = raid_manager_all_raid_id.begin(); iter != raid_manager_all_raid_id.end(); ++iter)
	{
		raid_struct *raid = iter->second;
		if (guild_battle_is_final())
		{
			if (raid->m_id != (uint32_t)GUILD_RAID_FINAL_ID)
			{
				continue;
			}
		}
		else
		{
			if (raid->m_id != (uint32_t)GUILD_RAID_ID)
			{
				continue;
			}
		}

		if (raid->data->state == RAID_STATE_PASS)
		{
			continue;
		}

		raid->data->state = RAID_STATE_PASS;
		if (!guild_battle_is_final())
		{
			int team_kill_1, team_kill_2;
			get_guild_raid_team_kill_num(raid, &team_kill_1, &team_kill_2);
			int win_team = -1;
			if (team_kill_1 > team_kill_2)
			{
				win_team = 1;
			}
			else if (team_kill_1 < team_kill_2)
			{
				win_team = 2;
			}
			else
			{
				win_team = 0;
			}

			get_guild_raid_reward(raid, win_team, reward_map);
		}
		else
		{
			get_guild_final_raid_reward(raid, reward_map);
		}
	}

	send_guild_battle_fight_reward_to_guildsrv(reward_map);
}

void set_final_guild_id(uint32_t *guild_id, uint32_t num)
{
	guild_battle_manager_final_guild_id.clear();
	for (uint32_t i = 0; i < num; ++i)
	{
		if (guild_id[i] > 0)
		{
			guild_battle_manager_final_guild_id.insert(guild_id[i]);
		}
	}
}

void add_final_guild_id(uint32_t guild_id)
{
	if (guild_battle_manager_final_guild_id.size() >= 4)
	{
		return ;
	}
	if (guild_id == 0)
	{
		return ;
	}

	guild_battle_manager_final_guild_id.insert(guild_id);

	uint32_t *pData = (uint32_t*)conn_node_base::get_send_data();
	*pData++ = guild_id;
	EXTERN_DATA ext_data;
	fast_send_msg_base(&conn_node_gamesrv::connecter, &ext_data, SERVER_PROTO_GUILD_ADD_FINAL_BATTLE_GUILD, sizeof(uint32_t), 0);
}

void fill_player_base_data(raid_player_info &info, PlayerBaseData &data)
{
	data.playerid = info.player_id;
	data.name = info.name;
	data.n_attrs = 0;
	data.attrs[data.n_attrs]->id = PLAYER_ATTR_LEVEL;
	data.attrs[data.n_attrs]->val = info.lv;
	data.n_attrs++;
	data.attrs[data.n_attrs]->id = PLAYER_ATTR_JOB;
	data.attrs[data.n_attrs]->val = info.job;
	data.n_attrs++;
	data.attrs[data.n_attrs]->id = PLAYER_ATTR_HEAD;
	data.attrs[data.n_attrs]->val = info.headicon;
	data.n_attrs++;
}

void init_guild_battle_manager()
{
	if (guild_battle_manager_action_act == 0)
	{
		get_guild_battle_next_activity_begin_time(guild_battle_manager_action_act, guild_battle_manager_action_tick);
		guild_battle_manager_action_state = GBS_BEGIN;
	}
}



