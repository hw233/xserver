#include "raid.h"
#include "ai.pb-c.h"
#include "so_game_srv/scene_manager.h"
#include "so_game_srv/sight_space_manager.h"
#include "msgid.h"
#include "monster_manager.h"
#include "uuid.h"
#include "team.h"
#include "raid_manager.h"
#include "player_manager.h"
#include "app_data_statis.h"
#include "time_helper.h"
#include "team.h"
#include "collect.h"
#include "scene_transfer.pb-c.h"
#include "raid.pb-c.h"
#include "role.pb-c.h"
#include "relive.pb-c.h"
#include "zhenying.pb-c.h"
#include "zhenying_battle.h"
#include "guild_wait_raid_manager.h"
#include "guild_battle_manager.h"
#include <math.h>

int raid_struct::init_common_script_data(const char *script_name, struct raid_script_data *script_data)
{
	for (int i = 0; i < MAX_SCRIPT_COND_NUM; ++i)
		script_data->cur_finished_num[i] = 0;
	script_data->cur_index = 0;
	script_data->script_config = get_config_by_name((char *)script_name, &all_raid_script_config);
	if (script_data->script_config)
		return (0);
	else
		return (-1);
}

int raid_struct::init_script_data()
{
	int ret = init_common_script_data(m_config->DungeonPass, &SCRIPT_DATA.script_data);
	assert(ret == 0);
	raid_set_ai_interface(8);	
	LOG_INFO("%s: raid[%u][%lu]", __FUNCTION__, data->ID, data->uuid);
	return (0);
}

int	raid_struct::init_wanyaogu_data()
{
	assert(m_config->n_RandomID < 30);
	assert(m_config->RandomNum <= MAX_WANYAOGU_RAID_NUM);
	
	uint64_t all_raid_id[30];
	for (uint32_t i = 0; i < m_config->n_RandomID; ++i)
		all_raid_id[i] = m_config->RandomID[i];
	
	for (uint32_t i = 0; i < m_config->RandomNum; ++i)
	{
		int index = random();
		for (;; ++index)
		{
			index = index % m_config->n_RandomID;
			if (all_raid_id[index] != 0)
			{
				WANYAOGU_DATA.wanyaogu_raid_id[i] = all_raid_id[index];
				all_raid_id[index] = 0;
				break;
			}
		}
	}
//	m_id = all_raid_id[0];
	m_id = WANYAOGU_DATA.wanyaogu_raid_id[0];
	WANYAOGU_DATA.wanyaogu_state = WANYAOGU_STATE_INIT;
	WANYAOGU_DATA.m_config = get_config_by_id(m_id, &all_raid_config);
	assert(WANYAOGU_DATA.m_config);
	init_common_script_data(WANYAOGU_DATA.m_config->DungeonPass, &WANYAOGU_DATA.script_data);
//	WANYAOGU_DATA.m_control_config = get_config_by_id(WANYAOGU_DATA.m_config->ActivityControl, &all_control_config);
//	assert(WANYAOGU_DATA.m_control_config);
	
	raid_set_ai_interface(3);

	if (m_config->DynamicLevel == 0)
	{
		init_scene_struct(m_id, false, 0);		
	}
	else
	{
		init_scene_struct(m_id, false, lv);
	}
	stop_monster_ai();
	return (0);
}

int raid_struct::init_guoyu_raid_data(player_struct *player)
{
	if (player == NULL)
	{
		return -1;
	}
	raid_set_ai_interface(7);
	return 0;
}

int raid_struct::init_guild_raid_data()
{
	raid_set_ai_interface(11);
	return 0;
}
int raid_struct::init_guild_final_raid_data()
{
	raid_set_ai_interface(12);
	return 0;
}


int	raid_struct::init_pvp_raid_data_3()
{
	m_id = sg_3v3_pvp_raid_param1[0];
	raid_set_ai_interface(5);	
	init_scene_struct(m_id, true, lv);
	PVP_DATA.pvp_raid_state = PVP_RAID_STATE_INIT;
//	data->start_time += (15 + 10) * 1000; //等待最多15秒后开始10秒倒计时
	return (0);
}
int	raid_struct::init_pvp_raid_data_5()
{
	m_id = sg_5v5_pvp_raid_param1[0];
	raid_set_ai_interface(6);	
	init_scene_struct(m_id, true, lv);
	PVP_DATA.pvp_raid_state = PVP_RAID_STATE_INIT;
//	data->start_time += (15 + 10) * 1000;  //等待最多15秒后开始10秒倒计时
	return (0);	
}

int raid_struct::init_special_raid_data(player_struct *player)
{
	raid_set_ai_interface(0);
	uint32_t _lv = lv;
	if (m_config->DynamicLevel == 0)
		_lv = 0;

	switch (m_config->DengeonRank)
	{
		case DUNGEON_TYPE_YAOSHI:
			init_guoyu_raid_data(player);
			init_scene_struct(m_id, true, _lv);
			break;
		case DUNGEON_TYPE_RAND_MASTER://万妖谷副本
			init_wanyaogu_data();
			break;
		case DUNGEON_TYPE_PVP_3:
			init_pvp_raid_data_3();
			break;
		case DUNGEON_TYPE_PVP_5:
			init_pvp_raid_data_5();			
			break;
		case DUNGEON_TYPE_SCRIPT:
			init_script_data();
			init_scene_struct(m_id, false, _lv);
			break;
		case DUNGEON_TYPE_GUILD_RAID:
			init_guild_raid_data();
			init_scene_struct(m_id, true, _lv);
			break;
		case DUNGEON_TYPE_GUILD_FINAL_RAID:
			init_guild_final_raid_data();
			init_scene_struct(m_id, true, _lv);
			break;
		case 13:
		{
			raid_set_ai_interface(13);
			init_scene_struct(m_id, true, _lv);
		}
			break;
		case 14:
		{
			raid_set_ai_interface(14);
			init_scene_struct(m_id, true, _lv);
		}
			break;
		case DUNGEON_TYPE_BATTLE:
		case DUNGEON_TYPE_BATTLE_NEW:
		{
			raid_set_ai_interface(DUNGEON_TYPE_BATTLE);
			init_scene_struct(m_id, true, _lv);
		}
			break;
		case DUNGEON_TYPE_MAOGUI_LEYUAN:
		{
			raid_set_ai_interface(18);
			init_scene_struct(m_id, false, _lv);
		}
			break;
		case DUNGEON_TYPE_TOWER:
		{
			raid_set_ai_interface(DUNGEON_TYPE_TOWER);
			init_scene_struct(m_id, true, _lv);
		}
			break;
		default:
			init_scene_struct(m_id, true, _lv);			
			break;
	}
	return (0);
}

void raid_struct::set_raid_lv(player_struct *player)
{
	if (!player || !player->data)
	{
		lv = 0;
		return;
	}
	if (!player->m_team)
	{
		lv = player->get_level();
		return;
	}
	uint32_t all_level = 0;
	uint32_t num = 0;
	for (int pos = 0; pos < player->m_team->m_data->m_memSize; ++pos)
	{
		player_struct *t_player = player_manager::get_player_by_id(player->m_team->m_data->m_mem[pos].id);
		if(t_player)
		{
			all_level += t_player->get_level();
			num++;
		}
	}
	if(num != 0)
	{
		lv = all_level / num;
	}
	else
	{
		lv = 0;
	}
}

int raid_struct::init_raid(player_struct *player)
{
	mark_finished = 0;
	ai = NULL;
	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		m_player[i] = NULL;
		m_player2[i] = NULL;
		m_player3[i] = NULL;
		m_player4[i] = NULL;		
	}
	m_raid_team = NULL;
	m_raid_team2 = NULL;
	m_raid_team3 = NULL;
	m_raid_team4 = NULL;		
	data->ID = m_id;
	player_num = 0;
	set_raid_lv(player);
	data->state = RAID_STATE_START;
	m_monster.clear();
	m_config = get_config_by_id(m_id, &all_raid_config);
	assert(m_config);
	m_control_config = get_config_by_id(m_config->ActivityControl, &all_control_config);
	assert(m_control_config);
	
	data->start_time = time_helper::get_cached_time();
	LOG_DEBUG("%s: raid[%p][%u][%lu] data[%p] lv[%u] curtime = %lu", __FUNCTION__, this, data->ID, data->uuid, data, lv, time_helper::get_cached_time());
	ruqin_data.guild_ruqin = false;
	ruqin_data.zhengying = 0;
	ruqin_data.level = 0;
	ruqin_data.open_time = 0;
	ruqin_data.all_boshu = 0;
	ruqin_data.monster_boshu = 0;
	ruqin_data.monster_id = 0;
	ruqin_data.space_time = 0; 
	ruqin_data.pos_x = 0;
	ruqin_data.pos_z = 0;
	ruqin_data.juli = 0;
	ruqin_data.huodui_time = 0;
	ruqin_data.exp = 0;
	ruqin_data.status = GUILD_RUQIN_ACTIVE_INIT;
	ruqin_data.player_data.clear();

	init_special_raid_data(player);

	if (ai && ai->raid_on_init)
		ai->raid_on_init(this, player);
		
	return (0);
}
uint32_t raid_struct::get_area_width()
{
	return 20;
}

