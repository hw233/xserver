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


void battle_raid_ai_tick(raid_struct *raid)
{
	ZhenyingBattle::GetInstance()->Tick(raid->data->ai_data.battle_data.room, raid);
}

static void battle_raid_ai_init(raid_struct *raid, player_struct *player)
{
	if (raid->m_config->DengeonRank != DUNGEON_TYPE_BATTLE)
	{
		int ret = ZhenyingBattle::GetInstance()->CreatePrivateBattle(*player, raid);
		if (ret != 0)
		{
			LOG_INFO("%s: player[%lu] ret = %d", __FUNCTION__, player->get_uuid(), ret);
		}
	}
}

void battle_raid_ai_finished(raid_struct *raid)
{
	
}

static void battle_raid_ai_failed(raid_struct *raid)
{
	raid->clear_monster();
	
	ZhenyingBattle::GetInstance()->Settle(raid->data->ai_data.battle_data.room);
}

static void battle_raid_ai_player_enter(raid_struct *raid, player_struct *player)
{
	LOG_INFO("%s: player[%lu] add to %lu", __FUNCTION__, player->get_uuid(), raid->data->uuid);
//	raid->ZHENYING_DATA.cur_player_num++;

	BattlefieldTable *table = get_config_by_id(raid->data->ai_data.battle_data.step + 360500001, &zhenying_fight_config);
	if (table == NULL)
	{
		return ;
	}
	
	FbCD notify;
	fb_cd__init(&notify);
	notify.cd = 0; 
	if (raid->data->start_time / 1000 + table->ReadyTime > time_helper::get_cached_time() / 1000)
	{
		notify.cd = raid->data->start_time / 1000 + table->ReadyTime - time_helper::get_cached_time() / 1000;
	}
	EXTERN_DATA extern_data;
	extern_data.player_id = player->get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_FIGHT_READY_CD_NOTIFY, fb_cd__pack, notify);

	notify.cd = 0;
	if (raid->data->start_time / 1000 + raid->m_config->FailValue[0] > time_helper::get_cached_time() / 1000)
	{
		notify.cd = raid->data->start_time / 1000 + raid->m_config->FailValue[0] - time_helper::get_cached_time() / 1000;
	}
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_FIGHT_CD_NOTIFY, fb_cd__pack, notify);

	ZhenyingBattle::GetInstance()->SendMyScore(*player);
}
static void battle_raid_ai_player_leave(raid_struct *raid, player_struct *player)
{
	LOG_INFO("%s: player[%lu] del from %lu", __FUNCTION__, player->get_uuid(), raid->data->uuid);	
	ZhenyingBattle::GetInstance()->LeaveRegion(raid->data->ai_data.battle_data.room, player, player->get_attr(PLAYER_ATTR_REGION_ID));
	ROOM_INFO *pRoom = ZhenyingBattle::GetInstance()->GetRoom(raid->data->ai_data.battle_data.room);
	if (pRoom != NULL && pRoom->m_state != REST_STATE && raid->m_config->DengeonRank != DUNGEON_TYPE_BATTLE_NEW)
	{
		ZhenyingBattle::GetInstance()->AddFbCd(*player);
	}
}

static void battle_raid_ai_player_dead(raid_struct *raid, player_struct *player, unit_struct *killer)
{
	if (killer == NULL || player == NULL) //这2个是同一个
	{
		return;
	}
	
	ZhenyingBattle::GetInstance()->KillEnemy(killer, *player);
}
static void battle_raid_ai_monster_dead(raid_struct *raid, monster_struct *monster, unit_struct *killer)
{

}
static void battle_raid_ai_collect(raid_struct *raid, player_struct *player, Collect *collect)
{

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
		ZhenyingBattle::GetInstance()->IntoRegion(raid->data->ai_data.battle_data.room, player, player->get_attr(PLAYER_ATTR_REGION_ID));
	}
	else
	{
		BattlefieldTable *table = get_config_by_id(raid->data->ai_data.battle_data.step + 360500001, &zhenying_fight_config);
		if (table != NULL)
		{
			int x, z;
			double direct = 0;
			ZhenyingBattle::GetInstance()->GetRelivePos(table, player->get_attr(PLAYER_ATTR_ZHENYING), &x, &z, &direct);
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
}

static void battle_raid_ai_player_region_changed(raid_struct *raid, player_struct *player, uint32_t old_region, uint32_t new_region)
{
	ZhenyingBattle::GetInstance()->OnRegionChanged(raid, player, old_region, new_region);
}

//副本阵营战(阵营攻防)  新手阵营战
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
	battle_raid_ai_player_region_changed,
	NULL, //护送结果
	NULL, //和npc对话
	NULL, //获取配置，主要是万妖谷的配置
	battle_raid_ai_failed, //失败
	NULL	
};
