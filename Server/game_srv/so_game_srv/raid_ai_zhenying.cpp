#include <math.h>
#include <stdlib.h>
#include <algorithm>
#include "game_event.h"
#include "raid_ai.h"
#include "raid_manager.h"
#include "time_helper.h"
#include "app_data_statis.h"
#include "unit.h"
#include "msgid.h"
#include "raid.pb-c.h" 
#include "../proto/relive.pb-c.h"
#include "../proto/zhenying.pb-c.h"
#include "../proto/player_redis_info.pb-c.h"
#include "zhenying_battle.h"
#include "collect.h"
//#include "zhenying_raid.h"
#include "zhenying_raid_manager.h"
#include "uuid.h"
#include "buff_manager.h"
#include "monster_manager.h"
#include "player_manager.h"


static void update_task_process(uint32_t type, player_struct *player)
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

void update_zhenying_score(player_struct *player, int add, uint64_t taskId)
{
	if (player == NULL)
	{
		return;
	}
	if (player->task_is_finish(taskId))
		return;
	player->data->zhenying.score += add;
	if (player->data->zhenying.score < 0)
	{
		player->data->zhenying.score = 0;
	}
	player->add_task_progress(TCT_ZHENYING_SCORE, 0, player->data->zhenying.score);
}

void zhenying_raid_ai_tick_on_truck(raid_struct *raid)
{
	monster_struct *pTruck = monster_manager::get_monster_by_id(raid->data->ai_data.zhenying_data.truck);
	if (pTruck == NULL)
	{
		return;
	}
	CampDefenseTable *tableDaily = get_config_by_id(raid->data->ai_data.zhenying_data.camp, &zhenying_daily_config);
	if (tableDaily == NULL)
	{
		return;
	}
	uint64_t now = time_helper::get_cached_time() / 1000;

	int player_num = *pTruck->get_cur_sight_player();
	uint64_t *t_player_id = pTruck->get_all_sight_player();
	for (int i = 0; i < player_num; ++i)
	{
		if (get_entity_type(t_player_id[i]) != ENTITY_TYPE_PLAYER)
			continue;
		player_struct *player = player_manager::get_player_by_id(t_player_id[i]);
		if (player == NULL)
		{
			continue;
		}
		if (player->get_attr(PLAYER_ATTR_ZHENYING) != pTruck->get_attr(PLAYER_ATTR_ZHENYING))
		{
			continue;
		}
		int dx = player->get_pos()->pos_x - pTruck->get_pos()->pos_x;
		int dz = player->get_pos()->pos_z - pTruck->get_pos()->pos_z;
		uint64_t dis = dx * dx + dz * dz;
		if (dis > tableDaily->SupportMine[0] * tableDaily->SupportMine[0] * 2)
		{
			player->data->zhenying.score_time = 0;
			continue;
		}
		if (player->data->zhenying.score_time == 0)
		{
			player->data->zhenying.score_time = now + tableDaily->SupportMine[1];
		}
		else if (player->data->zhenying.score_time < now)
		{
			player->data->zhenying.score_time = now + tableDaily->SupportMine[1];
			player->data->zhenying.protect_num += 1;
			update_zhenying_score(player, tableDaily->SupportMine[2], tableDaily->TaskID);
		}
	}
}
void zhenying_raid_ai_tick_check_progress(raid_struct *raid)
{
	CampDefenseTable *tableDaily = get_config_by_id(raid->data->ai_data.zhenying_data.camp, &zhenying_daily_config);
	if (tableDaily == NULL)
	{
		return;
	}
	uint64_t now = time_helper::get_cached_time() / 1000;

	EXTERN_DATA extern_data;
	if ((raid->data->ai_data.zhenying_data.progress == DAILY__MINE_STATE_DEAD || raid->data->ai_data.zhenying_data.progress == DAILY__MINE_STATE_COMPLETE)
		&& raid->data->ai_data.zhenying_data.time_rest < now)
	{
		MineState sendState;
		mine_state__init(&sendState);
		sendState.state = DAILY__MINE_STATE_REST;
		raid->data->ai_data.zhenying_data.progress = DAILY__MINE_STATE_REST;

		std::set<uint64_t> playerIds;
		raid->get_all_player(playerIds);
		for (std::set<uint64_t>::iterator it = playerIds.begin(); it != playerIds.end(); ++it)
		{
			if (get_entity_type(*it) == ENTITY_TYPE_AI_PLAYER)
			{
				continue;
			}
			player_struct *player = player_manager::get_player_by_id(*it);
			if (player != NULL)
			{
				extern_data.player_id = player->get_uuid();
				fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_DAILY_MINE_STATE_NOTIFY, mine_state__pack, sendState);
				DailyMine sendMine;
				daily_mine__init(&sendMine);
				sendMine.cur = raid->data->ai_data.zhenying_data.cur;
				sendMine.max = tableDaily->TruckPlan;
				fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_DAILY_MINE_NOTIFY, daily_mine__pack, sendMine);
			}
		}
	}
}