void raid_struct::stop_monster_ai()
{
	for (std::set<monster_struct *>::iterator ite = m_monster.begin(); ite != m_monster.end(); ++ite)
	{
		(*ite)->data->stop_ai = true;
	}
	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		if (m_player[i] && m_player[i]->ai_data)
		{
			m_player[i]->stop_partner_ai();
		}
	}
}
void raid_struct::start_monster_ai()
{
	for (std::set<monster_struct *>::iterator ite = m_monster.begin(); ite != m_monster.end(); ++ite)
	{
		(*ite)->data->stop_ai = false;
	}
	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		if (m_player[i] && m_player[i]->ai_data)
		{
			m_player[i]->start_partner_ai();
		}
	}
}

void raid_struct::stop_player_ai()
{
	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		if (m_player[i] && m_player[i]->ai_data)
		{
			assert(get_entity_type(m_player[i]->get_uuid()) == ENTITY_TYPE_AI_PLAYER);
			m_player[i]->ai_data->stop_ai = true;
			m_player[i]->send_msgid_to_aisrv(AI_SERVER_MSG_ID__STOP_AI);			
		}
		if (m_player2[i] && m_player2[i]->ai_data)
		{
			assert(get_entity_type(m_player2[i]->get_uuid()) == ENTITY_TYPE_AI_PLAYER);			
			m_player2[i]->ai_data->stop_ai = true;
			m_player2[i]->send_msgid_to_aisrv(AI_SERVER_MSG_ID__STOP_AI);
		}
	}
}
void raid_struct::start_player_ai()
{
//	return;
	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		if (m_player[i] && m_player[i]->ai_data)
		{
			assert(get_entity_type(m_player[i]->get_uuid()) == ENTITY_TYPE_AI_PLAYER);						
			m_player[i]->ai_data->stop_ai = false;
			m_player[i]->send_msgid_to_aisrv(AI_SERVER_MSG_ID__START_AI);
		}
		if (m_player2[i] && m_player2[i]->ai_data)
		{
			assert(get_entity_type(m_player2[i]->get_uuid()) == ENTITY_TYPE_AI_PLAYER);						
			m_player2[i]->ai_data->stop_ai = false;
			m_player2[i]->send_msgid_to_aisrv(AI_SERVER_MSG_ID__START_AI);			
		}
	}
}

void raid_struct::clear_monster()
{
	for (std::set<monster_struct *>::iterator ite = m_monster.begin(); ite != m_monster.end(); ++ite)
	{
		monster_manager::delete_monster(*ite);
/*
		monster_struct *monster = *ite;
		switch ((int)monster->get_unit_type())
		{
			case UNIT_TYPE_MONSTER:
				monster_manager::delete_monster(monster);
				break;
			case UNIT_TYPE_BOSS:
				monster_manager::delete_boss((boss_struct *)monster);
				break;
			case UNIT_TYPE_PLAYER:
				assert(0);
		}
*/
	}
	m_monster.clear();

	for (int i = 0; i < m_area_size; ++i)
	{
		m_area[i].cur_monster_num = 0;
	}
	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		if (m_player[i] && m_player[i]->data)
		{
			m_player[i]->send_clear_sight_monster();
			m_player[i]->data->cur_sight_monster = 0;
		}
		if (m_player2[i] && m_player2[i]->data)
		{
			m_player2[i]->send_clear_sight_monster();			
			m_player2[i]->data->cur_sight_monster = 0;
		}
		if (m_player3[i] && m_player3[i]->data)
		{
			m_player3[i]->send_clear_sight_monster();			
			m_player3[i]->data->cur_sight_monster = 0;
		}		
		if (m_player4[i] && m_player4[i]->data)
		{
			m_player4[i]->send_clear_sight_monster();			
			m_player4[i]->data->cur_sight_monster = 0;
		}		
	}
	
}

void raid_struct::team_destoryed(Team *team)
{
	if (m_raid_team == team)
	{
		m_raid_team = NULL;
		data->team_id = 0;
	}
	else if (m_raid_team2 == team)
	{
		m_raid_team2 = NULL;
		data->team2_id = 0;
	}
	else if (m_raid_team3 == team)
	{
		m_raid_team3 = NULL;
		data->team3_id = 0;
	}
	else if (m_raid_team4 == team)
	{
		m_raid_team4 = NULL;
		data->team4_id = 0;
	}
}

void raid_struct::clear()
{
	if (m_config->DengeonRank == DUNGEON_TYPE_BATTLE || m_config->DengeonRank == DUNGEON_TYPE_BATTLE_NEW)
	{
		ZhenyingBattle::GetInstance()->ClearRob(data->ai_data.battle_data.room);
		ZhenyingBattle::GetInstance()->DestroyRoom(data->ai_data.battle_data.room);
	}

	
	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		if (m_player[i] && m_player[i]->data && get_entity_type(m_player[i]->get_uuid()) == ENTITY_TYPE_AI_PLAYER)
		{
			player_manager::delete_player(m_player[i]);
			m_player[i] = NULL;
		}
		if (m_player2[i] && m_player2[i]->data && get_entity_type(m_player2[i]->get_uuid()) == ENTITY_TYPE_AI_PLAYER)
		{
			player_manager::delete_player(m_player2[i]);
			m_player2[i] = NULL;			
		}
		if (m_player3[i] && m_player3[i]->data && get_entity_type(m_player3[i]->get_uuid()) == ENTITY_TYPE_AI_PLAYER)
		{
			player_manager::delete_player(m_player3[i]);
			m_player3[i] = NULL;			
		}
		if (m_player4[i] && m_player4[i]->data && get_entity_type(m_player4[i]->get_uuid()) == ENTITY_TYPE_AI_PLAYER)
		{
			player_manager::delete_player(m_player4[i]);
			m_player4[i] = NULL;			
		}
	}

	scene_struct::clear();
	
	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		assert(m_player[i] == NULL);
		assert(m_player2[i] == NULL);
		assert(m_player3[i] == NULL);
		assert(m_player4[i] == NULL);
		// if (m_player[i] && m_player[i]->is_avaliable())
		// 	m_player[i]->set_out_raid_pos_and_clear_scene();
		// if (m_player2[i] && m_player2[i]->is_avaliable())
		// 	m_player2[i]->set_out_raid_pos_and_clear_scene();
		// if (m_player3[i] && m_player3[i]->is_avaliable())
		// 	m_player3[i]->set_out_raid_pos_and_clear_scene();
		// if (m_player4[i] && m_player4[i]->is_avaliable())
		// 	m_player4[i]->set_out_raid_pos_and_clear_scene();				
	}
	if (m_raid_team)
	{
		m_raid_team->m_data->m_raid_uuid = 0;
		if (data->delete_team1)
		{
			Team::DestroyTeamAndNotify(m_raid_team);
		}
		m_raid_team = NULL;
		data->team_id = 0;
	}
	if (m_raid_team2)
	{
		m_raid_team2->m_data->m_raid_uuid = 0;
		if (data->delete_team2)
		{
			Team::DestroyTeamAndNotify(m_raid_team2);
		}
		m_raid_team2 = NULL;
		data->team2_id = 0;
	}
	if (m_raid_team3)
	{
		m_raid_team3->m_data->m_raid_uuid = 0;
		if (data->delete_team3)
		{
			Team::DestroyTeamAndNotify(m_raid_team3);
		}
		m_raid_team3 = NULL;
		data->team3_id = 0;
	}
	if (m_raid_team4)
	{
		m_raid_team4->m_data->m_raid_uuid = 0;
		if (data->delete_team4)
		{
			Team::DestroyTeamAndNotify(m_raid_team4);
		}
		m_raid_team4 = NULL;
		data->team4_id = 0;
	}

	clear_monster();
}

