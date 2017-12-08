#include <math.h>
#include "cast_skill.pb-c.h"
#include "so_game_srv/buff_manager.h"
#include <stdlib.h>
#include "game_event.h"
#include "raid_ai.h"
#include "collect.h"
#include "raid_manager.h"
#include "time_helper.h"
#include "unit.h"
#include "msgid.h"
#include "wanyaogu.pb-c.h"
#include "raid.pb-c.h"
#include "scene_transfer.pb-c.h"
#include "relive.pb-c.h"
#include "app_data_statis.h"
#include "raid_ai_common.h"
#include "server_level.h"

static int get_wanyaogu_cur_raid_index(raid_struct *raid);

//万妖谷完成一个关卡
static void wanyaogu_end_one_raid(raid_struct *raid, bool fail)
{
	LOG_INFO("%s: raid[%p][%lu][%u] fail[%d]", __FUNCTION__, raid, raid->data->uuid, raid->m_id, fail);

	raid->data->pass_index = 0;
	raid->data->pass_value = 0;	
	
		//完成当前关卡，开始烧烤，进入下一关卡
	raid->WANYAOGU_DATA.wanyaogu_state = WANYAOGU_STATE_BBQ;
	raid->WANYAOGU_DATA.timer1 = time_helper::get_cached_time() + sg_wanyaogu_time_total * 1000;
	raid->WANYAOGU_DATA.timer2 = time_helper::get_cached_time() + sg_wanyaogu_time_delta * 1000;				
//	raid->stop_monster_ai();
	raid->clear_monster();
	raid->clear_all_collet();
	raid->mark_finished = 0;   //让tick能继续执行

	// for (int i = 0; i < MAX_TEAM_MEM; ++i)
	// {
	// 	if (!raid->m_player[i])
	// 		continue;
	// 	raid->m_player[i]->data->cur_sight_monster = 0;
	// }
	
	WanyaoguBbqNotify notify;
	wanyaogu_bbq_notify__init(&notify);
	if (fail)
		notify.result = 1;
//		notify.pos_x = 0;
//		notify.pos_z = 0;
	raid->broadcast_to_raid(MSG_ID_WANYAOKA_BBQ_NOTIFY, &notify, (pack_func)wanyaogu_bbq_notify__pack);
	
//	raid->broadcast_to_raid(MSG_ID_WANYAOKA_BBQ_NOTIFY, NULL, NULL);

	int i = get_wanyaogu_cur_raid_index(raid);
	if (i >= 0)
	{
		if (!fail)
		{
			raid->WANYAOGU_DATA.raid_pass[i] = 1;
		}
	}

		//计算万妖卡
	int param_index = 0;
	for (uint32_t i = 0; i < raid->WANYAOGU_DATA.m_config->n_wanyaoka; ++i)
	{
		RandomCardTable *config = get_config_by_id(raid->WANYAOGU_DATA.m_config->wanyaoka[i], &wanyaoka_config);
		if (!config)
			continue;
		bool get = true;
		for (uint32_t j = 0; j < config->n_Condition; ++j, ++param_index)
		{
				//通关副本
			if (config->Condition[j] == 1 && fail)
			{
				get = false;
				break;
			}

				//杀怪，采集
			if (config->Condition[j] == 2 ||
				config->Condition[j] == 3)
			{
				if (raid->WANYAOGU_DATA.wanyaoka_cond_param[param_index] < config->Parameter2[j])
				{
					get = false;
					break;
				}
			}

				//死亡次数
			if (config->Condition[j] == 4)
			{
				if (raid->data->dead_count > config->Parameter1[j])
				{
					get = false;
					break;
				}
			}
		}
		
		if (get)
		{
			for (uint32_t i = 0; i < MAX_WANYAOKA_EACH_TIME; ++i)
			{
				if (raid->WANYAOGU_DATA.wanyaoka_id[i] == 0)
				{
					raid->WANYAOGU_DATA.wanyaoka_id[i] = config->CardID;
					break;
				}
			}
		}
	}
}