void zhenying_raid_ai_tick(raid_struct *raid)
{
	zhenying_raid_ai_tick_on_truck(raid);
	zhenying_raid_ai_tick_check_progress(raid);
}

static void zhenying_raid_ai_init(raid_struct *raid, player_struct *)
{
	
}

void zhenying_raid_ai_finished(raid_struct *raid)
{
	//raid->clear_monster();
	//raid->data->state = RAID_STATE_PASS; 
	//一直开
}

static void zhenying_raid_ai_player_enter(raid_struct *raid, player_struct *player)
{
	LOG_INFO("%s: player[%lu] add to %lu", __FUNCTION__, player->get_uuid(), raid->data->uuid);
//	raid->ZHENYING_DATA.cur_player_num++;

	//FbCD notify;
	//fb_cd__init(&notify);
	//notify.cd = 0;
	//if (raid->data->start_time / 1000 + raid->m_config->FailValue[0] > time_helper::get_cached_time() / 1000)
	//{
	//	notify.cd = raid->data->start_time / 1000 + raid->m_config->FailValue[0] - time_helper::get_cached_time() / 1000;
	//}
	//EXTERN_DATA extern_data;
	//extern_data.player_id = player->get_uuid();
	//fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_DAILY_CD_NOTIFY, fb_cd__pack, notify);

	//CampDefenseTable *tableDaily = get_config_by_id(raid->data->ai_data.zhenying_data.camp, &zhenying_daily_config);
	//if (tableDaily == NULL)
	//{
	//	return;
	//}
	//if (player->accept_task(tableDaily->TaskID, false) != 0)
	//{
	//	LOG_ERR("%s: accept TASK fail", __FUNCTION__);
	//	return;
	//}
	//player->add_task_progress(TCT_ACTIVITY, 0, raid->data->ai_data.zhenying_data.cur);
}
static void zhenying_raid_ai_player_leave(raid_struct *raid, player_struct *player)
{
	LOG_INFO("%s: player[%lu] del from %lu", __FUNCTION__, player->get_uuid(), raid->data->uuid);	
//	raid->ZHENYING_DATA.cur_player_num--;
	CampDefenseTable *tableDaily = get_config_by_id(raid->data->ai_data.zhenying_data.camp, &zhenying_daily_config);
	if (tableDaily == NULL)
	{
		return;
	}
	//player->add_finish_task(tableDaily->TaskID);
	player->del_finish_task(tableDaily->TaskID);
}