int raid_struct::team_enter_raid(Team *team)
{
	LOG_DEBUG("%s: raid[%u][%lu], team[%lu]", __FUNCTION__, data->ID, data->uuid, team->GetId());
	data->team_id = team->GetId();
	m_raid_team = team;
	int index = 0;
	assert(res_config);
	for (int i = 0; i < team->m_data->m_memSize; ++i)
	{
		player_struct *t_player = player_manager::get_player_by_id(team->m_data->m_mem[i].id);
		if (!t_player)
		{
			LOG_ERR("%s %d: can not find player[%lu] to enter raid %u", __FUNCTION__, __LINE__, team->m_data->m_mem[i].id, data->ID);
			continue;
		}

		// 	//如果在押镖的位面，就不跟随进副本
		// if (t_player->sight_space != NULL && t_player->sight_space->data->type == 2)
		// {
		// 	continue;
		// }
		player_enter_raid_impl(t_player, index++, res_config->BirthPointX, res_config->BirthPointZ);
	}
	team->m_data->m_raid_uuid = data->uuid;
	return (0);
}
// int raid_struct::team2_enter_raid(Team *team)
// {
// 	LOG_DEBUG("%s: raid[%u][%lu], team[%lu]", __FUNCTION__, data->ID, data->uuid, team->GetId());
// 	data->team2_id = team->GetId();
// 	m_raid_team2 = team;
// 	int index = 0;
// 	assert(res_config);
// 	for (int i = 0; i < team->m_data->m_memSize; ++i)
// 	{
// 		player_struct *t_player = player_manager::get_player_by_id(team->m_data->m_mem[i].id);
// 		if (!t_player)
// 		{
// 			LOG_ERR("%s %d: can not find player[%lu] to enter raid %u", __FUNCTION__, __LINE__, team->m_data->m_mem[i].id, data->ID);
// 			continue;
// 		}
// 		player_enter_raid_impl(t_player, MAX_TEAM_MEM + index, res_config->BirthPointX, res_config->BirthPointZ);
// 		++index;
// 	}
// 	team->m_data->m_raid_uuid = data->uuid;
// 	return (0);
// }
// int raid_struct::team3_enter_raid(Team *team)
// {
// 	LOG_DEBUG("%s: raid[%u][%lu], team[%lu]", __FUNCTION__, data->ID, data->uuid, team->GetId());
// 	data->team3_id = team->GetId();
// 	m_raid_team3 = team;
// 	int index = 0;
// 	assert(res_config);
// 	for (int i = 0; i < team->m_data->m_memSize; ++i)
// 	{
// 		player_struct *t_player = player_manager::get_player_by_id(team->m_data->m_mem[i].id);
// 		if (!t_player)
// 		{
// 			LOG_ERR("%s %d: can not find player[%lu] to enter raid %u", __FUNCTION__, __LINE__, team->m_data->m_mem[i].id, data->ID);
// 			continue;
// 		}
// 		player_enter_raid_impl(t_player, MAX_TEAM_MEM * 2 + index, res_config->BirthPointX, res_config->BirthPointZ);
// 		++index;
// 	}
// 	team->m_data->m_raid_uuid = data->uuid;
// 	return (0);
// }
// int raid_struct::team4_enter_raid(Team *team)
// {
// 	LOG_DEBUG("%s: raid[%u][%lu], team[%lu]", __FUNCTION__, data->ID, data->uuid, team->GetId());
// 	data->team4_id = team->GetId();
// 	m_raid_team4 = team;
// 	int index = 0;
// 	assert(res_config);
// 	for (int i = 0; i < team->m_data->m_memSize; ++i)
// 	{
// 		player_struct *t_player = player_manager::get_player_by_id(team->m_data->m_mem[i].id);
// 		if (!t_player)
// 		{
// 			LOG_ERR("%s %d: can not find player[%lu] to enter raid %u", __FUNCTION__, __LINE__, team->m_data->m_mem[i].id, data->ID);
// 			continue;
// 		}
// 		player_enter_raid_impl(t_player, MAX_TEAM_MEM * 3 + index, res_config->BirthPointX, res_config->BirthPointZ);
// 		++index;
// 	}
// 	team->m_data->m_raid_uuid = data->uuid;
// 	return (0);
// }

int raid_struct::get_free_player_pos()
{
	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		if (data->player_info[i].player_id == 0)
		{
			assert(m_player[i] == NULL);
			return i;
		}
	}
	
	// for (int i = 0; i < MAX_TEAM_MEM; ++i)
	// {
	// 	if (m_player[i])
	// 	{
	// 		assert(data->player_info[i].player_id != 0);
	// 		continue;
	// 	}
	// 	assert(data->player_info[i].player_id == 0);
	// 	return i;
	// }

	// for (int i = 0; i < MAX_TEAM_MEM; ++i)
	// {
	// 	if (m_player2[i])
	// 	{
	// 		assert(data->player_info2[i].player_id != 0);
	// 		continue;
	// 	}
	// 	assert(data->player_info2[i].player_id == 0);
	// 	return i + MAX_TEAM_MEM;
	// }
	
	return (-1);
}

struct raid_player_info *raid_struct::get_raid_player_info(uint64_t player_id, int *pos)
{
	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		if (data->player_info[i].player_id == player_id)
		{
			if (pos)
				*pos = i;
			return &data->player_info[i];
		}
	}

	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		if (data->player_info2[i].player_id == player_id)
		{
			if (pos)
				*pos = i + MAX_TEAM_MEM;
			return &data->player_info2[i];			
		}
	}

	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		if (data->player_info3[i].player_id == player_id)
		{
			if (pos)
				*pos = i + MAX_TEAM_MEM * 2;
			return &data->player_info3[i];			
		}
	}
	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		if (data->player_info4[i].player_id == player_id)
		{
			if (pos)
				*pos = i + MAX_TEAM_MEM * 3;
			return &data->player_info4[i];			
		}
	}
	
	return NULL;
}

// int raid_struct::get_raid_player_pos(uint64_t player_id)
// {
// 	for (int i = 0; i < MAX_TEAM_MEM; ++i)
// 	{
// 		if (data->player_info[i].player_id == player_id)
// 		{
// 			return i;
// 		}
// 	}

// 	for (int i = 0; i < MAX_TEAM_MEM; ++i)
// 	{
// 		if (data->player_info2[i].player_id == player_id)
// 		{
// 			return i + MAX_TEAM_MEM;
// 		}
// 	}
// 	return (-1);
// }

int raid_struct::player_return_raid(player_struct *player)
{
		//万妖谷不返回副本
	if (m_config->DengeonRank == DUNGEON_TYPE_RAND_MASTER)
		return -1;
	
	LOG_DEBUG("%s: raid[%u][%lu], player[%lu]", __FUNCTION__, data->ID, data->uuid, player->get_uuid());
	int index;
	struct raid_player_info *info = get_raid_player_info(player->get_uuid(), &index);
	if (!info)
	{
		LOG_ERR("%s: player[%lu] can not add raid[%u %lu]", __FUNCTION__, player->get_uuid(), data->ID, data->uuid);
		return (-1);
	}

	player_struct **t;
	switch (index / MAX_TEAM_MEM)
	{
		case 0:
			t = m_player;
			break;
		case 1:
			t = m_player2;
			break;
		case 2:
			t = m_player3;
			break;
		case 3:
			t = m_player4;
			break;
		default:
			assert(0);
	}

	t[index % MAX_TEAM_MEM] = player;
	
	// if (index < MAX_TEAM_MEM)
	// {
	// 	m_player[index] = player;
	// }
	// else
	// {
	// 	index -= MAX_TEAM_MEM;
	// 	m_player2[index] = player;
	// }
	
	if (player->data->scene_id != data->ID)
	{
		LOG_ERR("%s: player[%lu] scene_id = %u, it should be %u", __FUNCTION__, player->get_uuid(), player->data->scene_id, data->ID);
		player->data->scene_id = data->ID;
		player->set_pos(res_config->BirthPointX, res_config->BirthPointZ);
	}

		//万妖谷设置成小关卡的位置
	if (m_config->DengeonRank == DUNGEON_TYPE_RAND_MASTER)
	{
		player->data->scene_id = WANYAOGU_DATA.m_config->DungeonID;
		player->set_pos(m_born_x, m_born_z);
	}

	player->data->m_angle = unity_angle_to_c_angle(res_config->FaceY);	

//	player->conserve_out_raid_pos_and_scene(this);
	
	on_player_enter_raid(player);		
	return (0);
}

int raid_struct::set_player_info(player_struct *player, struct raid_player_info *info)
{
	memset(info, 0, sizeof(struct raid_player_info));
	info->player_id = player->get_uuid();
	memcpy(info->name, player->get_name(), sizeof(info->name));
	info->headicon = player->get_attr(PLAYER_ATTR_HEAD);
	info->guild = player->data->guild_id;
	info->lv = player->get_attr(PLAYER_ATTR_LEVEL);
	info->job = player->get_attr(PLAYER_ATTR_JOB);
	return (0);
}

int raid_struct::set_m_player_and_player_info(player_struct *player, int index)
{
	if (!use_m_player())
		return (0);
	
	if (index < 0)
		return 0;
	player_struct **t;
	switch (index / MAX_TEAM_MEM)
	{
		case 0:
			t = m_player;
			set_player_info(player, &data->player_info[index]);
			break;
		case 1:
			t = m_player2;
			set_player_info(player, &data->player_info2[index % MAX_TEAM_MEM]);
			break;
		case 2:
			t = m_player3;
			set_player_info(player, &data->player_info3[index % MAX_TEAM_MEM]);			
			break;
		case 3:
			t = m_player4;
			set_player_info(player, &data->player_info4[index % MAX_TEAM_MEM]);						
			break;
		default:
			assert(0);
	}
	t[index % MAX_TEAM_MEM] = player;
	return (0);
}

