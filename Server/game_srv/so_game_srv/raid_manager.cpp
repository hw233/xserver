#include "raid_manager.h"
#include "player_manager.h"
#include "game_event.h"
#include "uuid.h"
#include "raid.pb-c.h"
#include "msgid.h"
#include "app_data_statis.h"
#include "zhenying_raid_manager.h"
#include "guild_wait_raid_manager.h"

//队员和队长的最大距离20米，太远就要传送过来
static const uint64_t max_team_mem_distance = 20 * 20;

// std::map<uint64_t, raid_struct *> raid_manager::raid_manager_all_raid_id;
// std::list<raid_struct *> raid_manager::raid_manager_raid_free_list;
// std::set<raid_struct *> raid_manager::raid_manager_raid_used_list;
// struct comm_pool raid_manager::raid_manager_raid_data_pool;
int raid_manager::add_raid(raid_struct *p)
{
	raid_manager_all_raid_id[p->data->uuid] = p;
	return (0);
}

int raid_manager::remove_raid(raid_struct *p)
{
	raid_manager_all_raid_id.erase(p->data->uuid);
	return (0);
}

raid_struct *raid_manager::alloc_raid()
{
	raid_struct *ret = NULL;
	raid_data *data = NULL;
	if (raid_manager_raid_free_list.empty())
		return NULL;
	ret = raid_manager_raid_free_list.back();
	raid_manager_raid_free_list.pop_back();
	data = (raid_data *)comm_pool_alloc(&raid_manager_raid_data_pool);
	if (!data)
		goto fail;
	memset(data, 0, sizeof(raid_data));
	ret->data = data;
	raid_manager_raid_used_list.insert(ret);

	return ret;
fail:
	if (ret) {
		raid_manager_raid_used_list.erase(ret);
		raid_manager_raid_free_list.push_back(ret);
	}
	if (data) {
		comm_pool_free(&raid_manager_raid_data_pool, data);
	}
	return NULL;
}

void raid_manager::delete_raid(raid_struct *p)
{
	assert(p->m_config->DengeonRank != DUNGEON_TYPE_ZHENYING);
	assert(p->m_config->DengeonRank != DUNGEON_TYPE_GUILD_WAIT);	
	
	raid_manager_raid_used_list.erase(p);
	raid_manager_raid_free_list.push_back(p);

	LOG_DEBUG("[%s:%d] raid[%u %lu], %p, data:%p", __FUNCTION__, __LINE__, p->m_id, p->data->uuid, p, p->data);

	p->clear();

	if (p->data) {
		remove_raid(p);
		comm_pool_free(&raid_manager_raid_data_pool, p->data);
		p->data = NULL;
	}
}

int raid_manager::do_team_enter_raid_cost(player_struct *player, uint32_t raid_id)
{
	struct DungeonTable *r_config = get_config_by_id(raid_id, &all_raid_config);	
	
	assert(r_config->DengeonType != 2);
	assert(player->m_team);

	for (int pos = 0; pos < player->m_team->m_data->m_memSize; ++pos)
	{
		player_struct *t_player = player_manager::get_player_by_id(player->m_team->m_data->m_mem[pos].id);
		assert(t_player);
		do_enter_raid_cost(t_player, r_config->CostItemID, r_config->CostNum);
	}
	return (0);
}

