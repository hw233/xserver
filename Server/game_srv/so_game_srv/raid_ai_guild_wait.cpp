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
#include "buff_manager.h"
#include "player_manager.h"
#include "guild_battle_manager.h"

void guild_wait_raid_ai_tick(raid_struct *raid)
{
}

static void guild_wait_raid_ai_init(raid_struct *raid, player_struct *)
{
}

void guild_wait_raid_ai_finished(raid_struct *raid)
{
	raid->clear_monster();
	raid->data->state = RAID_STATE_PASS;
}

static void guild_wait_raid_ai_player_enter(raid_struct *raid, player_struct *player)
{
	LOG_INFO("%s: player[%lu] add to %lu", __FUNCTION__, player->get_uuid(), raid->data->uuid);
//	assert(!player->m_team);
}
static void guild_wait_raid_ai_player_leave(raid_struct *raid, player_struct *player)
{
	LOG_INFO("%s: player[%lu] del from %lu", __FUNCTION__, player->get_uuid(), raid->data->uuid);
		// 移除等待队列，如果有队伍，退出队伍
	del_from_guild_battle_waiting(player);
//	assert(!player->m_team);
}
static void guild_wait_raid_ai_player_dead(raid_struct *raid, player_struct *player, unit_struct *killer)
{
}
static void guild_wait_raid_ai_monster_dead(raid_struct *raid, monster_struct *monster, unit_struct *killer)
{
}
static void guild_wait_raid_ai_collect(raid_struct *raid, player_struct *player, Collect *collect)
{
}

static void guild_wait_raid_ai_player_relive(raid_struct *raid, player_struct *player, uint32_t type)
{
}

static void guild_wait_raid_ai_attack(raid_struct *raid, player_struct *player, unit_struct *target, int damage)
{
}

static void guild_wait_raid_ai_player_ready(raid_struct *raid, player_struct *player)
{
	if (!is_guild_battle_opening())
	{
		return;
	}

	// 加入等待队列，不能有队伍
	add_to_guild_battle_waiting(player);

	//获取等待区信息
	EXTERN_DATA extern_data;
	extern_data.player_id = player->get_uuid();
	size_t pack_len = pack_guild_wait_info(player->data->guild_id, conn_node_base::get_send_data());
	fast_send_msg_base(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_GUILD_BATTLE_WAIT_INFO_NOTIFY, pack_len, 0);

	//发消息给guild_srv获取其他信息
	uint8_t *pData = conn_node_base::get_send_data();
	*pData++ = (is_guild_battle_settling());
	fast_send_msg_base(&conn_node_gamesrv::connecter, &extern_data, SERVER_PROTO_GUILD_BATTLE_ENTER_WAIT, sizeof(uint8_t), 0);
}

struct raid_ai_interface raid_ai_guild_wait_interface =
{
	guild_wait_raid_ai_init,
	guild_wait_raid_ai_tick,
	guild_wait_raid_ai_player_enter,
	guild_wait_raid_ai_player_leave,
	guild_wait_raid_ai_player_dead,
	guild_wait_raid_ai_player_relive,
	guild_wait_raid_ai_monster_dead,
	guild_wait_raid_ai_collect,
	guild_wait_raid_ai_player_ready,
	guild_wait_raid_ai_finished,
	guild_wait_raid_ai_attack,
};