static void reward_wanyaoka(raid_struct *raid)
{
	WanyaokaGetNotify nty;
	wanyaoka_get_notify__init(&nty);
	nty.wanyaoka_id = raid->WANYAOGU_DATA.wanyaoka_id;
	for (uint32_t i = 0; i < MAX_WANYAOKA_EACH_TIME; ++i)
	{
		if (raid->WANYAOGU_DATA.wanyaoka_id[i] == 0)
			break;
		++nty.n_wanyaoka_id;
	}

	
	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		if (!raid->m_player[i] || !raid->m_player[i]->is_online())
			continue;
		uint32_t num = raid->m_player[i]->get_raid_reward_count(raid->data->ID);
		if (raid->m_control_config->RewardTime <= num)
			continue;
		
//		raid->broadcast_to_raid(MSG_ID_WANYAOKA_GET_NOTIFY, &nty, (pack_func)wanyaoka_get_notify__pack);
		EXTERN_DATA extern_data;
		extern_data.player_id = raid->m_player[i]->get_uuid();
		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data,
			MSG_ID_WANYAOKA_GET_NOTIFY, wanyaoka_get_notify__pack, nty);
		raid->m_player[i]->add_wanyaoka(nty.wanyaoka_id, nty.n_wanyaoka_id);
	}
}

static void send_raid_reward(raid_struct *raid, int star)
{
	RaidFinishNotify notify;
	raid_finish_notify__init(&notify);
	notify.result = 0;
	notify.raid_id = raid->data->ID;
	notify.n_star = MAX_WANYAOGU_RAID_NUM;
	notify.n_score_param = MAX_WANYAOGU_RAID_NUM;	
//	uint32_t star_param[3] = {0, 0, 0};
//	for (int i = 0; i < star; ++i)
//		star_param[i] = 1;
	notify.star = raid->WANYAOGU_DATA.raid_pass;
	notify.score_param = raid->WANYAOGU_DATA.wanyaogu_raid_id;
	// notify.n_score_param = 3;
	// notify.score_param = score_param;

	std::map<uint32_t, uint32_t> item_list;
	int n_item = 0;
	uint32_t item_id[MAX_ITEM_REWARD_PER_RAID];
	uint32_t item_num[MAX_ITEM_REWARD_PER_RAID];
	uint32_t gold = 0, exp = 0;

	reward_wanyaoka(raid);

	uint32_t drop_id = get_drop_by_lv(raid->lv, star, raid->m_config->n_Rewards, raid->m_config->Rewards,
		raid->m_config->n_ItemRewardSection, raid->m_config->ItemRewardSection);

	if (get_drop_item(drop_id, item_list) == 0)
	{
		for (std::map<uint32_t, uint32_t>::iterator ite = item_list.begin(); ite != item_list.end() && n_item < MAX_ITEM_REWARD_PER_RAID; ++ite)
		{
			// int type = get_item_type(ite->first);
			// switch (type)
			// {
			// 	case ITEM_TYPE_COIN:
			// 		gold += ite->second;
			// 		break;
			// 	case ITEM_TYPE_EXP:
			// 		exp += ite->second;
			// 		break;
			// 	default:
			// 		item_id[n_item] = ite->first;
			// 		item_num[n_item] = ite->second;
			// 		++n_item;
			// }
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

	EXTERN_DATA extern_data;
	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		if (!raid->m_player[i])
			continue;
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

			notify.n_item_id = notify.n_item_num = n_item;
			notify.gold = _gold;
			notify.exp = _exp;
			fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_RAID_FINISHED_NOTIFY, raid_finish_notify__pack, notify);

			raid->m_player[i]->add_exp(_exp, MAGIC_TYPE_RAID);
			raid->m_player[i]->add_coin(_gold, MAGIC_TYPE_RAID);
			raid->m_player[i]->add_item_list_otherwise_send_mail(item_list, MAGIC_TYPE_RAID, 270200002, NULL, true);
			raid->m_player[i]->add_raid_reward_count(raid->data->ID);
			raid->m_player[i]->check_activity_progress(AM_RAID, raid->data->ID);
//			raid->m_player[i]->send_raid_earning_time_notify();
		}
		else
		{
			notify.n_item_id = notify.n_item_num = 0;
			notify.gold = 0;
			notify.exp = 0;
			fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_RAID_FINISHED_NOTIFY, raid_finish_notify__pack, notify);
		}

		raid->m_player[i]->add_task_progress(TCT_WANYAOGU, raid->data->ID, 1);
		server_level_listen_raid_finish(raid->data->ID, raid->m_player[i]);
		raid->m_player[i]->add_achievement_progress(ACType_RAID_PASS_STAR, raid->data->ID, star, 0, 1);
	}
}

