#include <math.h>
#include "uuid.h"
#include <stdlib.h>
#include "game_event.h"
#include "raid_ai.h"
#include "server_proto.h"
#include "raid_ai_normal.h"
#include "raid_manager.h"
#include "time_helper.h"
#include "app_data_statis.h"
#include "unit.h"
#include "msgid.h"
#include "raid.pb-c.h"

void doufachang_raid_ai_finished(raid_struct *raid)
{
	raid->clear_monster();
// 	EXTERN_DATA extern_data;
// 	for (int i = 0; i < MAX_TEAM_MEM; ++i)
// 	{
// 		if (!raid->m_player[i])
// 			continue;
// 		extern_data.player_id = raid->m_player[i]->get_uuid();
//		fast_send_msg_base(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_DOUFACHANG_FB_PASS_NOTIFY, 0, 0);
// 	}
}

static void doufachang_send_raid_result(player_struct *attack, player_struct *defence, int result, uint32_t add_gold, bool notify)
{
	DOUFACHANG_CHALLENGE_ANSWER *ans = (DOUFACHANG_CHALLENGE_ANSWER *)conn_node_gamesrv::connecter.get_send_data();
	ans->attack = attack->get_uuid();
	if (defence->ai_data)
		ans->defence = defence->ai_data->origin_player_id;
	else
		ans->defence = defence->get_uuid();
	ans->result = result;
	ans->add_gold = add_gold;
	ans->notify = notify;
	
	EXTERN_DATA extern_data;
	extern_data.player_id = ans->attack;
	fast_send_msg_base(&conn_node_gamesrv::connecter, &extern_data, SERVER_PROTO_DOUFACHANG_CHALLENGE_ANSWER, sizeof(*ans), 0);
}

static void doufachang_raid_ai_player_leave(raid_struct *raid, player_struct *player)
{
	player->data->attrData[PLAYER_ATTR_HP] = player->data->attrData[PLAYER_ATTR_MAXHP];
	player->notify_one_attr_changed(PLAYER_ATTR_HP, player->data->attrData[PLAYER_ATTR_HP]);
	
	if (raid->data->state == RAID_STATE_PASS)
		return;
	raid->data->state = RAID_STATE_PASS;	
	player->add_item(sg_doufachang_raid_lose_reward[0], sg_doufachang_raid_lose_reward[1], MAGIC_TYPE_DOUFACHANG_REWARD, true);

	doufachang_send_raid_result(player, raid->m_player2[0], 1, sg_doufachang_raid_lose_reward[1], false);
//	player->del_all_formation_partner_from_scene(false);	
	// DOUFACHANG_CHALLENGE_ANSWER *ans = (DOUFACHANG_CHALLENGE_ANSWER *)conn_node_gamesrv::connecter.get_send_data();
	// ans->attack = player->get_uuid();
	// ans->defence = raid->m_player2[0]->data->origin_player_id;
	// //玩家输
	// ans->result = 1;
	
	// EXTERN_DATA extern_data;
	// extern_data.player_id = ans->attack;
	// fast_send_msg_base(&conn_node_gamesrv::connecter, &extern_data, SERVER_PROTO_DOUFACHANG_CHALLENGE_ANSWER, sizeof(*ans), 0);
}

static void doufachang_raid_ai_player_dead(raid_struct *raid, player_struct *player, unit_struct *killer)
{
	if (raid->data->state == RAID_STATE_PASS)
		return;
	
	raid->data->state = RAID_STATE_PASS;
	if (!raid->m_player[0] || !raid->m_player[0]->is_avaliable())
	{
		LOG_ERR("%s: raid[%p][%lu] can not find m_player[0]", __FUNCTION__, raid, raid->data->uuid);
		return;
	}
	if (!raid->m_player2[0] || !raid->m_player2[0]->is_avaliable())
	{
		LOG_ERR("%s: raid[%p][%lu] can not find m_player2[0]", __FUNCTION__, raid, raid->data->uuid);
		return;
	}

	if (player == raid->m_player[0])  //玩家死了，输
	{
		raid->m_player[0]->add_item(sg_doufachang_raid_lose_reward[0], sg_doufachang_raid_lose_reward[1], MAGIC_TYPE_DOUFACHANG_REWARD, true);
		doufachang_send_raid_result(raid->m_player[0], raid->m_player2[0], 1, sg_doufachang_raid_lose_reward[1], true);
	}
	else //否则是AI死了，赢
	{
		raid->m_player[0]->add_item(sg_doufachang_raid_win_reward[0], sg_doufachang_raid_win_reward[1], MAGIC_TYPE_DOUFACHANG_REWARD, true);
		doufachang_send_raid_result(raid->m_player[0], raid->m_player2[0], 0, sg_doufachang_raid_win_reward[1], true);
	}
//	raid->m_player[0]->del_all_formation_partner_from_scene(false);
	if (raid->m_player2[0]->ai_data)
		raid->m_player2[0]->ai_data->stop_ai = true;
}

static void doufachang_raid_ai_player_ready(raid_struct *raid, player_struct *player)
{
	if (get_entity_type(player->get_uuid()) != ENTITY_TYPE_PLAYER)
	{
		if (player->ai_data)
			player->ai_data->ai_patrol_config = robot_patrol_config[0];				
		return;
	}
	player->data->attrData[PLAYER_ATTR_HP] = player->data->attrData[PLAYER_ATTR_MAXHP];
	player->notify_one_attr_changed(PLAYER_ATTR_HP, player->data->attrData[PLAYER_ATTR_HP]);	
	raid->DOUFACHANG_DATA.pvp_raid_state = PVP_RAID_STATE_WAIT_START;
	raid->data->start_time = time_helper::get_cached_time() + 10 * 1000;
}

static void doufachang_raid_start(raid_struct *raid)
{
	raid->PVP_DATA.pvp_raid_state = PVP_RAID_STATE_START;
	raid->start_player_ai();
}

static void doufachang_raid_ai_tick(raid_struct *raid)
{
	uint32_t now = time_helper::get_cached_time() / 1000;
	int delta_time = now - raid->data->start_time / 1000;
//	LOG_DEBUG("%s %d state[%d]", __FUNCTION__, delta_time, raid->PVP_DATA.pvp_raid_state);
	switch (raid->PVP_DATA.pvp_raid_state)
	{
		case PVP_RAID_STATE_START:
		{
		}
		break;
		case PVP_RAID_STATE_WAIT_START:
		{
			if (delta_time >= 0)
				doufachang_raid_start(raid);
		}
		return;
		default:
			return;
	}
}

struct raid_ai_interface raid_ai_doufachang_interface =
{
	NULL,
	.raid_on_tick = doufachang_raid_ai_tick,
	NULL,
	.raid_on_player_leave = doufachang_raid_ai_player_leave,
	.raid_on_player_dead = doufachang_raid_ai_player_dead,
	NULL, 
	NULL,
	NULL,
	.raid_on_player_ready = doufachang_raid_ai_player_ready,
	doufachang_raid_ai_finished,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	doufachang_raid_ai_finished
};
