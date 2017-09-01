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
#include "zhenying_raid.h"
#include "buff_manager.h"
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

void battle_raid_ai_tick(raid_struct *raid)
{
}

static void battle_raid_ai_init(raid_struct *raid, player_struct *)
{
}

void battle_raid_ai_finished(raid_struct *raid)
{
	//raid->clear_monster();
	//raid->data->state = RAID_STATE_PASS; 
	//一直开
}

static void battle_raid_ai_player_enter(raid_struct *raid, player_struct *player)
{
	LOG_INFO("%s: player[%lu] add to %lu", __FUNCTION__, player->get_uuid(), raid->data->uuid);
//	raid->ZHENYING_DATA.cur_player_num++;

	BattlefieldTable *table = get_config_by_id(ZhenyingBattle::GetInstance()->GetStep(*player) + 360500001, &zhenying_fight_config);
	if (table == NULL)
	{
		return ;
	}
	
	FbCD notify;
	fb_cd__init(&notify);
	notify.cd = 0; 
	if (raid->data->start_time / 1000 + table->ReadyTime > time_helper::get_cached_time())
	{
		notify.cd = raid->data->start_time / 1000 + table->ReadyTime - time_helper::get_cached_time();
	}
	EXTERN_DATA extern_data;
	extern_data.player_id = player->get_uuid();
	notify.cd = 0;
	if (raid->data->start_time / 1000 + raid->m_config->FailValue[0] > time_helper::get_cached_time())
	{
		notify.cd = raid->data->start_time / 1000 + raid->m_config->FailValue[0] - time_helper::get_cached_time();
	}
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_FIGHT_CD_NOTIFY, fb_cd__pack, notify);

	ZhenyingBattle::GetInstance()->SendMyScore(*player);
}
static void battle_raid_ai_player_leave(raid_struct *raid, player_struct *player)
{
	LOG_INFO("%s: player[%lu] del from %lu", __FUNCTION__, player->get_uuid(), raid->data->uuid);	
//	raid->ZHENYING_DATA.cur_player_num--;

	for (std::deque<uint64_t>::iterator it = player->m_hitMe.begin(); it != player->m_hitMe.end(); ++it)
	{
		player_struct *hit = player_manager::get_player_by_id(*it);
		if (hit != NULL)
		{
			hit->m_meHit.erase(*it);
		}
	}
	player->m_hitMe.clear();

	for (std::set<uint64_t>::iterator it = player->m_meHit.begin(); it != player->m_meHit.end(); ++it)
	{
		player_struct *other = player_manager::get_player_by_id(*it);
		if (other != NULL)
		{
			std::deque<uint64_t>::iterator itf = std::find(other->m_hitMe.begin(), other->m_hitMe.end(), player->get_uuid());
			if (itf != other->m_hitMe.end())
			{
				other->m_hitMe.erase(itf);
			}
		}
	}
	player->m_meHit.clear();

	zhenying_raid_struct *zhenying = (zhenying_raid_struct *)raid;
	zhenying->m_hit_flag.erase(player->get_uuid());
}

static void UpdateOneTeamInfo(player_struct &player)
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
static void battle_raid_ai_player_dead(raid_struct *raid, player_struct *player, unit_struct *killer)
{
	if (killer == NULL || player == NULL)
	{
		return;
	}
	if (killer->get_unit_type() == UNIT_TYPE_PLAYER)
	{
		player_struct *pKill = (player_struct *)killer;

		pKill->add_zhenying_exp(5);
		++pKill->data->zhenying.kill;
		++pKill->data->zhenying.kill_week;
		int64_t addScore = pKill->data->zhenying.kill * 3 + pKill->data->zhenying.help - pKill->data->zhenying.death * 2;
		if (addScore < 0)
		{
			addScore = 0;
		}
		pKill->data->zhenying.score = addScore;
		//pKill->data->zhenying.score_week += 5;

		++player->data->zhenying.death;
		addScore = player->data->zhenying.kill * 3 + player->data->zhenying.help - player->data->zhenying.death * 2;
		if (addScore < 0)
		{
			addScore = 0;
		}
		player->data->zhenying.score = addScore; 

		UpdateOneTeamInfo(*player);
		UpdateOneTeamInfo(*pKill);

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

		for (std::deque<uint64_t>::iterator it = player->m_hitMe.begin(); it != player->m_hitMe.end(); ++it)
		{
			player_struct *hit = player_manager::get_player_by_id(*it);
			if (hit != NULL)
			{
				hit->m_meHit.erase(*it);
				++hit->data->zhenying.help;
			}
		}
		player->m_hitMe.clear();
	}
}
static void battle_raid_ai_monster_dead(raid_struct *raid, monster_struct *monster, unit_struct *killer)
{
	std::vector<struct FactionBattleTable*>::iterator itF = zhenying_battle_config.begin();
	FactionBattleTable *table = *itF;
	player_struct *pKill = NULL;
	if (killer->get_unit_type() == UNIT_TYPE_PLAYER)
	{
		pKill = (player_struct *)killer;
		update_task_process(3, pKill);
	}

	zhenying_raid_struct *zhenying = (zhenying_raid_struct *)raid;
	if (monster->config->Type == 4) //旗子
	{
		if (pKill != NULL)
		{
			pKill->add_zhenying_exp(table->FlagExp);
		}
		for (std::set<uint64_t>::iterator it = zhenying->m_hit_flag.begin(); it != zhenying->m_hit_flag.end(); ++it)
		{
			player_struct *other = player_manager::get_player_by_id(*it);
			if (other != NULL)
			{
				other->add_task_progress(TCT_ZHENYING_SCORE, 0, table->FlagIntegral);
			}
		}
		zhenying->m_hit_flag.clear();
	}
}
static void battle_raid_ai_collect(raid_struct *raid, player_struct *player, Collect *collect)
{
	if (collect == NULL)
	{
		return;
	}
	zhenying_raid_struct *zhenying = (zhenying_raid_struct *)raid;
	zhenying->delete_collect_pos(collect->m_pos.pos_x * collect->m_pos.pos_z);
	if (player == NULL)
	{
		return;
	}
	if (player->data->zhenying.mine < 1)
	{
		return;
	}
	
	GradeTable *table = get_config_by_id((36020 + player->get_attr(PLAYER_ATTR_ZHENYING)) * 10000 + player->data->zhenying.level, &zhenying_level_config);
	if (table == NULL)
	{
		return;
	}
	FactionBattleTable *table1 = get_zhenying_battle_table(player->get_attr(PLAYER_ATTR_LEVEL));
	if (table1 == NULL)
	{
		return;
	}
	if (collect->m_collectId != table1->BoxID)
	{
		return;
	}	
	uint32_t exp = table->LevelExp * table1->BoxExp / 100;
	if (exp < table1->BoxExp2)
	{
		exp = table1->BoxExp2;
	}
	player->add_zhenying_exp(exp);
	--player->data->zhenying.mine;
}