bool player_near_bbq(raid_struct *raid, player_struct *player)
{
	if (raid->WANYAOGU_DATA.m_config->n_RewardPosition != 3)
		return false;
	struct position *pos = player->get_pos();
	if (abs((int)(pos->pos_x - raid->WANYAOGU_DATA.m_config->RewardPosition[0])) > sg_wanyaogu_range)
		return false;
	if (abs((int)(pos->pos_z - raid->WANYAOGU_DATA.m_config->RewardPosition[2])) > sg_wanyaogu_range)
		return false;	
	return true;
}

static int get_wanyaogu_cur_raid_index(raid_struct *raid)
{
	for (int i = 0; i < MAX_WANYAOGU_RAID_NUM; ++i)
	{
		if (raid->WANYAOGU_DATA.wanyaogu_raid_id[i] == raid->m_id)
		{
			return i;
		}
	}
	return -1;
}

static int count_wanyaogu_star(raid_struct *raid)
{
	int ret = 0;
	for (int i = 0; i < MAX_WANYAOGU_RAID_NUM; ++i)
	{
		if (raid->WANYAOGU_DATA.raid_pass[i])
			++ret;
	}
	return (ret);
}

static void wanyaogu_raid_ai_tick(raid_struct *raid)
{
	switch (raid->WANYAOGU_DATA.wanyaogu_state)
	{
//		case WANYAOGU_STATE_INIT:
//			break;
		case WANYAOGU_STATE_WAIT_START:
		{
			if (time_helper::get_cached_time() > raid->WANYAOGU_DATA.timer1)
			{
				LOG_INFO("%s %d: start raid[%p][%lu][%u]", __FUNCTION__, __LINE__, raid, raid->data->uuid, raid->m_id);
				
				raid->WANYAOGU_DATA.wanyaogu_state = WANYAOGU_STATE_START;
				raid->start_monster_ai();

				if (raid->m_config->DengeonType == 1 && raid->WANYAOGU_DATA.script_data.script_config)
				{
					do_script_raid_init_cond(raid, &raid->WANYAOGU_DATA.script_data);
				}

			}
		}
		break;
		case WANYAOGU_STATE_BBQ:
		{
			if (time_helper::get_cached_time() > raid->WANYAOGU_DATA.timer2)
			{
				raid->WANYAOGU_DATA.timer2 = raid->WANYAOGU_DATA.timer2 + sg_wanyaogu_time_delta * 1000;
					// 增加经验
				for (int i = 0; i < MAX_TEAM_MEM; ++i)
				{
					if (!raid->m_player[i] || !raid->m_player[i]->is_alive())
						continue;
					if (player_near_bbq(raid, raid->m_player[i]))
						raid->m_player[i]->add_exp(raid->m_player[i]->get_attr(PLAYER_ATTR_LEVEL) * sg_wanyaogu_reward, MAGIC_TYPE_WANYAOGU_BBQ, true);
				}
			}
			
			if (time_helper::get_cached_time() > raid->WANYAOGU_DATA.timer1)
			{
				LOG_INFO("%s %d: end raid[%p][%lu][%u]", __FUNCTION__, __LINE__, raid, raid->data->uuid, raid->m_id);
				
				raid->clear_monster();

				int i = get_wanyaogu_cur_raid_index(raid);
				if (i >= 0)
				{
					if (i + 1 == MAX_WANYAOGU_RAID_NUM || raid->WANYAOGU_DATA.wanyaogu_raid_id[i + 1] == 0)
					{
							// 副本结束
						raid->WANYAOGU_DATA.wanyaogu_state = WANYAOGU_STATE_FINISH;
						int star = count_wanyaogu_star(raid);
						if (star == 0)//失败
						{
							raid->data->state = RAID_STATE_PASS;
							RaidFinishNotify notify;
							raid_finish_notify__init(&notify);
							notify.result = -1;
							notify.raid_id = raid->data->ID;
							raid->broadcast_to_raid(MSG_ID_RAID_FINISHED_NOTIFY, &notify, (pack_func)raid_finish_notify__pack);
						}
						else
						{
							send_raid_reward(raid, star);
						}
					}
					else
					{
							//进入下一关卡
						for (int j = 0; j < MAX_TEAM_MEM; ++j)
						{
							if (!raid->m_player[j])
								continue;
							raid->delete_player_from_scene(raid->m_player[j]);
						}
							
						raid->WANYAOGU_DATA.wanyaogu_state = WANYAOGU_STATE_INIT;
						raid->scene_struct::clear();
						raid->m_id = raid->WANYAOGU_DATA.wanyaogu_raid_id[i + 1];
						raid->WANYAOGU_DATA.m_config = get_config_by_id(raid->m_id, &all_raid_config);
						assert(raid->WANYAOGU_DATA.m_config);
						raid->init_common_script_data(raid->WANYAOGU_DATA.m_config->DungeonPass, &raid->WANYAOGU_DATA.script_data);
//						raid->WANYAOGU_DATA.m_control_config = get_config_by_id(raid->WANYAOGU_DATA.m_config->ActivityControl, &all_control_config);
//						assert(raid->WANYAOGU_DATA.m_control_config);

						if (raid->m_config->DynamicLevel == 0)
						{
							raid->init_scene_struct(raid->m_id, false, 0);							
						}
						else
						{							
							raid->init_scene_struct(raid->m_id, false, raid->lv);
						}
						raid->stop_monster_ai();

						for (int j = 0; j < MAX_TEAM_MEM; ++j)
						{
							if (!raid->m_player[j])
								continue;
							if (!raid->m_player[j]->is_alive())
							{
								double hp = raid->m_player[j]->data->attrData[PLAYER_ATTR_MAXHP];
								raid->m_player[j]->data->attrData[PLAYER_ATTR_HP] = hp;
								raid->m_player[j]->on_hp_changed(0);
//								uint32_t id[1];
//								double value[1];
//								id[0] = PLAYER_ATTR_HP;
//								value[0] = hp;
//								raid->m_player[j]->notify_attr_changed(1, id, value);

								ReliveNotify nty;
								relive_notify__init(&nty);
								nty.playerid = raid->m_player[j]->get_uuid();
								nty.type = 1;
								EXTERN_DATA extern_data;
								extern_data.player_id = nty.playerid;
								fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_RELIVE_NOTIFY, relive_notify__pack, nty);
//								broadcast_to_sight(MSG_ID_RELIVE_NOTIFY, &nty, (pack_func)relive_notify__pack, true);

								if (raid->m_player[j]->m_team)
									raid->m_player[j]->m_team->OnMemberHpChange(*raid->m_player[j]);

							}
							raid->m_player[j]->set_enter_raid_pos_and_scene(raid, raid->res_config->BirthPointX, raid->res_config->BirthPointZ);
						}

						SceneTransferAnswer resp;
						scene_transfer_answer__init(&resp);
						resp.direct = raid->m_born_direct;
						resp.new_scene_id = raid->m_id;
						resp.pos_x = raid->m_born_x;
						resp.pos_y = raid->m_born_y;
						resp.pos_z = raid->m_born_z;
//							fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_TRANSFER_ANSWER, scene_transfer_answer__pack, resp);
						raid->broadcast_to_raid(MSG_ID_TRANSFER_ANSWER, &resp, (pack_func)scene_transfer_answer__pack);
					}
					break;
				}
			}
		}
		break;
		case WANYAOGU_STATE_START:
		{
			// 	uint32_t param;
			// 	//失败了
			// if (raid->check_cond_finished(0, raid->WANYAOGU_DATA.m_config->Score[0],
			// 		raid->WANYAOGU_DATA.m_config->ScoreValue[0],
			// 		raid->WANYAOGU_DATA.m_config->ScoreValue1[0], &param) == 0)
			// {
			// 	wanyaogu_end_one_raid(raid, true);				
			// }
			// else if (raid->check_raid_timeout())
			// {
			// 	wanyaogu_end_one_raid(raid, true);
			// }
			if (raid->WANYAOGU_DATA.script_data.script_config)
			{
				script_ai_common_tick(raid, &raid->WANYAOGU_DATA.script_data);
			}

			// if (raid->data->pass_index < raid->WANYAOGU_DATA.m_config->n_PassType && raid->WANYAOGU_DATA.m_config->PassType[raid->data->pass_index] == 2)
			// {
			// 	uint64_t t = (time_helper::get_cached_time() - raid->data->start_time) / 1000;
			// 	if (raid->WANYAOGU_DATA.m_config->PassValue[raid->data->pass_index] <= t)
			// 		raid->add_raid_pass_value(2, raid->WANYAOGU_DATA.m_config);
			// }
		}
		break;
		default:
			break;
	}
}

