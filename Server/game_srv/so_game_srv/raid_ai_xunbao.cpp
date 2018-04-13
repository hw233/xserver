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
#include "server_level.h"

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
		raid->m_player[i]->add_achievement_progress(ACType_RAID_PASS_STAR, raid->data->ID, 0, 0, 1);
		raid->m_player[i]->add_achievement_progress(ACType_RAID_PASS_TYPE, raid->m_config->DengeonRank, 0, 0, 1);
	}

	RaidFinishNotify notify;
	raid_finish_notify__init(&notify);
	notify.result = 0;
	notify.raid_id = raid->data->ID;
	notify.n_star = 0;
	std::map<uint32_t, uint32_t> item_list;
	int n_item = 0;
	uint32_t item_id[MAX_ITEM_REWARD_PER_RAID];
	uint32_t item_num[MAX_ITEM_REWARD_PER_RAID];
	uint32_t gold = 0, exp = 0;
	uint32_t drop_id = get_drop_by_lv(raid->lv, 3, raid->m_config->n_Rewards, raid->m_config->Rewards,
		raid->m_config->n_ItemRewardSection, raid->m_config->ItemRewardSection);
	if (get_drop_item(drop_id, item_list) == 0)
	{
		for (std::map<uint32_t, uint32_t>::iterator ite = item_list.begin(); ite != item_list.end() && n_item < MAX_ITEM_REWARD_PER_RAID; ++ite)
		{
			item_id[n_item] = ite->first;
			item_num[n_item] = ite->second;
			++n_item;
		}
	}
	gold = raid->m_config->MoneyReward;
	exp = raid->m_config->ExpReward;
	notify.item_id = &item_id[0];
	notify.item_num = &item_num[0];

	raid->data->state = RAID_STATE_PASS;

	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		if (!raid->m_player[i])
			continue;
		if (raid->m_player[i]->scene != raid)
		{
			continue;
		}

		extern_data.player_id = raid->m_player[i]->get_uuid();

		uint32_t num = raid->m_player[i]->get_raid_reward_count(raid->data->ID);
		if (raid->m_control_config->RewardTime > num)
		{
			int _gold = gold;
			int _exp = exp;
			if (raid->m_config->DynamicLevel)
			{
				_gold *= raid->m_player[i]->get_coin_rate();
				_exp *= raid->m_player[i]->get_exp_rate();
			}
			_gold += raid->m_config->MoneyReward1;
			_exp += raid->m_config->ExpReward1;

			notify.n_item_id = notify.n_item_num = n_item;
			notify.gold = _gold;
			notify.exp = _exp;
			fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_RAID_FINISHED_NOTIFY, raid_finish_notify__pack, notify);

			raid->m_player[i]->add_exp(_exp, MAGIC_TYPE_RAID);
			raid->m_player[i]->add_coin(_gold, MAGIC_TYPE_RAID);
			raid->m_player[i]->add_item_list_otherwise_send_mail(item_list, MAGIC_TYPE_RAID, 270200002, NULL, true);
			raid->m_player[i]->add_raid_reward_count(raid->data->ID);
			raid->m_player[i]->check_activity_progress(AM_RAID, raid->data->ID);
			if (raidid_to_hero_challenge_config.find(raid->data->ID) != raidid_to_hero_challenge_config.end())
			{
				raid->m_player[i]->check_activity_progress(AM_HERO_CHLLENGE, 0);
			}
		}
		else
		{
			notify.n_item_id = notify.n_item_num = 0;
			notify.gold = 0;
			notify.exp = 0;
			fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_RAID_FINISHED_NOTIFY, raid_finish_notify__pack, notify);
		}

		server_level_listen_raid_finish(raid->data->ID, raid->m_player[i]);
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