int raid_struct::player_enter_raid_impl(player_struct *player, int index, double pos_x, double pos_z, double direct)
{
	uint32_t old_scene = 0;
	set_m_player_and_player_info(player, index);
	
	if (player->sight_space)
		sight_space_manager::del_player_from_sight_space(player->sight_space, player, false);
	if (player->scene)
	{
		old_scene = player->scene->m_id;
		player->scene->player_leave_scene(player);
	}
	player->conserve_out_raid_pos_and_scene(m_config);
	player->set_enter_raid_pos_and_scene(this, pos_x, pos_z);

	if (get_entity_type(player->get_uuid()) == ENTITY_TYPE_PLAYER)
	{
		EXTERN_DATA extern_data;
		extern_data.player_id = player->get_uuid();

		EnterRaidNotify notify;
		enter_raid_notify__init(&notify);
		notify.raid_id = m_id;
		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ENTER_RAID_NOTIFY, enter_raid_notify__pack, notify);

		if (direct == 0)
		{
			direct = res_config->FaceY;
		}
		player->data->m_angle = unity_angle_to_c_angle(direct);
		player->send_scene_transfer(direct, pos_x, res_config->BirthPointY,
			pos_z, old_scene, m_id, 0);
	}
	else
	{
		add_player_to_scene(player);
		if (ai && ai->raid_on_player_ready)
			ai->raid_on_player_ready(this, player);
	}
	
	on_player_enter_raid(player);	

	return (0);
}

// int raid_struct::player_enter_raid(player_struct *player, double pos_x, double pos_z, double direct)
// {
// 	LOG_DEBUG("%s: raid[%u][%lu], player[%lu]", __FUNCTION__, data->ID, data->uuid, player->get_uuid());
// 	int index = get_free_player_pos();
// 	if (index < 0)
// 	{
// 		LOG_ERR("%s: player[%lu] can not add raid[%u %lu]", __FUNCTION__, player->get_uuid(), data->ID, data->uuid);
// 		return (-1);
// 	}
// 	return player_enter_raid_impl(player, index, pos_x, pos_z, direct);
// }

// int raid_struct::add_player_to_scene(player_struct *player)
// {
// 	return scene_struct::add_player_to_scene(player);
// }
int raid_struct::add_collect_to_scene(Collect *pCollect)
{
	pCollect->m_raid_uuid = data->uuid;
	return scene_struct::add_collect_to_scene(pCollect);	
}
// int raid_struct::delete_player_from_scene(player_struct *player)
// {
// //	player->data->raid_uuid = 0;
// 	return scene_struct::delete_player_from_scene(player);	
// }
// int raid_struct::delete_collect_to_scene(Collect *pCollect)
// {
// //	pCollect->m_raid_uuid = 0;
// 	return scene_struct::delete_collect_to_scene(pCollect);		
// }

int raid_struct::delete_monster_from_scene(monster_struct *monster, bool send_msg)
{
	LOG_DEBUG("%s: raid[%u][%lu], monster[%p][%u][%lu]", __FUNCTION__, data->ID,
		data->uuid, monster, monster->data->monster_id, monster->get_uuid());
	int ret = scene_struct::delete_monster_from_scene(monster, send_msg);
	if (ret != 0)
		return ret;
		//因为召唤怪，宠物的死亡不会经过on_monster_dead, 所以从m_monster.erase移到这里来了
	assert(m_monster.find(monster) != m_monster.end());
	m_monster.erase(monster);
	return ret;
}

int raid_struct::add_monster_to_scene(monster_struct *monster, uint32_t effectid)
{
	LOG_DEBUG("%s: raid[%u][%lu], monster[%u][%lu]", __FUNCTION__, data->ID,
		data->uuid, monster->data->monster_id, monster->get_uuid());
	int ret = scene_struct::add_monster_to_scene(monster, effectid);
	if (ret != 0)
		return ret;
	assert(m_monster.find(monster) == m_monster.end());

	m_monster.insert(monster);
	monster->data->raid_uuid = data->uuid;

	if(ai && ai->raid_on_monster_relive)
	{
		ai->raid_on_monster_relive(this, monster);
	}

	// if (monster->config->Camp == 2)
	// {
	// 	monster->set_camp_id(ZHENYING__TYPE__FULONGGUO);
	// }
	// else if (monster->config->Camp == 3)
	// {
	// 	monster->set_camp_id(ZHENYING__TYPE__WANYAOGU);
	// }
	return ret;
}

int raid_struct::clear_m_player_and_player_info(player_struct *player, bool clear_player_info)
{
	if (!use_m_player())
		return (0);

	LOG_DEBUG("%s: raid[%u][%lu], player[%lu]", __FUNCTION__, data->ID, data->uuid, player->get_uuid());
	int index;
	if (!get_raid_player_info(player->get_uuid(), &index))
	{
		LOG_ERR("%s: player[%lu] not in raid[%u %lu]", __FUNCTION__, player->get_uuid(), data->ID, data->uuid);
		return (-1);
	}

	player_struct **t;
	switch (index / MAX_TEAM_MEM)
	{
		case 0:
			t = m_player;
			if (clear_player_info)
				data->player_info[index].player_id = 0;
			break;
		case 1:
			t = m_player2;
			if (clear_player_info)			
				data->player_info2[index].player_id = 0;
			break;
		case 2:
			t = m_player3;
			if (clear_player_info)						
				data->player_info3[index].player_id = 0;
			break;
		case 3:
			t = m_player4;
			if (clear_player_info)									
				data->player_info4[index].player_id = 0;
			break;
		default:
			assert(0);
	}
	t[index % MAX_TEAM_MEM] = NULL;
	
	return (0);
}

int raid_struct::player_offline(player_struct *player)
{
	LOG_DEBUG("%s: player[%lu] leave raid %u[%lu]", __FUNCTION__, player->get_uuid(), data->ID, data->uuid);
	if (clear_m_player_and_player_info(player, false) != 0)
		return -1;

	on_player_leave_raid(player);	
	//新手副本玩家下线重置玩家属性
	if( data != NULL && player != NULL && data->ID == 20035)
	{		
		player->calculate_attribute(true);
		player->set_attr(PLAYER_ATTR_HP, player->data->attrData[PLAYER_ATTR_MAXHP]);
		player->data->buff_fight_attr[PLAYER_ATTR_HP] = player->data->attrData[PLAYER_ATTR_HP];
		player->broadcast_one_attr_changed(PLAYER_ATTR_HP, player->data->attrData[PLAYER_ATTR_HP], false, true);
		
	}
	
		//单人副本，直接删除
//	if (m_config->DengeonType == 2)
//	{
//		player->set_out_raid_pos_and_clear_scene();
//		raid_manager::delete_raid(this);
//	}
//	else //多人副本通知其他人
//	{
		delete_player_from_scene(player);
//	}
	return (0);
}

int raid_struct::check_all_monster_region_buff(struct RaidScriptTable *config)
{
	if (config->n_Parameter1 == 0)
		return (0);

	if (!ai || !ai->raid_on_monster_region_changed)
		return (0);
	
	for (std::set<monster_struct *>::iterator ite = m_monster.begin(); ite != m_monster.end(); ++ite)
	{
		monster_struct *monster = *ite;
		if (!monster->data || monster->mark_delete)
			continue;
		if (!monster->is_alive())
			continue;
		if (monster->get_attr(PLAYER_ATTR_REGION_ID) != config->Parameter1[0])
			continue;
		ai->raid_on_monster_region_changed(this, monster, 1, config->Parameter1[0]);
	}
	return (0);
}

int raid_struct::get_id_collect_num(uint32_t id)
{
	int ret = 0;
	for (std::set<uint64_t>::iterator ite = m_collect.begin(); ite != m_collect.end(); ++ite)
	{
		Collect *collect = Collect::GetById(*ite);
		if (!collect)
			continue;
		if (collect->m_collectId == id)
			++ret;
	}
	return (ret);
}

bool raid_struct::is_monster_alive(uint32_t id)
{
	for (std::set<monster_struct *>::iterator ite = m_monster.begin(); ite != m_monster.end(); ++ite)
	{
		if ((*ite)->data->monster_id == id)
			return true;
	}
	return false;
}

int raid_struct::get_id_monster_num(uint32_t id)
{
	int ret = 0;
	for (std::set<monster_struct *>::iterator ite = m_monster.begin(); ite != m_monster.end(); ++ite)
	{
		if ((*ite)->data->monster_id == id)
			++ret;
	}
	return (ret);
}

