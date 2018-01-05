
#include "zhenying_raid.h"
#include "time_helper.h"
#include "raid.pb-c.h"
#include "zhenying.pb-c.h"
#include "msgid.h"
#include "zhenying_raid_manager.h"
#include "raid_manager.h"
#include "camp_judge.h"
#include "collect.h"

int zhenying_raid_struct::raid_num;

zhenying_raid_struct::zhenying_raid_struct()
{
	++raid_num;
}

zhenying_raid_struct::~zhenying_raid_struct()
{
	--raid_num;
}

int zhenying_raid_struct::init_special_raid_data(player_struct *player)
{
	raid_set_ai_interface(9);
	init_scene_struct(m_id, true, 0);
	return (0);
}

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
	LOG_INFO("%s: player[%lu]", __FUNCTION__, player->get_uuid());
	FactionBattleTable *table = get_zhenying_battle_table(player->get_attr(PLAYER_ATTR_LEVEL));
	if (table == NULL)
	{
		return -1;
	}
	assert(get_cur_player_num() < MAX_ZHENYING_RAID_PLAYER_NUM);

	int x, z;
	double direct = 0;
	zhenying_raid_manager::GetRelivePos(table, player->get_attr(PLAYER_ATTR_ZHENYING), &x, &z, &direct);
	player_enter_raid_impl(player, this->get_free_player_pos(), x, z, direct);
	player->set_attr(PLAYER_ATTR_PK_TYPE, PK_TYPE_CAMP);
	player->broadcast_one_attr_changed(PLAYER_ATTR_PK_TYPE, PK_TYPE_CAMP, false, true);
	return (0);
}

bool zhenying_raid_struct::use_m_player()
{
	return false;
}

bool zhenying_raid_struct::check_raid_need_delete()
{
	//if (data->state == RAID_STATE_START)
	//	return false;
	//
	//if (get_cur_player_num() == 0)
	//	return true;
	return false;  //一直开
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

