#include <math.h>
#include <stdlib.h>
#include "game_event.h"
#include "raid_ai.h"
#include "raid_ai_normal.h"
#include "raid_manager.h"
#include "time_helper.h"
#include "app_data_statis.h"
#include "unit.h"
#include "msgid.h"
#include "raid.pb-c.h"

void xunbao_raid_ai_tick(raid_struct *raid)
{

}

static void xunbao_raid_ai_init(raid_struct *raid, player_struct *)
{
}

void xunbao_raid_ai_finished(raid_struct *raid)
{
	raid->clear_monster();
//	FbCD notify;
//	fb_cd__init(&notify);
	EXTERN_DATA extern_data;
	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		if (!raid->m_player[i])
			continue;
		extern_data.player_id = raid->m_player[i]->get_uuid();
		fast_send_msg_base(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_XUNBAO_FB_PASS_NOTIFY, 0, 0);
	}
}

static void xunbao_raid_ai_player_enter(raid_struct *raid, player_struct *player)
{
	FbCD notify;
	fb_cd__init(&notify);
	notify.cd = 0;
	if (time_helper::get_cached_time() < raid->data->start_time + raid->m_config->FailValue[0] * 1000)
	{
		notify.cd = raid->data->start_time + raid->m_config->FailValue[0] * 1000 - time_helper::get_cached_time();
		notify.cd /= 1000;
	}
	EXTERN_DATA extern_data;
	extern_data.player_id = player->get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_XUNBAO_FB_COUNTDOWN_NOTIFY, fb_cd__pack, notify);
}
static void xunbao_raid_ai_player_leave(raid_struct *raid, player_struct *player)
{
}
static void xunbao_raid_ai_player_dead(raid_struct *raid, player_struct *player, unit_struct *killer)
{
}
// static void normal_raid_ai_player_relive(raid_struct *raid, player_struct *player)
// {
// }
static void xunbao_raid_ai_monster_dead(raid_struct *raid, monster_struct *monster, unit_struct *killer)
{
}

struct raid_ai_interface raid_ai_xunbao_interface =
{
	xunbao_raid_ai_init,
	xunbao_raid_ai_tick,
	xunbao_raid_ai_player_enter,
	xunbao_raid_ai_player_leave,
	xunbao_raid_ai_player_dead,
	NULL, //normal_raid_ai_player_relive,
	xunbao_raid_ai_monster_dead,
	NULL,
	NULL,
	xunbao_raid_ai_finished,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	xunbao_raid_ai_finished
};