int raid_struct::player_leave_raid(player_struct *player)
{
	if (player_leave_scene(player) != 0)
	{
		LOG_ERR("%s: player[%lu] leave raid %u failed", __FUNCTION__, player->get_uuid(), m_id);
		return -1;
	}
#ifdef __RAID_SRV__
	EXTERN_DATA extern_data;
	extern_data.player_id = player->get_uuid();
	fast_send_msg_base(&conn_node_gamesrv::connecter, &extern_data, SERVER_PROTO_RAID_LEAVE_REQUEST, 0, 0);
	player_manager::delete_player(player);
#else	
	if (player->data->scene_id <= SCENCE_DEPART)
	{
		struct position *pos = player->get_pos();
		uint32_t scene_id = player->data->scene_id;

		scene_struct *new_scene = scene_manager::get_scene(scene_id);
		if (!new_scene)
		{
			LOG_ERR("%s %d: player[%lu] transfer to the wrong scene[%u]", __FUNCTION__, __LINE__, player->data->player_id, scene_id);
			return (-30);
		}
		new_scene->player_enter_scene(player, pos->pos_x, player->data->pos_y, pos->pos_z, player->data->leaveraid.direct);		
	}
	else
	{
		DungeonTable *config = get_config_by_id(player->data->scene_id, &all_raid_config);
		if (config)
			player->move_to_raid_impl(config, true);
	}
#endif
	return (0);
}

// int raid_struct::player_leave_raid(player_struct *player)
// {
// 	if (get_entity_type(player->get_uuid()) == ENTITY_TYPE_AI_PLAYER)
// 	{
// 		LOG_ERR("%s: ai player[%lu] can not leave raid[%u] uuid[%lu]", __FUNCTION__, player->get_uuid(), m_id, data->uuid);
// 		return (-1);
// 	}

// //	get_raid_player_info(player->get_uuid(), NULL);
	
// 	LOG_DEBUG("%s: player[%lu] leave raid %u[%lu]", __FUNCTION__, player->get_uuid(), data->ID, data->uuid);
// 	if (clear_m_player_and_player_info(player, true) != 0)
// 		return -1;
// //	int index = get_player_pos(player);
// //	if (index < 0)
// //	{
// //		LOG_ERR("%s: player[%lu] not in raid[%u %lu]", __FUNCTION__, player->get_uuid(), data->ID, data->uuid);
// //		return (-1);
// //	}
// 	if (player->scene != this)
// 	{
// 		LOG_ERR("%s: player[%lu] not in raid[%u %lu]", __FUNCTION__, player->get_uuid(), data->ID, data->uuid);
// 		return (-1);
// 	}
// //	//新手副本，删除玩家不死buff
// //	if (player->scene->m_id == 20035)
// //	{
// //		player->clear_one_buff(114400018);
// //	}
// 	player->set_out_raid_pos_and_clear_scene();
// //	m_player[index] = NULL;
// //	data->player_info[index].player_id = 0;
// 	delete_player_from_scene(player);

// 	on_player_leave_raid(player);

// 		//单人副本，直接删除
// //	if (m_config->DengeonType == 2 && m_config->DengeonRank != DUNGEON_TYPE_ZHENYING)
// //	{
// //		raid_manager::delete_raid(this);
// //	}

// 	//如果是在帮战副本里退帮，把玩家拉到野外场景
// 	if (is_guild_battle_raid())
// 	{
// 		if (player->data->guild_id == 0)
// 		{
// 			player->data->scene_id = player->data->last_scene_id;
// 			float pos_x = 0.0, pos_y = 0.0, pos_z = 0.0, face_y = 0.0;
// 			get_scene_birth_pos(player->data->scene_id, &pos_x, &pos_y, &pos_z, &face_y);
// 			player->set_pos(pos_x, pos_z);
// 			player->data->m_angle = unity_angle_to_c_angle(face_y);
			
// 			EXTERN_DATA extern_data;
// 			extern_data.player_id = player->get_uuid();	
// 			fast_send_msg_base(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_LEAVE_RAID_NOTIFY, 0, 0);

// 			player->data->m_angle = unity_angle_to_c_angle(face_y);			
// 			player->send_scene_transfer(face_y, pos_x, pos_y, pos_z, player->data->scene_id, 0);
// 			return 0;
// 		}
// 	}

// 		//帮会战返回帮会准备副本
// 	if (m_config->DengeonRank == DUNGEON_TYPE_GUILD_RAID || m_config->DengeonRank == DUNGEON_TYPE_GUILD_FINAL_RAID)
// 	{
// 		if (is_guild_battle_opening())
// 		{
// 			if (guild_wait_raid_manager::add_player_to_guild_wait_raid(player))
// 				return (0);
// 		}
// 		else
// 		{
// 			struct DungeonTable* r_config = get_config_by_id(m_config->ExitScene, &all_raid_config);
// 			if (r_config)
// 			{
// 				player->data->scene_id = r_config->ExitScene;
// 				player->set_pos(r_config->ExitPointX, r_config->BirthPointZ);
// 				player->data->m_angle = r_config->BirthPointY;

// 				EXTERN_DATA extern_data;
// 				extern_data.player_id = player->get_uuid();	
// 				fast_send_msg_base(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_LEAVE_RAID_NOTIFY, 0, 0);

// 				player->send_scene_transfer(r_config->FaceY, r_config->ExitPointX, r_config->BirthPointY,
// 						r_config->BirthPointZ, r_config->ExitScene, 0);
// 				return 0;
// 			}
// 		}
// 	}
	
// 	// else
// 	// {
// 	// 		//组队副本如果没结束，那么踢出队伍
// 	// 	if (data->state == RAID_STATE_START && player->m_team)
// 	// 	{
// 	// 		player->m_team->RemoveMember(*player, true);
// 	// 	}
// 	// }

// 	EXTERN_DATA extern_data;
// 	extern_data.player_id = player->get_uuid();	
// 	fast_send_msg_base(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_LEAVE_RAID_NOTIFY, 0, 0);

// 	player->data->m_angle = unity_angle_to_c_angle(player->data->leaveraid.direct);
// 	player->send_scene_transfer(player->data->leaveraid.direct, player->data->leaveraid.ExitPointX, player->data->leaveraid.ExitPointY,
// 		player->data->leaveraid.ExitPointZ, player->data->leaveraid.scene_id, 0);
// 	return (0);
// }

SCENE_TYPE_DEFINE raid_struct::get_scene_type()
{
	return SCENE_TYPE_RAID;
}

bool raid_struct::check_raid_failed()
{
	if (m_config->DengeonRank == DUNGEON_TYPE_ZHENYING)
	{
		return false; //阵营战
	}
	struct DungeonTable *t_config = get_raid_config();
	if (!t_config)
		return false;
	for (size_t i = 0; i < t_config->n_FailType; ++i)
	{
		switch (t_config->FailType[i])
		{
			case 1: //限时
			{
				if (time_helper::get_cached_time() > data->start_time)
				{
					uint32_t time_escape = (time_helper::get_cached_time() - data->start_time) / 1000;
					if (time_escape >= t_config->FailValue[i])
						return true;
				}
			}
			break;
			case 2: //检测指定怪物ID是否仍然存活
			{
				uint32_t num = get_id_monster_num(t_config->FailValue[i]);
				if (num < t_config->FailValue1[i])
					return true;
			}
			break;
		}
	}
	return false;
}

int raid_struct::check_cond_finished(int index, uint64_t cond_type, uint64_t cond_value, uint64_t cond_value1, uint32_t *ret_param)
{
	assert(index >= 0 && index < 3);
	switch (cond_type)
	{
		case 1://通关时间
			{
				*ret_param = (time_helper::get_cached_time() - data->start_time) / 1000;
				if (*ret_param <= cond_value)
					return (1);
				else
					return (0);
			}
			break;
		case 2: //击杀怪物
		{
			if (data->star_param[index] >= cond_value1)
			{
				*ret_param = cond_value1;
				return (1);
			}
			else
			{
				*ret_param = data->star_param[index];
				return (0);
			}
		}
		break;
		case 3: //怪物存活
		{
			if (data->star_param[index] >= cond_value1)
			{
				*ret_param = cond_value1;
				return (0);
			}
			else
			{
				*ret_param = data->star_param[index];
				return (1);
			}
		}
		break;
		case 4: //通关副本
		{
			*ret_param = 0;
			return (1);
		}
		break;
		case 5: //击杀特殊颜色狰狞猫鬼让其他猫鬼瞬间死亡
		{
			if (data->star_param[index] >= cond_value)
			{
				*ret_param = cond_value1;  //要求显示后面那个数字 
				return (1);
			}
			else
			{
//				*ret_param = data->star_param[index];
				*ret_param = 0;
				return (0);
			}
		}
		break;
		case 6: //猫鬼王召唤第N次小怪前将它击杀
		{
			assert(data->ai_type == 18);
			if (data->ai_data.maogui_data.raid_finished &&
				data->ai_data.maogui_data.zhaohuan_num <= cond_value)
			{
				*ret_param = cond_value1;
				return (1);
			}
			else
			{
//				*ret_param = data->ai_data.maogui_data.zhaohuan_num;
				*ret_param = 0;
				return (0);
			}
		}
		break;
		// case 2://死亡次数
		// {
		// 	*star_param = data->dead_count;
		// 	if (*star_param <= m_config->ScoreValue[i])
		// 		++ret;
		// }
		// break;
		// case 3://伤害数值
		// {
		// 	*star_param = 0;
		// 	for (int j = 0; j < MAX_TEAM_MEM; ++j)
		// 	{
		// 		*star_param += data->player_info[j].damage;
		// 	}
		// 	if (*star_param >= m_config->ScoreValue[i])
		// 		++ret;
		// }
		// break;
		// case 4://受到伤害
		// {
		// 	*star_param = 0;
		// 	for (int j = 0; j < MAX_TEAM_MEM; ++j)
		// 	{
		// 		*star_param += data->player_info[j].injured;
		// 	}
		// 	if (*star_param <= m_config->ScoreValue[i])
		// 		++ret;
		// }
		// break;
		// default:
		// {
		// 	*star_param = 0;
		// 	return 0;
		// }
	}
	return (0);
}