static void battle_raid_ai_player_ready(raid_struct *raid, player_struct *player)
{
	if (raid->data->state != RAID_STATE_START)
	{
		raid->player_leave_raid(player);
//		player->send_scene_transfer(raid->m_config->FaceY, raid->m_config->ExitPointX, raid->m_config->BirthPointY,
//			raid->m_config->BirthPointZ, raid->m_config->ExitScene, 0);
	 	return;
	}
}

static void battle_raid_ai_player_relive(raid_struct *raid, player_struct *player, uint32_t type)
{
	if (type == 1)	//原地复活
		return;
	player->data->attrData[PLAYER_ATTR_HP] = player->data->attrData[PLAYER_ATTR_MAXHP];
	player->on_hp_changed(0);

	ReliveNotify nty;
	relive_notify__init(&nty);
	nty.playerid = player->get_uuid();
	nty.type = type;
	std::vector<struct FactionBattleTable*>::iterator itF = zhenying_battle_config.begin();
	FactionBattleTable *table = *itF;
	if (table != NULL)
	{
		if (player->get_attr(PLAYER_ATTR_ZHENYING) == ZHENYING__TYPE__FULONGGUO)
		{
			nty.pos_x = table->BirthPoint1[0];
			nty.pos_z = table->BirthPoint1[2];
		}
		else
		{
			nty.pos_x = table->BirthPoint2[0];
			nty.pos_z = table->BirthPoint2[2];
		}
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

	//复活的时候加上一个无敌buff
	buff_manager::create_default_buff(114400001, player, player, false);

	player->m_team == NULL ? true : player->m_team->OnMemberHpChange(*player);
}

static void battle_raid_ai_attack(raid_struct *raid, player_struct *player, unit_struct *target, int damage)
{
	if (target->get_unit_type() == UNIT_TYPE_PLAYER)
	{
		player_struct *other = (player_struct *)target;
		std::deque<uint64_t>::iterator it = std::find(other->m_hitMe.begin(), other->m_hitMe.end(), player->get_uuid());
		if (it != other->m_hitMe.end())
		{
			other->m_hitMe.erase(it);
		}
		other->m_hitMe.push_back(player->get_uuid());
		if (other->m_hitMe.size() > 5)
		{
			player_struct *hit = player_manager::get_player_by_id(other->m_hitMe.front());
			if (hit != NULL)
			{
				hit->m_meHit.erase(other->m_hitMe.front());
			}
			other->m_hitMe.pop_front();
		}
	}
	else if (target->get_unit_type() == UNIT_TYPE_MONSTER)
	{
		monster_struct *monster = (monster_struct *)target;
		if (monster->config->Type == 4)
		{
			zhenying_raid_struct *zhenying = (zhenying_raid_struct *)raid;
			zhenying->m_hit_flag.insert(player->get_uuid());
		}
	}
}

struct raid_ai_interface raid_ai_battle_interface =
{
	battle_raid_ai_init,
	battle_raid_ai_tick,
	battle_raid_ai_player_enter,
	battle_raid_ai_player_leave,
	battle_raid_ai_player_dead,
	battle_raid_ai_player_relive,
	battle_raid_ai_monster_dead,
	battle_raid_ai_collect,
	battle_raid_ai_player_ready,
	battle_raid_ai_finished,
	battle_raid_ai_attack,
};
