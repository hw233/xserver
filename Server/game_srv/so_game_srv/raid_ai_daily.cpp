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


void zhenying_daily_raid_ai_tick(raid_struct *raid)
{

}

static void zhenying_daily_raid_ai_init(raid_struct *raid, player_struct *player)
{
}

void zhenying_daily_raid_ai_finished(raid_struct *raid)
{
	
}

static void zhenying_daily_raid_ai_failed(raid_struct *raid)
{
	raid->clear_monster();
}

static void zhenying_daily_raid_ai_player_enter(raid_struct *raid, player_struct *player)
{
	LOG_INFO("%s: player[%lu] add to %lu", __FUNCTION__, player->get_uuid(), raid->data->uuid);
	
	FbCD notify;
	fb_cd__init(&notify);
	notify.cd = 0; 
	EXTERN_DATA extern_data;
	extern_data.player_id = player->get_uuid();
	if (raid->data->start_time / 1000 + raid->m_config->FailValue[0] > time_helper::get_cached_time() / 1000)
	{
		notify.cd = raid->data->start_time / 1000 + raid->m_config->FailValue[0] - time_helper::get_cached_time() / 1000;
	}
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_DAILY_CD_NOTIFY, fb_cd__pack, notify);
	
}
static void zhenying_daily_raid_ai_player_leave(raid_struct *raid, player_struct *player)
{
	LOG_INFO("%s: player[%lu] del from %lu", __FUNCTION__, player->get_uuid(), raid->data->uuid);	
}

static void zhenying_daily_raid_ai_player_dead(raid_struct *raid, player_struct *player, unit_struct *killer)
{
	if (killer == NULL || player == NULL) //这2个是同一个
	{
		return;
	}
}
static void zhenying_daily_raid_ai_monster_dead(raid_struct *raid, monster_struct *monster, unit_struct *killer)
{

}
static void zhenying_daily_raid_ai_collect(raid_struct *raid, player_struct *player, Collect *collect)
{

}

static void zhenying_daily_raid_ai_player_ready(raid_struct *raid, player_struct *player)
{
	if (raid->data->state != RAID_STATE_START)
	{
		raid->player_leave_raid(player);
	 	return;
	}
}

static void zhenying_daily_raid_ai_player_relive(raid_struct *raid, player_struct *player, uint32_t type)
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

static void zhenying_daily_raid_ai_attack(raid_struct *raid, player_struct *player, unit_struct *target, int damage)
{
	
}

static void zhenying_daily_raid_ai_player_region_changed(raid_struct *raid, player_struct *player, uint32_t old_region, uint32_t new_region)
{
	
}

struct raid_ai_interface raid_ai_zhenying_daily_interface =
{
	zhenying_daily_raid_ai_init,
	zhenying_daily_raid_ai_tick,
	zhenying_daily_raid_ai_player_enter,
	zhenying_daily_raid_ai_player_leave,
	zhenying_daily_raid_ai_player_dead,
	zhenying_daily_raid_ai_player_relive,
	zhenying_daily_raid_ai_monster_dead,
	zhenying_daily_raid_ai_collect,
	zhenying_daily_raid_ai_player_ready,
	zhenying_daily_raid_ai_finished,
	zhenying_daily_raid_ai_attack,
	zhenying_daily_raid_ai_player_region_changed,
	NULL, //护送结果
	NULL, //和npc对话
	NULL, //获取配置，主要是万妖谷的配置
	zhenying_daily_raid_ai_failed, //失败
	NULL	
};