int raid_struct::calc_raid_star(uint32_t star_param[3], uint32_t score_param[3])
{
	if (m_config->n_ScoreValue != 3 || m_config->n_Score != 3)
	{
		LOG_ERR("%s: raid %u n_scorevalue = %u n_score = %u", __FUNCTION__, data->ID, m_config->n_ScoreValue, m_config->n_Score);
		*star_param = 0;
		return 0;
	}
	int ret = 0;
	data->star_bits = 0;
	for (int i = 0; i < 3; ++i)
	{
		star_param[i] = 0;
		score_param[i] = 0;
	}

	for (uint32_t i = 0; i < m_config->n_Score; ++i)
	{
		if (check_cond_finished(i, m_config->Score[i], m_config->ScoreValue[i], m_config->ScoreValue1[i], &score_param[i]) == 1)
		{
			data->star_bits |= (1 << i);
			++star_param[i];
			++ret;
		}
	}
	return (ret);
}

int raid_struct::on_raid_failed(uint32_t score_param)
{
	mark_finished = 1;	
	LOG_DEBUG("%s: raid[%u][%lu] curtime = %lu", __FUNCTION__, data->ID, data->uuid, time_helper::get_cached_time());
// 	clear_monster();
// 	data->state = RAID_STATE_PASS;

// 	RaidFinishNotify notify;
// 	raid_finish_notify__init(&notify);
// 	notify.result = -1;
// 	notify.raid_id = data->ID;
// //	notify.n_star = ;
// //	notify.star = 0;
// //	notify.score_param = score_param;

// 	broadcast_to_raid(MSG_ID_RAID_FINISHED_NOTIFY, &notify, (pack_func)raid_finish_notify__pack);
	return (0);
}

void raid_struct::on_player_leave_raid(player_struct *player)
{
		//如果死亡，就给予复活
	// if (!player->is_alive())
	// {
	// 	player->relive();
	// }
	
//	player->data->player_raid_uuid = 0;
	assert(get_entity_type(player->get_uuid()) == ENTITY_TYPE_PLAYER);
	--player_num;
	if (is_guild_battle_raid())
	{
		check_guild_participate_num(player->data->guild_id);
	}
	if (ai && ai->raid_on_player_leave)
		ai->raid_on_player_leave(this, player);	
}

void raid_struct::on_player_enter_raid(player_struct *player)
{
	player->data->player_raid_uuid = data->uuid;
	if (get_entity_type(player->get_uuid()) == ENTITY_TYPE_PLAYER)	
	{
		++player_num;
		if (is_guild_battle_raid())
		{
			check_guild_participate_num(player->data->guild_id);
		}
	}
	if (ai && ai->raid_on_player_enter)
		ai->raid_on_player_enter(this, player);
}

int raid_struct::on_raid_finished()
{
	mark_finished = 2;
	LOG_DEBUG("%s: raid[%u][%lu]", __FUNCTION__, data->ID, data->uuid);
//	if (ai && ai->raid_on_finished)
//		ai->raid_on_finished(this);

// 	clear_monster();

// 	uint32_t star_param[3];
// 	uint32_t score_param[3];
// 	uint32_t star = calc_raid_star(star_param, score_param);
// 	if (star <= 0)
// 	{
// 		on_raid_failed(0);
// 		return (0);
// 	}

// 	if (star > m_config->n_Rewards)
// 		star = 1;

// 	RaidFinishNotify notify;
// 	raid_finish_notify__init(&notify);
// 	notify.result = 0;
// 	notify.raid_id = data->ID;
// 	notify.n_star = 3;
// 	notify.star = star_param;
// 	notify.n_score_param = 3;
// 	notify.score_param = score_param;

// 	std::map<uint32_t, uint32_t> item_list;
// 	int n_item = 0;
// 	uint32_t item_id[MAX_ITEM_REWARD_PER_RAID];
// 	uint32_t item_num[MAX_ITEM_REWARD_PER_RAID];
// 	uint32_t gold = 0, exp = 0;


// //	for (size_t i = 0; i < star; ++i)
// //	{
// //		get_drop_item(m_config->Rewards[i], item_list);
// 		if (get_drop_item(m_config->Rewards[star - 1], item_list) == 0)
// //		if (get_drop_item(m_config->Rewards[i], item_list) == 0)
// 		{
// 			for (std::map<uint32_t, uint32_t>::iterator ite = item_list.begin(); ite != item_list.end() && n_item < MAX_ITEM_REWARD_PER_RAID; ++ite)
// 			{
// 				int type = get_item_type(ite->first);
// 				switch (type)
// 				{
// 					case ITEM_TYPE_COIN:
// 						gold += ite->second;
// 						break;
// 					case ITEM_TYPE_EXP:
// 						exp += ite->second;
// 						break;
// 					default:
// 						item_id[n_item] = ite->first;
// 						item_num[n_item] = ite->second;
// 						++n_item;
// 				}
// 			}
// 		}
// //	}
// /*
// 	for (std::map<uint32_t, uint32_t>::iterator ite = item_list.begin(); ite != item_list.end() && n_item < MAX_ITEM_REWARD_PER_RAID; ++ite)
// 	{
// 		int type = get_item_type(ite->first);
// 		switch (type)
// 		{
// 			case ITEM_TYPE_COIN:
// 				notify.gold += ite->second;
// 				break;
// 			case ITEM_TYPE_EXP:
// 				notify.exp += ite->second;
// 				break;
// 			default:
// 				item_id[n_item] = ite->first;
// 				item_num[n_item] = ite->second;
// 				++n_item;
// 		}
// 	}
// */
// 	notify.item_id = &item_id[0];
// 	notify.item_num = &item_num[0];

// //	broadcast_to_raid(MSG_ID_RAID_FINISHED_NOTIFY, &notify, (pack_func)raid_finish_notify__pack);
// 	data->state = RAID_STATE_PASS;

// 	EXTERN_DATA extern_data;
// 	for (int i = 0; i < MAX_TEAM_MEM; ++i)
// 	{
// 		if (!m_player[i])
// 			continue;
// 		extern_data.player_id = m_player[i]->get_uuid();

// 		uint32_t num = m_player[i]->get_raid_reward_count(data->ID);
// 		if (m_config->RewardTime > num)
// 		{
// 			notify.n_item_id = notify.n_item_num = n_item;
// 			notify.gold = gold;
// 			notify.exp = exp;
// 			fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_RAID_FINISHED_NOTIFY, raid_finish_notify__pack, notify);
// 			m_player[i]->add_item_list(item_list, MAGIC_TYPE_RAID, ADD_ITEM_SEND_MAIL_WHEN_BAG_FULL, true);
// 			m_player[i]->add_raid_reward_count(data->ID);
// 		}
// 		else
// 		{
// 			notify.n_item_id = notify.n_item_num = 0;
// 			notify.gold = 0;
// 			notify.exp = 0;
// 			fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_RAID_FINISHED_NOTIFY, raid_finish_notify__pack, notify);
// 		}
// 	}

	return (0);
}

player_struct *raid_struct::get_team_leader_player()
{
	if (!m_raid_team)
	{
		return m_player[0];
	}
	return m_raid_team->GetLead();
}

struct DungeonTable *raid_struct::get_raid_config()
{
	if (ai && ai->raid_get_config)
	{
		return ai->raid_get_config(this);
	}
	return m_config;
}

extern struct raid_ai_interface *all_raid_ai_interface[MAX_RAID_AI_INTERFACE];
void raid_struct::raid_set_ai_interface(int ai_type)
{
	if (ai_type >= 0 && ai_type < MAX_RAID_AI_INTERFACE)
	{
		data->ai_type = ai_type;
		ai = all_raid_ai_interface[ai_type];
	}
	else
	{
		data->ai_type = -1;
		ai = NULL;
	}
}

