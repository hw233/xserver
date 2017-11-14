#include "guild_wait_raid.h"
#include "guild_wait_raid_manager.h"
#include "time_helper.h"
#include "raid.pb-c.h"
#include "msgid.h"
#include "raid_manager.h"
#include "camp_judge.h"
#include "collect.h"
#include "guild_battle_manager.h"

int guild_wait_raid_struct::init_special_raid_data(player_struct *player)
{
	raid_set_ai_interface(10);
	init_scene_struct(m_id, true);	
	return (0);
}

void guild_wait_raid_struct::add_player_to_guild_wait_raid(player_struct *player, bool ignore_check)
{
		// 检查能否进入
	if (!ignore_check && raid_manager::check_player_enter_raid(player, GUILD_WAIT_RAID_ID) != 0)
	{
		LOG_ERR("%s: player[%lu] enter raid[%u] failed", __FUNCTION__, player->get_uuid(), GUILD_WAIT_RAID_ID);
		return;
	}

	assert(res_config);
	player_enter_raid(player, res_config->BirthPointX, res_config->BirthPointZ, 0);
	// player->set_enter_raid_pos_and_scene(this);

	// EXTERN_DATA extern_data;
	// extern_data.player_id = player->get_uuid();

	// EnterRaidNotify notify;
	// enter_raid_notify__init(&notify);
	// notify.raid_id = m_id;
	// fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ENTER_RAID_NOTIFY, enter_raid_notify__pack, notify);

	// player->send_scene_transfer(res_config->FaceY, res_config->BirthPointX, res_config->BirthPointY,
	// 	res_config->BirthPointZ, m_id, 0);

	// on_player_enter_raid(player);
}

void guild_wait_raid_struct::on_monster_dead(monster_struct *monster, unit_struct *killer)
{
		//应该没有怪物才对
	assert(0);
}

void guild_wait_raid_struct::on_collect(player_struct *player, Collect *collect)
{
		//应该没有采集物
	assert(0);
}

void guild_wait_raid_struct::get_wait_player(std::set<uint64_t> &playerIds)
{
	for (std::map<uint64_t, player_struct*>::iterator iter = m_players.begin(); iter != m_players.end(); ++iter)
	{
		playerIds.insert(iter->first);
	}
}

