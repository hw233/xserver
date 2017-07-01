#include <math.h>
#include <stdlib.h>
#include "game_event.h"
#include "raid_ai.h"
#include "raid_ai_normal.h"
#include "raid_manager.h"
#include "buff_manager.h"
#include "collect.h"
#include "time_helper.h"
#include "app_data_statis.h"
#include "unit.h"
#include "msgid.h"
#include "raid.pb-c.h"
#include "zhenying.pb-c.h"
#include "monster_manager.h"
#include "excel_data.h"
#include "check_range.h"
#include "raid_ai_common.h"

static void script_raid_ai_init(raid_struct *raid, player_struct *player)
{
	//多人副本在副本初始化的时候，初始化AI
	if (raid->m_config->DengeonType == 1)
	{
		do_script_raid_init_cond(raid, &raid->SCRIPT_DATA.script_data);
	}
}

static void script_raid_ai_finished(raid_struct *raid)
{
	LOG_INFO("%s: raid[%u][%lu]", __FUNCTION__, raid->data->ID, raid->data->uuid);
	raid->SCRIPT_DATA.script_data.cur_index = raid->SCRIPT_DATA.script_data.script_config->size();
	normal_raid_ai_finished(raid);
}

static void script_raid_ai_monster_dead(raid_struct *raid, monster_struct *monster, unit_struct *killer)
{
	script_ai_common_monster_dead(raid, monster, killer, &raid->SCRIPT_DATA.script_data);
}

static void script_raid_ai_collect(raid_struct *raid, player_struct *player, Collect *collect)
{
	script_ai_common_collect(raid, player, collect, &raid->SCRIPT_DATA.script_data);
}

static void script_raid_ai_tick(raid_struct *raid)
{
	script_ai_common_tick(raid, &raid->SCRIPT_DATA.script_data);
}

static void script_raid_ai_player_enter(raid_struct *raid, player_struct *player)
{
	if (raid->m_config->TaskID > 0)
	{
		if (player->m_team)
		{
			if (player->m_team->GetLeadId() == player->get_uuid())
			{
				player->accept_task(raid->m_config->TaskID, false);
			}
		}
	}		
}
static void script_raid_ai_player_leave(raid_struct *raid, player_struct *player)
{
	LOG_INFO("%s: player[%lu] del from %lu", __FUNCTION__, player->get_uuid(), raid->data->uuid);
}

static void script_raid_ai_player_ready(raid_struct *raid, player_struct *player)
{
	script_ai_common_player_ready(raid, player, &raid->SCRIPT_DATA.script_data);
}

static void script_raid_ai_escort_stop(raid_struct *raid, player_struct *player, uint32_t escort_id, bool success)
{
	script_ai_common_escort_stop(raid, player, escort_id, success, &raid->SCRIPT_DATA.script_data);
}

static void script_raid_ai_npc_talk(raid_struct *raid, player_struct *player, uint32_t npc_id)
{
	script_ai_common_npc_talk(raid, npc_id, &raid->SCRIPT_DATA.script_data);
}

struct raid_ai_interface raid_ai_script_interface =
{
	script_raid_ai_init,
	script_raid_ai_tick,
	script_raid_ai_player_enter,
	script_raid_ai_player_leave,
	NULL,
	NULL, //normal_raid_ai_player_relive,
	script_raid_ai_monster_dead,
	script_raid_ai_collect,
	script_raid_ai_player_ready,
	script_raid_ai_finished,
	NULL,
	NULL,
	script_raid_ai_escort_stop,
	.raid_on_npc_talk = script_raid_ai_npc_talk,
};