void raid_struct::raid_add_ai_interface(int ai_type, struct raid_ai_interface *ai)
{
	assert(ai_type >= 0 && ai_type < MAX_RAID_AI_INTERFACE);

	// if (ai)
	// {
	// 	assert(ai->raid_on_init);
	// }
	
//	assert(ai->on_tick);
//	assert(ai->on_player_enter);
//	assert(ai->on_player_leave);
//	assert(ai->on_player_dead);
//	assert(ai->on_player_relive);
//	assert(ai->on_monster_dead);

	all_raid_ai_interface[ai_type] = ai;
}

bool raid_struct::check_can_add_team_mem(player_struct *player)
{
//	return false;
	return scene_can_make_team(m_id);
	
	if (!m_config || !res_config)
		return false;

		//单人副本 组队玩家不能进单人副本，那么单人副本也不能组队
	if (m_config->DengeonType == 2)
		return false;
		//不允许中途加人
	if (m_config->AddMidway == 0)
		return false;

		// TODO: 计算消耗怎么办?
	if (raid_manager::check_player_enter_raid(player, data->ID) != 0)
		return false;
	
	// 	//当前在副本中
	// if (player->scene && player->scene->get_scene_type() == SCENE_TYPE_RAID)
	// 	return false;
	// 	//检查等级
	// if (player->get_attr(PLAYER_ATTR_LEVEL) < res_config->level)
	// 	return false;

	return true;
}

bool raid_struct::check_raid_need_delete()
{
	return get_cur_player_num() == 0;
}

static void do_raid_failed(raid_struct *raid)
{
	raid->mark_finished = 999;
	if (raid->ai && raid->ai->raid_on_failed)
	{
		raid->ai->raid_on_failed(raid);
	}
	else
	{
		raid->clear_monster();
		raid->data->state = RAID_STATE_PASS;
		RaidFinishNotify notify;
		raid_finish_notify__init(&notify);
		notify.result = -1;
		notify.raid_id = raid->data->ID;
		raid->broadcast_to_raid(MSG_ID_RAID_FINISHED_NOTIFY, &notify, (pack_func)raid_finish_notify__pack);
	}
}

void raid_struct::on_tick()
{
	switch (mark_finished)
	{
		case 0:  //没结束
		{
			if (ai && ai->raid_on_tick)
				ai->raid_on_tick(this);

			if (check_raid_failed())
			{
				do_raid_failed(this);
				return;
			}

			struct DungeonTable *t_config = get_raid_config();
			if (!t_config)
				return;
			if (data->pass_index < t_config->n_PassType)
			{
				switch (t_config->PassType[data->pass_index])
				{
					case 2:  //达到时间通关类型，单位秒
					{
						int t = (time_helper::get_cached_time() - data->start_time) / 1000;
						if ((int)(t_config->PassValue[data->pass_index]) <= t)
						{
							LOG_DEBUG("%s: raid[%lu][%p] time pass, start_time[%lu] [%lu][%d]", __FUNCTION__, data->uuid, this, data->start_time,
									  t_config->PassValue[data->pass_index], t);
							add_raid_pass_value(2, t_config);
						}
					}
					break;
					case 5:  //玩家到达某个坐标点
					{
						float pos_x = (int)t_config->PassValue[data->pass_index];
						float pos_z = (int)t_config->PassValue1[data->pass_index];						
						for (int i = 0; i < MAX_TEAM_MEM; ++i)
						{
							if (!m_player[i] || !m_player[i]->is_avaliable())
								continue;
							struct position *pos = m_player[i]->get_pos();
							if (fabsf(pos->pos_x - pos_x) <= 2 &&
								fabsf(pos->pos_z - pos_z) <= 2)
							{
								add_raid_pass_value(5, t_config);								
								break;
							}
						}
					}
					break;
				}
			}
		}
		break;
		case 1: //失败了
		{
			do_raid_failed(this);
		}
		break;
		case 2: //成功了
		{
			mark_finished = 999;
			if (ai && ai->raid_on_finished)
				ai->raid_on_finished(this);
		}
		break;
		default:
		{
		}
		break;
	}
}

void raid_struct::broadcast_player_hit_statis_changed(struct raid_player_info *info, player_struct *player)
{
	RaidHitStatisChangedNotify notify;
	raid_hit_statis_changed_notify__init(&notify);
	notify.player_id = info->player_id;
	notify.cure = info->cure;
	notify.damage = info->damage;
	notify.injured = info->injured;
	notify.guild = info->guild;
//	notify.job = player->get_attr(PLAYER_ATTR_JOB);
//	notify.lv = player->get_attr(PLAYER_ATTR_LEVEL);
//	notify.name = player->data->name;
	broadcast_to_raid(MSG_ID_RAID_HIT_STATIS_CHANGED_NOTIFY, &notify, (pack_func)raid_hit_statis_changed_notify__pack);
}

void raid_struct::on_player_attack(player_struct *player, unit_struct *target, int damage)
{
	if (ai && ai->raid_on_player_attack)
		ai->raid_on_player_attack(this, player, target, damage);

	struct raid_player_info *info = get_raid_player_info(player->get_uuid(), NULL);
	if (info)
	{
		info->damage += damage;
		broadcast_player_hit_statis_changed(info, player);
	}		
}
void raid_struct::on_monster_attack(monster_struct *monster, player_struct *player, int damage)
{
	struct raid_player_info *info = get_raid_player_info(player->get_uuid(), NULL);
	if (!info)
		return;
	info->injured += damage;
	broadcast_player_hit_statis_changed(info, player);

	if (monster->ai && monster->ai->on_raid_attack_player)
		monster->ai->on_raid_attack_player(monster, player, damage);

}

void raid_struct::on_player_dead(player_struct *player, unit_struct *killer)
{
	LOG_DEBUG("%s: raid[%u][%lu], player[%lu]", __FUNCTION__, data->ID, data->uuid, player->get_uuid());
	if (m_config->DengeonRank != DUNGEON_TYPE_ZHENYING
		&& m_config->DengeonRank != DUNGEON_TYPE_BATTLE
		&& m_config->DengeonRank != DUNGEON_TYPE_BATTLE_NEW
		&& m_config->DengeonRank != DUNGEON_TYPE_GUILD_LAND)
	{
		++data->dead_count;

		struct raid_player_info *info = get_raid_player_info(player->get_uuid(), NULL);
		assert(info);
		++info->dead_count;
	}
	

	if (ai && ai->raid_on_player_dead)
		ai->raid_on_player_dead(this, player, killer);
}

void raid_struct::delete_raid_collect_safe(uint32_t uuid)
{
	std::set<uint64_t>::iterator ite = m_collect.find(uuid);
	if (ite == m_collect.end())
		return;
	m_collect.erase(ite);
	Collect::DestroyCollect(uuid);	
}

void raid_struct::on_collect(player_struct *player, Collect *collect)
{
	LOG_DEBUG("%s: raid[%u][%lu], collect[%u][%u]", __FUNCTION__, data->ID, data->uuid, collect->m_collectId, collect->m_uuid);

	struct DungeonTable *t_config = get_raid_config();
	if (!t_config)
		return;
		//有可能副本结束把采集物删除了
	uint32_t collect_uuid = collect->m_uuid;
	if (data->pass_index < t_config->n_PassType && t_config->PassType[data->pass_index] == 3)
	{
		if (t_config->PassValue[data->pass_index] == collect->m_collectId)
		{
				// TODO: 不能直接干掉副本，要记录状态并通知玩家副本结束，然后等他们退出去
//			raid_manager::delete_raid(this);
			if (add_raid_pass_value(3, t_config))
			{
				delete_raid_collect_safe(collect_uuid);
//				m_collect.erase(collect->m_uuid);
//				Collect::DestroyCollect(collect->m_uuid);
				return;
			}
		}
	}
	
	if (ai && ai->raid_on_raid_collect)
		ai->raid_on_raid_collect(this, player, collect);
	delete_raid_collect_safe(collect_uuid);
//	m_collect.erase(collect->m_uuid);	
//	Collect::DestroyCollect(collect->m_uuid);	
}

void raid_struct::send_raid_pass_param(player_struct *player)
{
		//只有万妖谷才发送
		//改成所有副本都发送
//	if (m_config->DengeonRank != DUNGEON_TYPE_RAND_MASTER)
//		return;

//	if (need_show_star())
//		return;
	
	RaidPassParamChangedNotify nty;
	raid_pass_param_changed_notify__init(&nty);
	nty.pass_index = data->pass_index;
	nty.pass_value = data->pass_value;
	
	if (!player)
		broadcast_to_raid(MSG_ID_RAID_PASS_PARAM_CHANGED_NOTIFY, &nty, (pack_func)raid_pass_param_changed_notify__pack);
	else
	{
		EXTERN_DATA  extern_data;
		extern_data.player_id = player->get_uuid();
		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_RAID_PASS_PARAM_CHANGED_NOTIFY, raid_pass_param_changed_notify__pack, nty);
	}
}