static void wanyaogu_raid_ai_init(raid_struct *raid, player_struct *player)
{
}

static void wanyaogu_raid_ai_player_ready(raid_struct *raid, player_struct *player)
{
	if (raid->WANYAOGU_DATA.wanyaogu_state == WANYAOGU_STATE_INIT)
	{
		LOG_INFO("%s %d: wait start raid[%p][%lu][%u]", __FUNCTION__, __LINE__, raid, raid->data->uuid, raid->m_id);
		raid->WANYAOGU_DATA.wanyaogu_state = WANYAOGU_STATE_WAIT_START;
		raid->WANYAOGU_DATA.timer1 = time_helper::get_cached_time() + sg_wanyaogu_start_time * 1000;
		raid->data->start_time = raid->WANYAOGU_DATA.timer1;
	}

	WanyaoguStartNotify notify;
	wanyaogu_start_notify__init(&notify);
	notify.start_time = raid->data->start_time / 1000;//raid->WANYAOGU_DATA.timer1 / 1000;

	EXTERN_DATA ext_data;
	ext_data.player_id = player->get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_WANYAOKA_START_NOTIFY, wanyaogu_start_notify__pack, notify);

	if (raid->WANYAOGU_DATA.script_data.script_config)
	{
		script_ai_common_player_ready(raid, player, &raid->WANYAOGU_DATA.script_data);
	}
	