int raid_manager::check_player_enter_raid(player_struct *player, uint32_t raid_id)
{
	uint32_t n_reason_player = 0;
	uint64_t reason_player_id[MAX_TEAM_MEM];
	struct DungeonTable *r_config = get_config_by_id(raid_id, &all_raid_config);

	struct SceneResTable *s_config = get_config_by_id(raid_id, &scene_res_config);
	if (!r_config || !s_config)
	{
		LOG_ERR("%s %d: player[%lu] raid[%u]", __FUNCTION__, __LINE__, player->get_uuid(), raid_id);
		return (-1);
	}

	struct ControlTable *control_config = get_config_by_id(r_config->ActivityControl, &all_control_config);
	if (!control_config)
	{
		LOG_ERR("%s %d: player[%lu] raid[%u]", __FUNCTION__, __LINE__, player->get_uuid(), raid_id);
		return (-1);
	}

		//万妖谷小关卡不能直接进入
	if (r_config->DengeonRank == DUNGEON_TYPE_RAND_SLAVE)
	{
		LOG_ERR("%s %d: player[%lu] raid[%u]", __FUNCTION__, __LINE__, player->get_uuid(), raid_id);
		return (-2);
	}

		//当前在副本中
	if (player->scene && player->scene->get_scene_type() == SCENE_TYPE_RAID)
	{
		raid_struct *t_raid = (raid_struct *)(player->scene);
		LOG_ERR("%s %d: player[%lu] raid[%u] already in raid[%u][%lu]", __FUNCTION__, __LINE__,
			player->get_uuid(), raid_id, t_raid->data->ID, t_raid->data->uuid);
		return (-5);
	}

		//当前在位面副本中
	if (player->sight_space)
	{
		LOG_ERR("%s %d: player[%lu] sightspace[%p]", __FUNCTION__, __LINE__, player->get_uuid(), player->sight_space);
		return (-6);
	}

		//检查等级
	if (player->get_attr(PLAYER_ATTR_LEVEL) < s_config->Level)
	{
		LOG_ERR("%s %d: player[%lu] raid[%u]", __FUNCTION__, __LINE__, player->get_uuid(), raid_id);
		return (-10);
	}

	if (player->data->truck.truck_id != 0)
	{
		send_enter_raid_fail(player, 190500305, 0, NULL, 0);
		return -(11);
	}

		//检查时间
	if (control_config->n_OpenDay > 0)
	{
		bool pass = false;
		uint32_t week = time_helper::getWeek(time_helper::get_cached_time() / 1000);
		for (size_t i = 0; i < control_config->n_OpenDay; ++i)
		{
			if (week == control_config->OpenDay[i])
			{
				pass = true;
				break;
			}
		}
		if (pass == false)
		{
			LOG_ERR("%s %d: player[%lu] raid[%u]", __FUNCTION__, __LINE__, player->get_uuid(), raid_id);
			send_enter_raid_fail(player, 8, 0, NULL, 0);
			return (-20);
		}
	}
	assert(control_config->n_OpenTime == control_config->n_CloseTime);
	if (control_config->n_OpenTime > 0)
	{
		bool pass = false;

		uint32_t now = time_helper::get_cached_time() / 1000;

		for (size_t i = 0; i < control_config->n_OpenTime; ++i)
		{
			uint32_t start = time_helper::get_timestamp_by_day(control_config->OpenTime[i] / 100,
				control_config->OpenTime[i] % 100, now);
			uint32_t end = time_helper::get_timestamp_by_day(control_config->CloseTime[i] / 100,
				control_config->CloseTime[i] % 100, now);
			if (now >= start && now <= end)
			{
				pass = true;
				break;
			}
		}
		if (pass == false)
		{
			LOG_ERR("%s %d: player[%lu] raid[%u]", __FUNCTION__, __LINE__, player->get_uuid(), raid_id);
			send_enter_raid_fail(player, 8, 0, NULL, 0);
			return (-20);
		}
	}

	
		//个人副本通过
	if (r_config->DengeonType == 2)
	{
		if (player->m_team)
		{
			LOG_ERR("%s %d: player[%lu] raid[%u]", __FUNCTION__, __LINE__, player->get_uuid(), raid_id);
			return (-30);
		}

		int ret = check_enter_raid_reward_time(player, raid_id, control_config);
		if (ret != 0)
		{
			reason_player_id[0] = player->data->player_id;
			send_enter_raid_fail(player, 10, 1, reason_player_id, 0);
			return (-32);
		}

		//英雄挑战副本检测总收益次数
		if(raidid_to_hero_challenge_config.find(raid_id) != raidid_to_hero_challenge_config.end())
		{
			uint32_t all_challenge_num = 0;
			uint32_t use_challenge_num = 0;
			ParameterTable *param_config = get_config_by_id(161000332, &parameter_config); 
			if(param_config && param_config->n_parameter1 >0)
			{
				all_challenge_num = param_config->parameter1[0];
			}
			for(std::map<uint64_t, ChallengeTable*>::iterator itr = raidid_to_hero_challenge_config.begin(); itr != raidid_to_hero_challenge_config.end(); itr++)
			{
				use_challenge_num += player->get_raid_reward_count(itr->second->DungeonID);
			}
			if(use_challenge_num >= all_challenge_num)
			{
				reason_player_id[0] = player->data->player_id;
				send_enter_raid_fail(player, 10, 1, reason_player_id, 0);
				return (-33);
			}
		}

		ret = check_enter_raid_cost(player, r_config);
		if (ret != 0)
		{
			reason_player_id[0] = player->data->player_id;
			send_enter_raid_fail(player, ret, 1, reason_player_id, r_config->CostItemID);
			return (-35);
		}
		do_enter_raid_cost(player, r_config->CostItemID, r_config->CostNum);

		return (0);
	}

	switch (r_config->DengeonRank)
	{
		case DUNGEON_TYPE_PVP_3: //PVP副本
		case DUNGEON_TYPE_PVP_5: //PVP副本
		case DUNGEON_TYPE_ZHENYING: //阵营战副本
		case DUNGEON_TYPE_BATTLE: //阵营战副本
		case DUNGEON_TYPE_GUILD_WAIT: //帮会等待副本
			return (0);
		default:
			break;
	}
	
		//组队副本检查队伍
	if (!player->m_team)
	{
		LOG_ERR("%s %d: player[%lu] raid[%u] do not have team", __FUNCTION__, __LINE__, player->get_uuid(), raid_id);
		return (-40);
	}

		//是否是队长
	if (player->get_uuid() != player->m_team->GetLeadId())
	{
		LOG_ERR("%s %d: player[%lu] raid[%u]", __FUNCTION__, __LINE__, player->get_uuid(), raid_id);
		return (-50);
	}

		//检查人数
	uint32_t team_mem_num = player->m_team->GetMemberSize();
	if (control_config->MinActor > team_mem_num
		|| control_config->MaxActor < team_mem_num)
	{
		LOG_ERR("%s %d: player[%lu] raid[%u]", __FUNCTION__, __LINE__, player->get_uuid(), raid_id);
		return (-60);
	}

	bool pass = true;
		//检查是否有人离线
	for (int pos = 0; pos < player->m_team->m_data->m_memSize; ++pos)
	{
		if (player->m_team->m_data->m_mem[pos].timeremove != 0)
		{
//			send_enter_raid_fail(player, 7, player->m_team->m_data->m_mem[pos].id, 0);
//			return (-70);
			reason_player_id[n_reason_player++] = player->m_team->m_data->m_mem[pos].id;
			pass = false;
		}
	}
	if (!pass)
	{
		send_enter_raid_fail(player, 7, n_reason_player, reason_player_id, 0);
		return (-70);
	}


		//检查等级, 消耗
	player_struct *team_players[MAX_TEAM_MEM];
	uint32_t i = 0;
	for (int pos = 0; pos < player->m_team->m_data->m_memSize; ++pos)
	{
		player_struct *t_player = player_manager::get_player_by_id(player->m_team->m_data->m_mem[pos].id);
		if (!t_player)
		{
			LOG_ERR("%s: can not find team mem %lu", __FUNCTION__, player->m_team->m_data->m_mem[pos].id);
//			send_enter_raid_fail(player, 2, player->m_team->m_data->m_mem[pos].id, 0);
			return (-50);
		}
		if (t_player->get_attr(PLAYER_ATTR_LEVEL) < s_config->Level)
		{
			pass = false;
			reason_player_id[n_reason_player++] = player->m_team->m_data->m_mem[pos].id;
//			send_enter_raid_fail(player, 2, t_player->get_uuid(), 0);
//			return (-60);
		}

//		int ret = check_enter_raid_cost(t_player, r_config);
//		if (ret != 0)
//		{
//			send_enter_raid_fail(player, ret, t_player->data->player_id, r_config->CostItemID);
//			return (-65);
//		}

		team_players[i++] = t_player;
	}
	if (!pass)
	{
		send_enter_raid_fail(player, 2, n_reason_player, reason_player_id, 0);
		return (-60);
	}

//// 检查收益次数
	for (int pos = 0; pos < player->m_team->m_data->m_memSize; ++pos)
	{
		player_struct *t_player = team_players[pos];
		int ret = check_enter_raid_reward_time(t_player, raid_id, control_config);
		if (ret != 0)
		{
			pass = false;
			reason_player_id[n_reason_player++] = player->m_team->m_data->m_mem[pos].id;
		}

	}
	if (!pass)
	{
		send_enter_raid_fail(player, 10, n_reason_player, reason_player_id, r_config->CostItemID);
		return (-62);
	}
//// 英雄挑战类型副本检测总收益次数
	if(raidid_to_hero_challenge_config.find(raid_id) != raidid_to_hero_challenge_config.end())
	{
		for (int pos = 0; pos < player->m_team->m_data->m_memSize; ++pos)
		{
			player_struct *t_player = team_players[pos];
			uint32_t all_challenge_num = 0;
			uint32_t use_challenge_num = 0;
			ParameterTable *param_config = get_config_by_id(161000332, &parameter_config); 
			if(param_config && param_config->n_parameter1 >0)
			{
				all_challenge_num = param_config->parameter1[0];
			}
			for(std::map<uint64_t, ChallengeTable*>::iterator itr = raidid_to_hero_challenge_config.begin(); itr != raidid_to_hero_challenge_config.end(); itr++)
			{
				use_challenge_num += t_player->get_raid_reward_count(itr->second->DungeonID);
			}
			if(use_challenge_num >= all_challenge_num)
			{

				pass = false;
				reason_player_id[n_reason_player++] = t_player->m_team->m_data->m_mem[pos].id;
			}

		}
	
	}
	if (!pass)
	{
		send_enter_raid_fail(player, 10, n_reason_player, reason_player_id, r_config->CostItemID);
		return (-63);
	}
//// 检查消耗
	int fail_ret;
	for (int pos = 0; pos < player->m_team->m_data->m_memSize; ++pos)
	{
		player_struct *t_player = team_players[pos];
		int ret = check_enter_raid_cost(t_player, r_config);
		if (ret != 0)
		{
			fail_ret = ret;
			pass = false;
			reason_player_id[n_reason_player++] = player->m_team->m_data->m_mem[pos].id;
//			send_enter_raid_fail(player, ret, t_player->data->player_id, r_config->CostItemID);
//			return (-65);
		}
	}
	if (!pass)
	{
		send_enter_raid_fail(player, fail_ret, n_reason_player, reason_player_id, r_config->CostItemID);
		return (-65);
	}
////


//	if (i != team_mem_num)
//	{
//		LOG_ERR("%s %d: player[%lu] raid[%u]", __FUNCTION__, __LINE__, player->get_uuid(), raid_id);
//		return (-70);
//	}

		//检查距离
	struct position *leader_pos = player->get_pos();
//	uint64_t too_far_player_id = 0;
	for (i = 1; i < team_mem_num; ++i)
	{
		player_struct *t_player = team_players[i];

		if (player->scene != t_player->scene)
		{
			pass = false;
			reason_player_id[n_reason_player++] = t_player->get_uuid();
			continue;
		}

		struct position *pos = t_player->get_pos();
		uint64_t x = (pos->pos_x - leader_pos->pos_x);
		uint64_t z = (pos->pos_z - leader_pos->pos_z);
		if (x * x + z * z > max_team_mem_distance)
		{
			pass = false;
			reason_player_id[n_reason_player++] = t_player->get_uuid();
//			too_far_player_id = t_player->get_uuid();
//			send_transfer_to_leader_notify(too_far_player_id);
		}
	}

//	if (too_far_player_id != 0)
	if (!pass)
	{
		send_enter_raid_fail(player, 3, n_reason_player, reason_player_id, 0);
		return (-80);
	}

	// for (int pos = 0; pos < player->m_team->m_data->m_memSize; ++pos)
	// {
	// 	player_struct *t_player = player_manager::get_player_by_id(player->m_team->m_data->m_mem[pos].id);
	// 	do_enter_raid_cost(t_player, r_config->CostItemID, r_config->CostNum);
	// }

	return (0);
}