//static void UpdateOneTeamInfo(player_struct &player)
//{
//	ZhenyingTeamInfo send;
//	zhenying_team_info__init(&send);
//	send.playerid = player.get_uuid();
//	send.name = player.get_name();
//	send.job = player.get_attr(PLAYER_ATTR_JOB);
//	send.lv = player.get_attr(PLAYER_ATTR_LEVEL);
//	send.kill = player.data->zhenying.kill;
//	send.death = player.data->zhenying.death;
//	send.assist = player.data->zhenying.help;
//	send.score = player.data->zhenying.score;
//	EXTERN_DATA extern_data;
//	extern_data.player_id = player.get_uuid();
//	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_TEAM_INFO_NOTIFY, zhenying_team_info__pack, send);
//}
static void zhenying_raid_ai_player_dead(raid_struct *raid, player_struct *player, unit_struct *killer)
{
	if (killer == NULL || player == NULL)
	{
		return;
	}
	CampDefenseTable *tableDaily = get_config_by_id(raid->data->ai_data.zhenying_data.camp, &zhenying_daily_config);
	if (tableDaily == NULL)
	{
		return;
	}
	ParameterTable *table = get_config_by_id(161001019, &parameter_config);
	if (table != NULL)
	{
		return;
	}
	if (killer->get_unit_type() == UNIT_TYPE_PLAYER)
	{
		player_struct *pKill = (player_struct *)killer;
		update_task_process(1, pKill);
		int add = 0;
		if (!player->task_is_finish(tableDaily->TaskID))
		{
			add = player->data->zhenying.score * table->parameter1[0] / 100;
			if (add > 0)
			{
				update_zhenying_score(player, 0 - add, tableDaily->TaskID);
			}
			else
			{
				add = table->parameter1[1];
			}
		}
		update_zhenying_score(pKill, add, tableDaily->TaskID);
	}
}
static void zhenying_raid_ai_monster_dead(raid_struct *raid, monster_struct *monster, unit_struct *killer)
{
	player_struct *pKill = NULL;
	if (killer->get_unit_type() == UNIT_TYPE_PLAYER)
	{
		CampDefenseTable *tableDaily = get_config_by_id(raid->data->ai_data.zhenying_data.camp, &zhenying_daily_config);
		if (tableDaily == NULL)
		{
			return;
		}

		pKill = (player_struct *)killer;
		update_task_process(3, pKill);

		DailyScore send;
		daily_score__init(&send);
		send.point = pKill->data->zhenying.score;
		EXTERN_DATA extern_data;
		extern_data.player_id = pKill->get_uuid();
		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_DAILY_SCORE_NOTIFY, daily_score__pack, send);

		MineState sendState;
		mine_state__init(&sendState);
		if (monster->config->Type == 6) //阵营战的箱子
		{
			uint32_t step = 0;
			for (; step < tableDaily->n_MonsterID - 1; ++step)
			{
				if (monster->data->monster_id == tableDaily->MonsterID[step])
				{
					break;
				}
			}
			uint32_t addScore = 0;
			uint32_t mineLimit = 0;
			if (pKill->get_attr(PLAYER_ATTR_ZHENYING) == raid->data->ai_data.zhenying_data.camp % 10) // 守
			{
				if (raid->data->ai_data.zhenying_data.progress == DAILY__MINE_STATE_REST && pKill->get_attr(PLAYER_ATTR_ZHENYING) == raid->data->ai_data.zhenying_data.camp % 10)
				{
					addScore = tableDaily->MineralIntegral1[step];
					mineLimit = tableDaily->MiningLimit1;
					uint32_t old = raid->data->ai_data.zhenying_data.cur;
					raid->data->ai_data.zhenying_data.cur += addScore;
					if (raid->data->ai_data.zhenying_data.cur >= tableDaily->TruckPlan && old < tableDaily->TruckPlan)
					{
						FactionBattleTable *table = get_zhenying_battle_table(raid->data->ai_data.zhenying_data.lv);
						monster_struct *pMon = monster_manager::create_monster_at_pos(NULL, tableDaily->TruckID, table->Level, tableDaily->TruckRouteX[0], tableDaily->TruckRouteY[0], 0, NULL, 0);
						pMon->create_config = get_daily_zhenying_truck_config(raid->data->ai_data.zhenying_data.camp);
						//pMon->set_attr(PLAYER_ATTR_ZHENYING, raid->data->ai_data.zhenying_data.camp % 10);
						pMon->set_camp_id(raid->data->ai_data.zhenying_data.camp % 10);
						pMon->born_pos.pos_x = tableDaily->TruckRouteX[0];
						pMon->born_pos.pos_z = tableDaily->TruckRouteY[0];

						if (raid->add_monster_to_scene(pMon, 0) != 0)
						{
							LOG_ERR("%s: uuid[%lu] monster[%u] scene[%u]", __FUNCTION__, pMon->data->player_id, pMon->data->monster_id, raid->m_id);
						}
						sendState.state = DAILY__MINE_STATE_RUN;
						raid->data->ai_data.zhenying_data.progress = DAILY__MINE_STATE_RUN;
						fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_DAILY_MINE_STATE_NOTIFY, mine_state__pack, sendState);
						raid->data->ai_data.zhenying_data.truck = pMon->get_uuid();
					}
					DailyMine sendMine;
					daily_mine__init(&sendMine);
					sendMine.cur = raid->data->ai_data.zhenying_data.cur;
					sendMine.max = tableDaily->TruckPlan;
					fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_DAILY_MINE_NOTIFY, daily_mine__pack, sendMine);
				}
			} 
			else //攻
			{
				addScore = tableDaily->MineralIntegral2[step]; 
				mineLimit = tableDaily->MiningLimit2; 
			}
			if (pKill->data->zhenying.mine < mineLimit)
			{
				update_zhenying_score(pKill, addScore, tableDaily->TaskID); 
			}
		}
		else if (monster->data->monster_id == tableDaily->TruckID)
		{
			raid->data->ai_data.zhenying_data.cur = 0;
			std::set<uint64_t> playerIds;
			raid->get_all_player(playerIds);
			for (std::set<uint64_t>::iterator it = playerIds.begin(); it != playerIds.end(); ++it)
			{
				if (get_entity_type(*it) == ENTITY_TYPE_AI_PLAYER)
				{
					continue;
				}
				player_struct *player = player_manager::get_player_by_id(*it);
				if (player != NULL)
				{
					player->data->zhenying.protect_num = 0;
					player->data->zhenying.score_time = 0;
					Collect *collct = Collect::CreateCollectByPos(raid, tableDaily->TruckDrop1,monster->get_pos()->pos_x,10001, monster->get_pos()->pos_z, 0);
					if (collct != NULL)
					{
						collct->m_minType = 2;
					}
					sendState.state = DAILY__MINE_STATE_DEAD;
					raid->data->ai_data.zhenying_data.progress = DAILY__MINE_STATE_DEAD;
					raid->data->ai_data.zhenying_data.time_rest = time_helper::get_cached_time() / 1000 + tableDaily->ResurrectionTime;
					fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_DAILY_MINE_STATE_NOTIFY, mine_state__pack, sendState);
				}
			}
		}
	}

}
static void zhenying_raid_ai_collect(raid_struct *raid, player_struct *player, Collect *collect)
{
	if (collect == NULL)
	{
		return;
	}
	if (raid == NULL)
	{
		return;
	}
	if (player == NULL)
	{
		return;
	}
	CampDefenseTable *tableDaily = get_config_by_id(raid->data->ai_data.zhenying_data.camp, &zhenying_daily_config);
	if (tableDaily == NULL)
	{
		return;
	}
	uint64_t add = 0;
	if (collect->m_minType == 2)
	{
		add = tableDaily->CollectionIntegral1;
	}
	else if (collect->m_minType == 3)
	{
		add = tableDaily->CollectionIntegral2;
	}
	if (add > 0)
	{
		update_zhenying_score(player, add, tableDaily->TaskID);
	}
}