//	raid->broadcast_to_raid(MSG_ID_WANYAOKA_START_NOTIFY, &notify, (pack_func)wanyaogu_start_notify__pack);
}

static void wanyaogu_raid_ai_player_enter(raid_struct *raid, player_struct *player)
{
}
static void wanyaogu_raid_ai_player_leave(raid_struct *raid, player_struct *player)
{
}
static void wanyaogu_raid_ai_player_dead(raid_struct *raid, player_struct *player, unit_struct *killer)
{
}
// static void wanyaogu_raid_ai_player_relive(raid_struct *raid, player_struct *player)
// {
// }

static struct DungeonTable *wanyaogu_raid_get_config(raid_struct *raid)
{
	if (raid->WANYAOGU_DATA.wanyaogu_state != WANYAOGU_STATE_START)
		return NULL;
	return raid->WANYAOGU_DATA.m_config;
}

static void wanyaogu_raid_ai_monster_dead(raid_struct *raid, monster_struct *monster, unit_struct *killer)
{
	int param_index = 0;
	for (uint32_t i = 0; i < raid->WANYAOGU_DATA.m_config->n_wanyaoka; ++i)
	{
		RandomCardTable *config = get_config_by_id(raid->WANYAOGU_DATA.m_config->wanyaoka[i], &wanyaoka_config);
		if (!config)
			continue;
		for (uint32_t j = 0; j < config->n_Condition; ++j, ++param_index)
		{
			if (config->Condition[j] != 2)
				continue;
			if (config->Parameter1[j] != monster->data->monster_id)
				continue;
			++raid->WANYAOGU_DATA.wanyaoka_cond_param[param_index];
		}
	}

		//杀死怪物
	// if (raid->WANYAOGU_DATA.m_config->PassType == 1)
	// {
	// 	if (raid->WANYAOGU_DATA.m_config->PassValue == monster->config->ID)
	// 	{
	// 		raid->on_raid_finished();
	// 	}
	// }

	// if (raid->data->pass_index < raid->WANYAOGU_DATA.m_config->n_PassType && raid->WANYAOGU_DATA.m_config->PassType[raid->data->pass_index] == 1)
	// {
	// 	if (raid->WANYAOGU_DATA.m_config->PassValue[raid->data->pass_index] == monster->config->ID)
	// 	{
	// 		if (raid->add_raid_pass_value(1, raid->WANYAOGU_DATA.m_config))
	// 		{
	// 			return;
	// 		}
	// 	}
	// }
	if (raid->WANYAOGU_DATA.script_data.script_config)
	{
		return script_ai_common_monster_dead(raid, monster, killer, &raid->WANYAOGU_DATA.script_data);
	}
}