int raid_manager::send_enter_raid_fail(player_struct *player, uint32_t err_code, uint32_t n_reason_player, uint64_t *reason_player_id, uint32_t item_id)
{
	EnterRaidAnswer resp;
	enter_raid_answer__init(&resp);
	resp.result = err_code;
	resp.n_reson_player_id = n_reason_player;
	resp.reson_player_id = reason_player_id;
	resp.item_id = item_id;

	EXTERN_DATA extern_data;
	extern_data.player_id = player->get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ENTER_RAID_ANSWER, enter_raid_answer__pack, resp);
	return (0);
}

int raid_manager::check_enter_raid_reward_time(player_struct *player, uint32_t id, struct ControlTable *config)
{
	if (config->TimeType != 1)
		return (0);
	uint32_t count = player->get_raid_reward_count(id);
	if (count >= config->RewardTime)
		return 10;
	return (0);
}

int raid_manager::check_enter_raid_cost(player_struct *player, struct DungeonTable *r_config)
{
		//不消耗
	if (r_config->CostItemID == 0)
		return (0);

	switch (get_item_type(r_config->CostItemID))
	{
		case ITEM_TYPE_COIN: //金钱
			if (player->get_attr(PLAYER_ATTR_COIN) < r_config->CostNum)
				return 5;
			break;
		case ITEM_TYPE_BIND_GOLD: //元宝
		case ITEM_TYPE_GOLD:
			if (player->get_comm_gold() < (int)r_config->CostNum)
				return 6;
			break;
		default: //道具
			if (player->get_item_can_use_num(r_config->CostItemID) < (int)r_config->CostNum)
				return 4;
			break;
	}
	return (0);
}