static void zhenying_raid_ai_player_ready(raid_struct *raid, player_struct *player)
{	
	if (raid->data->state != RAID_STATE_START)
	{
		raid->player_leave_raid(player);
//		player->send_scene_transfer(raid->m_config->FaceY, raid->m_config->ExitPointX, raid->m_config->BirthPointY,
//			raid->m_config->BirthPointZ, raid->m_config->ExitScene, 0);
	 	return;
	}

	CampDefenseTable *tableDaily = get_config_by_id(raid->data->ai_data.zhenying_data.camp, &zhenying_daily_config);
	if (tableDaily == NULL)
	{
		return;
	}
	EXTERN_DATA extern_data;
	extern_data.player_id = player->get_uuid();
	DailyMine sendMine;
	daily_mine__init(&sendMine);
	sendMine.cur = raid->data->ai_data.zhenying_data.cur;
	sendMine.max = tableDaily->TruckPlan;
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_DAILY_MINE_NOTIFY, daily_mine__pack, sendMine);
	MineState sendState;
	mine_state__init(&sendState);
	sendState.state = raid->data->ai_data.zhenying_data.progress;
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_DAILY_MINE_STATE_NOTIFY, mine_state__pack, sendState);

}

static void zhenying_raid_ai_player_relive(raid_struct *raid, player_struct *player, uint32_t type)
{
	ReliveNotify nty;
	relive_notify__init(&nty);
	nty.playerid = player->get_uuid();
	nty.type = type;
	if (type == 1)	//原地复活
	{
		if (raid->m_config->InstantRelive == 0)
		{
			LOG_ERR("%s player[%lu] can not relive type 1 in raid %u", __FUNCTION__, player->get_uuid(), raid->m_id);
			return;
		}

		int relive_times = player->get_attr(PLAYER_ATTR_RELIVE_TYPE1);
		if (relive_times >= sg_relive_free_times)
		{
			int fin_cost = (relive_times - sg_relive_free_times) * sg_relive_grow_cost + sg_relive_first_cost;
			if (fin_cost > sg_relive_max_cost)
				fin_cost = sg_relive_max_cost;
			if (player->get_comm_gold() < fin_cost)
			{
				LOG_ERR("%s: player[%lu] do not have enough gold", __FUNCTION__, player->get_uuid());
				return;
			}
			player->sub_comm_gold(fin_cost, MAGIC_TYPE_RELIVE);
		}

		++player->data->attrData[PLAYER_ATTR_RELIVE_TYPE1];
		++player->data->attrData[PLAYER_ATTR_RELIVE_TYPE2];
		player->broadcast_to_sight(MSG_ID_RELIVE_NOTIFY, &nty, (pack_func)relive_notify__pack, true);
		//player->add_achievement_progress(ACType_RELIVE, 0, 0, 1);
	}
	else
	{
		std::vector<struct FactionBattleTable*>::iterator itF = zhenying_battle_config.begin();
		FactionBattleTable *table = *itF;
		if (table != NULL)
		{
			int x, z;
			double direct = 0;
			zhenying_raid_manager::GetRelivePos(table, player->get_attr(PLAYER_ATTR_ZHENYING), &x, &z, &direct);
			nty.pos_x = x;
			nty.pos_z = z;
			nty.direct = direct;
		}
		LOG_DEBUG("%s: player[%lu] relive to pos[%d][%d][%d]", __FUNCTION__, player->get_uuid(), nty.pos_x, nty.pos_z, nty.direct);
		EXTERN_DATA extern_data;
		extern_data.player_id = player->get_uuid();
		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_RELIVE_NOTIFY, relive_notify__pack, nty);

		player->send_clear_sight();
		scene_struct *t_scene = raid;
		raid->delete_player_from_scene(player);
		player->set_pos(nty.pos_x, nty.pos_z);
		int camp_id = player->get_camp_id();
		t_scene->add_player_to_scene(player);
		player->set_camp_id(camp_id);
	}

	player->data->attrData[PLAYER_ATTR_HP] = player->data->attrData[PLAYER_ATTR_MAXHP];
	player->on_hp_changed(0);

	//复活的时候加上一个无敌buff
	buff_manager::create_default_buff(114400001, player, player, false);
	player->m_team == NULL ? true : player->m_team->OnMemberHpChange(*player);
}