bool raid_struct::add_raid_pass_value(uint32_t pass_type, struct DungeonTable* config)
{
	++data->pass_value;
	if (pass_type == 5 || config->PassValue1[data->pass_index] <= data->pass_value)
	{
		++data->pass_index;
		data->pass_value = 0;
		if (data->pass_index >= config->n_PassType)
		{
			on_raid_finished();
			return true;
		}
	}

	send_raid_pass_param(NULL);
	// RaidPassParamChangedNotify nty;
	// raid_pass_param_changed_notify__init(&nty);
	// nty.pass_index = data->pass_index;
	// nty.pass_value = data->pass_value;

	// broadcast_to_raid(MSG_ID_RAID_PASS_PARAM_CHANGED_NOTIFY, &nty, (pack_func)raid_pass_param_changed_notify__pack);
	return false;
}

bool raid_struct::need_show_star()
{
	if (!m_config)
		return false;
	switch (m_config->DengeonRank)
	{
		case 0:
		case 8:
		case 18:			
			return true;
		default:
			return false;
	}
}

void raid_struct::send_star_changed_notify(uint32_t star_param[3], uint32_t score_param[3])
{
	RaidStarChangedNotify nty;
	raid_star_changed_notify__init(&nty);
	nty.n_star = 3;
	nty.n_param = 3;	
	nty.star = star_param;
	nty.param = score_param;	
	broadcast_to_raid(MSG_ID_RAID_STAR_CHANGED_NOTIFY, &nty, (pack_func)raid_star_changed_notify__pack)	;
}

void raid_struct::on_monster_dead(monster_struct *monster, unit_struct *killer)
{
	LOG_DEBUG("%s: raid[%u][%lu], monster[%u][%lu]", __FUNCTION__, data->ID, data->uuid, monster->data->monster_id, monster->get_uuid());
//	m_monster.remove(monster);
//	assert(data->pass_index < m_config->n_PassType);
	struct DungeonTable *t_config = get_raid_config();
	if (!t_config)
		return;
		//杀死怪物
	if (data->pass_index < t_config->n_PassType && t_config->PassType[data->pass_index] == 1)
	{
		if (t_config->PassValue[data->pass_index] == monster->config->ID)
		{
				// TODO: 不能直接干掉副本，要记录状态并通知玩家副本结束，然后等他们退出去
//			raid_manager::delete_raid(this);
			if (add_raid_pass_value(1, t_config))
			{
				monster_manager::delete_monster(monster);
				return;
			}
		}
	}
	if (data->pass_index < t_config->n_PassType && t_config->PassType[data->pass_index] == 7)
	{
		uint32_t config_id = t_config->PassValue[data->pass_index];
		MonsterIDTable* monsterid_config =  get_config_by_id(config_id, &raid_jincheng_suiji_kill_monster);
		if(monsterid_config != NULL)
		{
			bool flag = false;
			for(size_t i = 0; i < monsterid_config->n_MonseterID; i++)
			{
				if(monster->config->ID == monsterid_config->MonseterID[i])
					flag = true;
			}
			if(flag == true)
			{
				if (add_raid_pass_value(1, t_config))
				{
					monster_manager::delete_monster(monster);
					return;
				}
				
			}
		}
	}

	bool send_star_changed = false;
	for (uint32_t i = 0; i < t_config->n_Score; ++i)
	{
			//击杀怪物或者怪物存活
		if (t_config->Score[i] != 2 && t_config->Score[i] != 3)
			continue;
		if (t_config->ScoreValue[i] != monster->config->ID)
			continue;
		++data->star_param[i];

		if (data->star_param[i] <= t_config->ScoreValue1[i])
			send_star_changed = true;
	}
	if (send_star_changed && need_show_star())
	{
		uint32_t star_param[3];
		uint32_t score_param[3];
		calc_raid_star(star_param, score_param);
		send_star_changed_notify(star_param, score_param);		
	}

	if (ai && ai->raid_on_monster_dead)
		ai->raid_on_monster_dead(this, monster, killer);
	monster_manager::delete_monster(monster);
/*
	switch ((int)monster->get_unit_type())
	{
		case UNIT_TYPE_MONSTER:
			monster_manager::delete_monster(monster);
			break;
		case UNIT_TYPE_BOSS:
			monster_manager::delete_boss((boss_struct *)monster);
			break;
	}
*/
}

int raid_struct::broadcast_to_raid(uint32_t msg_id, void *msg_data, pack_func func)
{
	uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(msg_id, msg_data, func);
	PROTO_HEAD_CONN_BROADCAST *head;
	head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;

	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		if (m_player[i] && m_player[i]->is_avaliable())
			conn_node_gamesrv::broadcast_msg_add_players(m_player[i]->get_uuid(), ppp);
		if (m_player2[i] && m_player2[i]->is_avaliable())
			conn_node_gamesrv::broadcast_msg_add_players(m_player2[i]->get_uuid(), ppp);
		if (m_player3[i] && m_player3[i]->is_avaliable())
			conn_node_gamesrv::broadcast_msg_add_players(m_player3[i]->get_uuid(), ppp);
		if (m_player4[i] && m_player4[i]->is_avaliable())
			conn_node_gamesrv::broadcast_msg_add_players(m_player4[i]->get_uuid(), ppp);					
	}

	if (head->num_player_id > 0)
	{
//		head->len += sizeof(uint64_t) * head->num_player_id;
		return conn_node_gamesrv::broadcast_msg_send();
	}
	return (0);
}

bool raid_struct::is_guild_battle_raid()
{
	switch(m_config->DengeonRank)
	{
		case DUNGEON_TYPE_GUILD_WAIT:
		case DUNGEON_TYPE_GUILD_RAID:
		case DUNGEON_TYPE_GUILD_FINAL_RAID:
			return true;
	}
	return false;
}

bool raid_struct::is_guild_battle_fight_raid()
{
	switch(m_config->DengeonRank)
	{
		case DUNGEON_TYPE_GUILD_RAID:
		case DUNGEON_TYPE_GUILD_FINAL_RAID:
			return true;
	}
	return false;
}

monster_struct *raid_struct::get_first_boss()
{
	for (std::set<monster_struct*>::iterator iter = m_monster.begin(); iter != m_monster.end(); ++iter)
	{
		monster_struct *monster = *iter;
		if (monster->get_unit_type() == UNIT_TYPE_BOSS)
		{
			return monster;
		}
	}
	return NULL;
}

// int raid_struct::player_enter(player_struct *player, double pos_x, double pos_y, double pos_z, double direct)
// {
// 	return (0);
// }
// int raid_struct::player_leave(player_struct *player, bool send_clear_sight)
// {
// 	if (m_config->DengeonType != 2 && data->state == RAID_STATE_START && player->m_team)
// 	{
// 			//组队副本如果没结束，那么踢出队伍
// 		player->m_team->RemoveMember(*player, false);
// 	}
// 	else
// 	{
// 		player_leave_raid(player);
// 	}
// 	return (0);
// }

bool raid_struct::use_m_player()
{
	if (m_config->DengeonRank == DUNGEON_TYPE_BATTLE //|| m_config->DengeonRank == DUNGEON_TYPE_BATTLE_NEW
		|| m_config->DengeonRank == DUNGEON_TYPE_ZHENYING)
	{
		return false; //阵营战
	}
	return true;
}

int raid_struct::player_leave_scene(player_struct *player)
{
	if (get_entity_type(player->get_uuid()) == ENTITY_TYPE_AI_PLAYER)
	{
		LOG_ERR("%s: ai player[%lu] can not leave raid[%u] uuid[%lu]", __FUNCTION__, player->get_uuid(), m_id, data->uuid);
		return (-1);
	}

	LOG_DEBUG("%s: player[%lu] leave raid %u[%lu]", __FUNCTION__, player->get_uuid(), data->ID, data->uuid);
	if (clear_m_player_and_player_info(player, true) != 0)
		return -1;
	if (player->scene != this)
	{
		LOG_ERR("%s: player[%lu] not in raid[%u %lu]", __FUNCTION__, player->get_uuid(), data->ID, data->uuid);
		return (-1);
	}

//	player->set_out_raid_pos_and_clear_scene();
	player->on_leave_raid();
	delete_player_from_scene(player);
	on_player_leave_raid(player);

	EXTERN_DATA extern_data;
	extern_data.player_id = player->get_uuid();	
	fast_send_msg_base(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_LEAVE_RAID_NOTIFY, 0, 0);

	player->data->m_angle = unity_angle_to_c_angle(player->data->leaveraid.direct);
	return (0);
}
int raid_struct::player_enter_raid(player_struct *player, double pos_x, double pos_z, double direct)
{
	LOG_DEBUG("%s: raid[%u][%lu], player[%lu]", __FUNCTION__, data->ID, data->uuid, player->get_uuid());
	int index = get_free_player_pos();
	if (index < 0)
	{
		LOG_ERR("%s: player[%lu] can not add raid[%u %lu]", __FUNCTION__, player->get_uuid(), data->ID, data->uuid);
		return (-1);
	}
	return player_enter_raid_impl(player, index, pos_x, pos_z, direct);
}