int raid_manager::do_enter_raid_cost(player_struct *player, uint32_t item_id, uint32_t item_num)
{
	if (item_id == 0 || item_num == 0)
		return (0);
	switch (get_item_type(item_id))
	{
		case ITEM_TYPE_COIN: //金钱
			player->sub_coin(item_num, MAGIC_TYPE_ENTER_RAID, true);
			break;
		case ITEM_TYPE_BIND_GOLD: //元宝
		case ITEM_TYPE_GOLD:
			player->sub_comm_gold(item_num, MAGIC_TYPE_ENTER_RAID, true);
			break;
		default: //道具
			player->del_item(item_id, item_num, MAGIC_TYPE_ENTER_RAID, true);
			break;
	}
	return (0);
}

unsigned int raid_manager::get_raid_pool_max_num()
{
	return raid_manager_raid_data_pool.num;
}

raid_struct * raid_manager::get_raid_by_uuid(uint64_t id)
{
	std::map<uint64_t, raid_struct *>::iterator it = raid_manager_all_raid_id.find(id);
	if (it != raid_manager_all_raid_id.end())
		return it->second;

	raid_struct *ret = zhenying_raid_manager::get_zhenying_raid_by_uuid(id);
	if (ret)
		return ret;

	ret = guild_wait_raid_manager::get_guild_wait_raid_by_uuid(id);
	return ret;	
}