static void zhenying_raid_ai_attack(raid_struct *raid, player_struct *player, unit_struct *target, int damage)
{

}

static void zhenying_raid_ai_escort_end_piont(raid_struct *raid, monster_struct *monster)
{
	CampDefenseTable *tableDaily = get_config_by_id(raid->data->ai_data.zhenying_data.camp, &zhenying_daily_config);
	if (tableDaily == NULL)
	{
		return;
	}
	if (monster->data->monster_id == tableDaily->TruckID)
	{
		raid->data->ai_data.zhenying_data.cur = 0;
		raid->delete_monster_from_scene(monster, true);
		monster_manager::delete_monster(monster);
		MineState sendState;
		mine_state__init(&sendState);
		sendState.state = DAILY__MINE_STATE_COMPLETE;
		raid->data->ai_data.zhenying_data.progress = DAILY__MINE_STATE_COMPLETE;
		raid->data->ai_data.zhenying_data.time_rest = time_helper::get_cached_time() / 1000 + tableDaily->ResurrectionTime;
		
		EXTERN_DATA extern_data;
		
		std::set<uint64_t> playerIds;
		raid->get_all_player(playerIds);
		for (std::set<uint64_t>::iterator it = playerIds.begin(); it != playerIds.end(); ++it)
		{
			if (get_entity_type(*it) == ENTITY_TYPE_AI_PLAYER)
			{
				continue;
			}
			player_struct *player = player_manager::get_player_by_id(*it);
			if (player != NULL)
			{
				Collect *collct = Collect::CreateCollectByPos(raid, tableDaily->TruckDrop2, monster->get_pos()->pos_x, 10001, monster->get_pos()->pos_z, 0);
				if (collct != NULL)
				{
					collct->m_minType = 3;
				}
				player->data->zhenying.protect_num = 0;
				player->data->zhenying.score_time = 0;

				extern_data.player_id = player->get_uuid();
				fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_DAILY_MINE_STATE_NOTIFY, mine_state__pack, sendState);
			}
		}
	}
}

struct raid_ai_interface raid_ai_zhenying_interface =
{
	zhenying_raid_ai_init,
	zhenying_raid_ai_tick,
	zhenying_raid_ai_player_enter,
	zhenying_raid_ai_player_leave,
	zhenying_raid_ai_player_dead,
	zhenying_raid_ai_player_relive,
	zhenying_raid_ai_monster_dead,
	zhenying_raid_ai_collect,
	zhenying_raid_ai_player_ready,
	zhenying_raid_ai_finished,
	zhenying_raid_ai_attack,
	NULL, //区域变化
	NULL, //护送结果
	NULL, //和npc对话
	NULL, //获取配置，主要是万妖谷的配置
	NULL, //失败
	NULL, //区域变化	
	zhenying_raid_ai_escort_end_piont, //矿车到达终点
};

