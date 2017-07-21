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

void normal_raid_ai_tick(raid_struct *raid)
{
	if (raid->data->state == RAID_STATE_START)// && m_config->Score == 1)
	{
		uint32_t star_param[3];
		uint32_t score_param[3];
		uint8_t old_star_bits = raid->data->star_bits;
		uint32_t star = raid->calc_raid_star(star_param, score_param);
		if (star <= 0)
		{
			raid->on_raid_failed(0);
			return;
		}

		if (raid->data->star_bits != old_star_bits && raid->need_show_star())
		{
			raid->send_star_changed_notify(star_param, score_param);
		}

		if (raid->data->pass_index < raid->m_config->n_PassType && raid->m_config->PassType[raid->data->pass_index] == 2)
		{
			uint64_t t = (time_helper::get_cached_time() - raid->data->start_time) / 1000;
			if (raid->m_config->PassValue[raid->data->pass_index] <= t)
				raid->add_raid_pass_value(2, raid->m_config);
		}
	}
}

static void normal_raid_ai_init(raid_struct *raid, player_struct *)
{
}

void normal_raid_ai_finished(raid_struct *raid)
{
	raid->clear_monster();

	uint32_t star_param[3];
	uint32_t score_param[3];
	uint32_t star = raid->calc_raid_star(star_param, score_param);
	if (star <= 0)
	{
		raid->on_raid_failed(0);
		return;
	}

	if (star > raid->m_config->n_Rewards)
		star = 1;

	RaidFinishNotify notify;
	raid_finish_notify__init(&notify);
	notify.result = 0;
	notify.raid_id = raid->data->ID;
	notify.n_star = 3;
	notify.star = star_param;
	notify.n_score_param = 3;
	notify.score_param = score_param;

	std::map<uint32_t, uint32_t> item_list;
	int n_item = 0;
	uint32_t item_id[MAX_ITEM_REWARD_PER_RAID];
	uint32_t item_num[MAX_ITEM_REWARD_PER_RAID];
	uint32_t gold = 0, exp = 0;


//	for (size_t i = 0; i < star; ++i)
//	{
//		get_drop_item(m_config->Rewards[i], item_list);
		if (get_drop_item(raid->m_config->Rewards[star - 1], item_list) == 0)
//		if (get_drop_item(m_config->Rewards[i], item_list) == 0)
		{
			for (std::map<uint32_t, uint32_t>::iterator ite = item_list.begin(); ite != item_list.end() && n_item < MAX_ITEM_REWARD_PER_RAID; ++ite)
			{
				int type = get_item_type(ite->first);
				switch (type)
				{
					case ITEM_TYPE_COIN:
						gold += ite->second;
						break;
					case ITEM_TYPE_EXP:
						exp += ite->second;
						break;
					default:
						item_id[n_item] = ite->first;
						item_num[n_item] = ite->second;
						++n_item;
				}
			}
		}
//	}
/*
	for (std::map<uint32_t, uint32_t>::iterator ite = item_list.begin(); ite != item_list.end() && n_item < MAX_ITEM_REWARD_PER_RAID; ++ite)
	{
		int type = get_item_type(ite->first);
		switch (type)
		{
			case ITEM_TYPE_COIN:
				notify.gold += ite->second;
				break;
			case ITEM_TYPE_EXP:
				notify.exp += ite->second;
				break;
			default:
				item_id[n_item] = ite->first;
				item_num[n_item] = ite->second;
				++n_item;
		}
	}
*/
	notify.item_id = &item_id[0];
	notify.item_num = &item_num[0];

//	broadcast_to_raid(MSG_ID_RAID_FINISHED_NOTIFY, &notify, (pack_func)raid_finish_notify__pack);
	raid->data->state = RAID_STATE_PASS;

	EXTERN_DATA extern_data;
	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		if (!raid->m_player[i])
			continue;
		extern_data.player_id = raid->m_player[i]->get_uuid();

		uint32_t num = raid->m_player[i]->get_raid_reward_count(raid->data->ID);
		if (raid->m_control_config->RewardTime > num)
		{
			notify.n_item_id = notify.n_item_num = n_item;
			notify.gold = gold;
			notify.exp = exp;
			fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_RAID_FINISHED_NOTIFY, raid_finish_notify__pack, notify);
			raid->m_player[i]->add_item_list_otherwise_send_mail(item_list, MAGIC_TYPE_RAID, 270200002, NULL, true);
			raid->m_player[i]->add_raid_reward_count(raid->data->ID);
			raid->m_player[i]->check_activity_progress(AM_RAID, raid->data->ID);
		}
		else
		{
			notify.n_item_id = notify.n_item_num = 0;
			notify.gold = 0;
			notify.exp = 0;
			fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_RAID_FINISHED_NOTIFY, raid_finish_notify__pack, notify);
		}

		raid->m_player[i]->add_task_progress(TCT_FINISH_RAID, raid->data->ID, 1);
	}
}

static void normal_raid_ai_player_enter(raid_struct *raid, player_struct *player)
{
}
static void normal_raid_ai_player_leave(raid_struct *raid, player_struct *player)
{
}
static void normal_raid_ai_player_dead(raid_struct *raid, player_struct *player, unit_struct *killer)
{
}
// static void normal_raid_ai_player_relive(raid_struct *raid, player_struct *player)
// {
// }
static void normal_raid_ai_monster_dead(raid_struct *raid, monster_struct *monster, unit_struct *killer)
{
}

struct raid_ai_interface raid_ai_normal_interface =
{
	normal_raid_ai_init,
	normal_raid_ai_tick,
	normal_raid_ai_player_enter,
	normal_raid_ai_player_leave,
	normal_raid_ai_player_dead,
	NULL, //normal_raid_ai_player_relive,
	normal_raid_ai_monster_dead,
	NULL,
	NULL,
	normal_raid_ai_finished,
};