static void wanyaogu_raid_ai_collect(raid_struct *raid, player_struct *player, Collect *collect)
{
	int param_index = 0;
	for (uint32_t i = 0; i < raid->WANYAOGU_DATA.m_config->n_wanyaoka; ++i)
	{
		RandomCardTable *config = get_config_by_id(raid->WANYAOGU_DATA.m_config->wanyaoka[i], &wanyaoka_config);
		if (!config)
			continue;
		for (uint32_t j = 0; j < config->n_Condition; ++j, ++param_index)
		{
			if (config->Condition[j] != 3)
				continue;
			if (config->Parameter1[j] != collect->m_collectId)
				continue;
			++raid->WANYAOGU_DATA.wanyaoka_cond_param[param_index];
		}
	}

	// if (raid->data->pass_index < raid->WANYAOGU_DATA.m_config->n_PassType && raid->WANYAOGU_DATA.m_config->PassType[raid->data->pass_index] == 3)
	// {
	// 	if (raid->WANYAOGU_DATA.m_config->PassValue[raid->data->pass_index] == collect->m_collectId)
	// 	{
	// 		if (raid->add_raid_pass_value(3, raid->WANYAOGU_DATA.m_config))
	// 		{
	// 			return;
	// 		}
	// 	}
	// }

	if (raid->WANYAOGU_DATA.script_data.script_config)
	{
		return script_ai_common_collect(raid, player, collect, &raid->WANYAOGU_DATA.script_data);
	}	
	
}

static void wanyaogu_raid_ai_failed(raid_struct *raid)
{
		//失败了
	wanyaogu_end_one_raid(raid, true);		
}

static void wanyaogu_raid_ai_finished(raid_struct *raid)
{
		//胜利了
	wanyaogu_end_one_raid(raid, false);	
}

static void wanyaogu_raid_ai_escort_stop(raid_struct *raid, player_struct *player, uint32_t escort_id, bool success)
{
	script_ai_common_escort_stop(raid, player, escort_id, success, &raid->WANYAOGU_DATA.script_data);
}

static void wanyaogu_raid_ai_monster_region_changed(raid_struct *raid, monster_struct *monster, uint32_t old_id, uint32_t new_id)
{
	uint32_t pk_type = monster->get_attr(PLAYER_ATTR_PK_TYPE);
	struct raid_script_data *data = &raid->WANYAOGU_DATA.script_data;
	for (size_t i = 0; i < data->cur_region_config; ++i)
	{
		bool pk_type_right = false;
		for (size_t j = 2; j < data->region_config[i]->n_Parameter1; ++j)
		{
			if (pk_type == data->region_config[i]->Parameter1[j])
			{
				pk_type_right = true;
				break;
			}
		}

		if (!pk_type_right)
			continue;
		
		if (data->region_config[i]->Parameter1[0] == new_id)
		{
			buff_manager::create_default_buff(data->region_config[i]->Parameter1[1], monster, monster, true);
			return;
		}
		else if (data->region_config[i]->Parameter1[0] == old_id)
		{
			monster->delete_one_buff(data->region_config[i]->Parameter1[1], true);
			// AddBuffNotify notify;
			// add_buff_notify__init(&notify);
			// notify.buff_id = data->region_config[i]->Parameter1[1];
			// notify.playerid = monster->get_uuid();
			// monster->broadcast_to_sight(MSG_ID_DEL_BUFF_NOTIFY, &notify, (pack_func)add_buff_notify__pack, false);
			return;
		}
	}	
}

static void wanyaogu_raid_ai_player_region_changed(raid_struct *raid, player_struct *player, uint32_t old_id, uint32_t new_id)
{
	// struct raid_script_data *data = &raid->WANYAOGU_DATA.script_data;
	// for (size_t i = 0; i < data->cur_region_config; ++i)
	// {
	// 	if (data->region_config[i]->Parameter1[0] == new_id)
	// 	{
	// 		buff_manager::create_default_buff(data->region_config[i]->Parameter1[1], player, player, true);
	// 		break;
	// 	}
	// }
}

struct raid_ai_interface raid_ai_wanyaogu_interface =
{
	wanyaogu_raid_ai_init,
	wanyaogu_raid_ai_tick,
	wanyaogu_raid_ai_player_enter,
	wanyaogu_raid_ai_player_leave,
	wanyaogu_raid_ai_player_dead,
	NULL,//wanyaogu_raid_ai_player_relive,
	wanyaogu_raid_ai_monster_dead,
	wanyaogu_raid_ai_collect,
	wanyaogu_raid_ai_player_ready,
	wanyaogu_raid_ai_finished,
	NULL,  //ai_player_attack
	.raid_on_player_region_changed = wanyaogu_raid_ai_player_region_changed,
	.raid_on_escort_stop = wanyaogu_raid_ai_escort_stop,
	NULL,
	.raid_get_config = wanyaogu_raid_get_config,
	.raid_on_failed = wanyaogu_raid_ai_failed,	
	.raid_on_monster_region_changed = wanyaogu_raid_ai_monster_region_changed,
};