raid_struct *raid_manager::create_raid(uint32_t raid_id, player_struct *player)
{
	raid_struct *ret = alloc_raid();
	if (!ret)
		return NULL;
	ret->data->uuid = alloc_raid_uuid();
	ret->m_id = raid_id;
	ret->init_raid(player);

	add_raid(ret);
	return ret;
}

int raid_manager::init_raid_struct(int num, unsigned long key)
{
	raid_struct *raid;
	for (int i = 0; i < num; ++i) {
		raid = new raid_struct();
		raid_manager_raid_free_list.push_back(raid);
	}
	return init_comm_pool(0, sizeof(raid_data), num, key, &raid_manager_raid_data_pool);
}

int raid_manager::reset_all_raid_ai()
{
	{
		std::map<uint64_t, guild_wait_raid_struct *>::iterator it = guild_wait_raid_manager_all_raid_id.begin();
		for (; it != guild_wait_raid_manager_all_raid_id.end(); ++it)
		{
			it->second->raid_set_ai_interface(it->second->data->ai_type);
		}
	}
	{
		std::map<uint64_t, zhenying_raid_struct *>::iterator it = zhenying_raid_manager_all_raid_id.begin();
		for (; it != zhenying_raid_manager_all_raid_id.end(); ++it)
		{
			it->second->raid_set_ai_interface(it->second->data->ai_type);
		}
	}
	{
		std::map<uint64_t, raid_struct *>::iterator it = raid_manager_all_raid_id.begin();
		for (; it != raid_manager_all_raid_id.end(); ++it)
		{
			it->second->raid_set_ai_interface(it->second->data->ai_type);
		}
	}
	
	return (0);
}

void raid_manager::on_tick_10()
{
	for (std::set<raid_struct *>::iterator iter = raid_manager_raid_used_list.begin(); iter != raid_manager_raid_used_list.end(); )
	{
		if ((*iter)->check_raid_need_delete())
		{
			raid_struct *raid = (*iter);
			++iter;
//			if (!raid->m_team)
			delete_raid(raid);
		}
		else
		{
			(*iter)->on_tick();
			++iter;
		}
	}

}
