#include "zhenying_battle.h"
#include <unistd.h>
#include "comm.h"
#include "player.h"
#include "msgid.h"
#include "../proto/zhenying.pb-c.h"
#include "../proto/player_redis_info.pb-c.h"
#include "game_config.h"
#include "raid_manager.h"
#include "camp_judge.h"
#include "app_data_statis.h"
#include "zhenying_raid_manager.h"


void player_struct::send_zhenying_info()
{
	ZhenyingInfo send;
	zhenying_info__init(&send);
	send.zhenying = get_attr(PLAYER_ATTR_ZHENYING);
	send.level = data->zhenying.level;
	send.exp = data->zhenying.exp;
	send.task = data->zhenying.task;
	send.task_type = data->zhenying.task_type;
	send.task_num = data->zhenying.task_num;
	send.step =data->zhenying.step;
	send.free_change = data->zhenying.free;
	EXTERN_DATA extern_data;
	extern_data.player_id = get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_INFO_NOTIFY, zhenying_info__pack, send);
}

void player_struct::add_zhenying_exp(uint32_t num)
{
	if (get_attr(PLAYER_ATTR_ZHENYING) == 0)
	{
		return;
	}
	CampTable *config = get_config_by_id(360101001, &zhenying_base_config);
	if (config != NULL)
	{
		if (data->zhenying.exp_day >= config->MaxExp)
		{
			return;
		}
	}

	GradeTable *table = get_config_by_id((36020 + get_attr(PLAYER_ATTR_ZHENYING)) * 10000 + data->zhenying.level, &zhenying_level_config); 
	if (table != NULL && table->LevelExp == 0)
	{
		return;
	}
	data->zhenying.exp += num;
	data->zhenying.exp_day += num;
	while (table != NULL && data->zhenying.exp >= table->LevelExp && table->LevelExp > 0)
	{
		data->zhenying.exp -= table->LevelExp;
		++data->zhenying.level;
		table = get_config_by_id((36020 + get_attr(PLAYER_ATTR_ZHENYING)) * 10000 + data->zhenying.level, &zhenying_level_config);
	}

	ZhenyingExp send;
	zhenying_exp__init(&send);
	send.exp = data->zhenying.exp;
	send.exp_day = data->zhenying.exp_day;
	send.level = data->zhenying.level;
	EXTERN_DATA extern_data;
	extern_data.player_id = get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_EXP_NOTIFY, zhenying_exp__pack, send);
}

void player_struct::refresh_zhenying_task_oneday()
{
	data->zhenying.task_num = 0;
	data->zhenying.kill = 0;

	NewZhenyingTask send;
	new_zhenying_task__init(&send);
	send.task = data->zhenying.task;
	send.task_type = data->zhenying.task_type;
	send.task_num = data->zhenying.task_num;
	EXTERN_DATA extern_data;
	extern_data.player_id = get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_NEW_ZHENYING_TASK_NOTIFY, new_zhenying_task__pack, send);

}












//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// BattleField *ZhenyingBattle::GetFiled()
// {
// 	if (ZhenyingBattle_m_filed == NULL)
// 	{
// 		ZhenyingBattle_m_filed = new BattleField;
// 		ZhenyingBattle_m_filed->init_scene_struct(30001, true);
// 	}
// 	return ZhenyingBattle_m_filed;
// }

int ZhenyingBattle::GotoBattle(player_struct &player)
{
	//raid_struct * raid = raid_manager::get_raid_by_uuid(m_battlefield);
	//if (raid == NULL)
	//{
	//	raid = raid_manager::create_zhenying_raid(24, &player);
	//	m_battlefield = raid->data->uuid;
	//}
	//if (player.m_team)
	//{
	//	raid->team_enter_raid(player.m_team);
	//}
	//else
	//{
	//	if (raid->player_enter_raid(&player) != 0)
	//	{
	//		LOG_ERR("%s: player[%lu] enter raid failed", __FUNCTION__, player.get_uuid());
	//		return ;
	//	}
	//}



	//GetFiled();
	//player.set_attr(PLAYER_ATTR_PK_TYPE, PK_TYPE_CAMP);
	//player.broadcast_one_attr_changed(PLAYER_ATTR_PK_TYPE, PK_TYPE_CAMP, false, true);
	//EXTERN_DATA extern_data;
	//extern_data.player_id = player.get_uuid();
	//player.transfer_to_new_scene_impl(ZhenyingBattle_m_filed, 66, 100, 53, 0, &extern_data);

	zhenying_raid_struct *raid = zhenying_raid_manager::add_player_to_zhenying_raid(&player);
	if (raid == NULL)
		return (-1);
	return (0);
}

