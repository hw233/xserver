#include "zhenying_raid.h"
#include "time_helper.h"
#include "raid.pb-c.h"
#include "zhenying.pb-c.h"
#include "msgid.h"
#include "zhenying_raid_manager.h"
#include "raid_manager.h"
#include "camp_judge.h"
#include "collect.h"

int zhenying_raid_struct::init_special_raid_data(player_struct *player)
{
	raid_set_ai_interface(9);
	init_scene_struct(m_id, true);
	return (0);
}

// int zhenying_raid_struct::init_raid(player_struct */*player*/)
// {
// 	for (int i = 0; i < MAX_TEAM_MEM; ++i)
// 	{
// 		m_player[i] = NULL;
// 		m_player2[i] = NULL;
// 		m_player3[i] = NULL;
// 		m_player4[i] = NULL;		
// 	}
// 	m_raid_team = NULL;
// 	m_raid_team2 = NULL;
// 	m_raid_team3 = NULL;
// 	m_raid_team4 = NULL;	
// 	data->ID = m_id;
// 	data->state = RAID_STATE_START;
// 	m_monster.clear();

// 	m_config = get_config_by_id(m_id, &all_raid_config);
// 	assert(m_config);
// 	m_control_config = get_config_by_id(m_config->ActivityControl, &all_control_config);
// 	assert(m_control_config);

// 	data->start_time = time_helper::get_cached_time();

// 	raid_set_ai_interface(9);
// 	LOG_DEBUG("%s: raid[%u][%lu] curtime = %lu", __FUNCTION__, data->ID, data->uuid, time_helper::get_cached_time());

// 	init_scene_struct(m_id, true);

// 	if (ai && ai->raid_on_init)
// 		ai->raid_on_init(this, NULL);

// //	add_player_to_zhenying_raid(player);
// 	return (0);
// }

int zhenying_raid_struct::clear_m_player_and_player_info(player_struct *player, bool clear_player_info)
{
	return (0);
}

int zhenying_raid_struct::set_m_player_and_player_info(player_struct *player, int index)
{
	return (0);
}

int zhenying_raid_struct::add_player_to_zhenying_raid(player_struct *player)
{
	FactionBattleTable *table = get_zhenying_battle_table(player->get_attr(PLAYER_ATTR_LEVEL));
	if (table == NULL)
	{
		return -1;
		
	}
		// 检查能否进入
	//if (raid_manager::check_player_enter_raid(player, ZHENYING_RAID_ID) != 0)
	//{
	//	LOG_INFO("%s: player[%lu] enter raid[%u] failed", __FUNCTION__, player->get_uuid(), ZHENYING_RAID_ID);
	//	return;
	//}
	
	assert(get_cur_player_num() < MAX_ZHENYING_RAID_PLAYER_NUM);
	// player->set_enter_raid_pos_and_scene(this);

	// EXTERN_DATA extern_data;
	// extern_data.player_id = player->get_uuid();

	// EnterRaidNotify notify;
	// enter_raid_notify__init(&notify);
	// notify.raid_id = m_id;
	// fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ENTER_RAID_NOTIFY, enter_raid_notify__pack, notify);

	float x, y, z;
	UNUSED(y);
	if (player->get_attr(PLAYER_ATTR_ZHENYING) == ZHENYING__TYPE__FULONGGUO)
	{
		x = table->BirthPoint1[0];
		y = table->BirthPoint1[1];
		z = table->BirthPoint1[2];
		player->set_camp_id(ZHENYING__TYPE__FULONGGUO);
	}
	else
	{
		x = table->BirthPoint2[0];
		y = table->BirthPoint2[1];
		z = table->BirthPoint2[2];
		player->set_camp_id(ZHENYING__TYPE__WANYAOGU);
	}

	player_enter_raid_impl(player, 0, x, z);
	
	// player->send_scene_transfer(0, x, y, z, m_id, 0);

	// on_player_enter_raid(player);

	player->set_attr(PLAYER_ATTR_PK_TYPE, PK_TYPE_CAMP);
	player->broadcast_one_attr_changed(PLAYER_ATTR_PK_TYPE, PK_TYPE_CAMP, false, true);
	return (0);
}

bool zhenying_raid_struct::check_raid_need_delete()
{
	if (data->state == RAID_STATE_START)
		return false;
	
	if (get_cur_player_num() == 0)
		return true;
	return false;
}

void zhenying_raid_struct::on_monster_dead(monster_struct *monster, unit_struct *killer)
{
		//不要删除怪物了，阵营副本怪物需要重生
		//从场景删除的时候，移出了m_monster, 会导致raid::clear不会删除该怪物
	if (ai && ai->raid_on_monster_dead)
		ai->raid_on_monster_dead(this, monster, killer);
}

void zhenying_raid_struct::on_collect(player_struct *player, Collect *collect)
{
	if (ai && ai->raid_on_raid_collect)
		ai->raid_on_raid_collect(this, player, collect);
}

// uint16_t zhenying_raid_struct::get_cur_player_num()
// {
// 	return ZHENYING_DATA.cur_player_num;
// }

bool zhenying_raid_struct::is_in_zhenying_raid()
{
	return true;
}

// int zhenying_raid_struct::delete_player_from_scene(player_struct *player)
// {
// 	on_player_leave_raid(player);
// 	return scene_struct::delete_player_from_scene(player);
// }

void zhenying_raid_struct::create_collect()
{
	std::vector<struct FactionBattleTable*>::iterator it = zhenying_battle_config.begin();
	FactionBattleTable *table = *it;
	while (m_collect_pos.size() < table->BoxReloadNum)
	{
		uint32_t r = rand() % table->n_BoxReloadX;
		uint64_t iPos = table->BoxReloadX[r] * table->BoxReloadZ[r];
		std::set<uint64_t>::iterator it = m_collect_pos.find(iPos);
		if (it == m_collect_pos.end())
		{
			m_collect_pos.insert(iPos);
			Collect::CreateCollectByPos(this, table->BoxID, table->BoxReloadX[r], table->BoxReloadY[r], table->BoxReloadZ[r], 0);
		}
	}
}

void zhenying_raid_struct::delete_collect_pos(uint64_t pos)
{
	m_collect_pos.erase(pos);
}