void ZhenyingBattle::UpdateOneTeamInfo(player_struct &player)
{
	ZhenyingTeamInfo send;
	zhenying_team_info__init(&send);
	send.playerid = player.get_uuid();
	send.name = player.get_name();
	send.job = player.get_attr(PLAYER_ATTR_JOB);
	send.lv = player.get_attr(PLAYER_ATTR_LEVEL);
	send.kill = player.data->zhenying.kill;
	send.death = player.data->zhenying.death;
	send.assist = player.data->zhenying.help;
	send.score = player.data->zhenying.score;
	EXTERN_DATA extern_data;
	extern_data.player_id = player.get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_TEAM_INFO_NOTIFY, zhenying_team_info__pack, send);
}










///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void BattleField::update_task_process(uint32_t type, player_struct *player)
{
	if (player == NULL)
	{
		return;
	}
	if (player->data->zhenying.task == 0)
	{
		return;
	}
	if (player->data->zhenying.task_type != type)
	{
		return;
	}
	WeekTable *table = get_config_by_id(player->data->zhenying.task, &zhenying_week_config);
	if (table == NULL)
	{
		return;
	}

	if (player->data->zhenying.task_num < table->Num)
	{
		++player->data->zhenying.task_num;
		ZhenyingTaskProcess send;
		zhenying_task_process__init(&send);
		send.task_num = player->data->zhenying.task_num;
		EXTERN_DATA extern_data;
		extern_data.player_id = player->get_uuid();
		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_TASK_PROCESS_NOTIFY, zhenying_task_process__pack, send);
	}
}
void BattleField::on_collect(player_struct *player, Collect *collect)
{}

void BattleField::on_monster_dead(monster_struct *monster, unit_struct *killer)
{
	if (killer->get_unit_type() == UNIT_TYPE_PLAYER)
	{
		player_struct *pKill = (player_struct *)killer;

		pKill->add_zhenying_exp(5);

		update_task_process(3, pKill);
	}
}

void BattleField::on_player_dead(player_struct *player, unit_struct *killer)
{
	if (killer == NULL || player == NULL)
	{
		return;
	}
	if (killer->get_unit_type() == UNIT_TYPE_PLAYER)
	{
		player_struct *pKill = (player_struct *)killer;

		pKill->add_zhenying_exp(10);
		++pKill->data->zhenying.kill;
		++pKill->data->zhenying.kill_week;
		pKill->data->zhenying.score += 5;
		pKill->data->zhenying.score_week += 5;

		++player->data->zhenying.death;

		ZhenyingBattle::UpdateOneTeamInfo(*player);
		ZhenyingBattle::UpdateOneTeamInfo(*pKill);

		update_task_process(1, pKill);

		EXTERN_DATA extern_data;
		extern_data.player_id = pKill->get_uuid();
		AddZhenyingPlayer tofr;
		add_zhenying_player__init(&tofr);
		tofr.name = pKill->get_name();
		tofr.zhenying = pKill->get_attr(PLAYER_ATTR_ZHENYING);
		tofr.zhenying_old = pKill->get_attr(PLAYER_ATTR_ZHENYING);
		tofr.fighting_capacity = pKill->get_attr(PLAYER_ATTR_FIGHTING_CAPACITY);
		tofr.kill = pKill->data->zhenying.kill_week;
		conn_node_gamesrv::connecter.send_to_friend(&extern_data, SERVER_PROTO_ADD_ZHENYING_KILL_REQUEST, &tofr, (pack_func)add_zhenying_player__pack);
	}
}


