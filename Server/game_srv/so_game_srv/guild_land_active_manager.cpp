#include "guild_land_active_manager.h"
#include "team.h"
#include "time_helper.h"
#include "excel_data.h"
#include "lua_config.h"
#include "guild_land_raid_manager.h"
#include "raid_ai_common.h"
#include "guild_battle_manager.h"
#include "msgid.h"
#include "player_manager.h"
#include "uuid.h"
#include "monster_manager.h"
#include "check_range.h"
#include "app_data_statis.h"
#include "collect.h"
#include "guild.pb-c.h"



static const int GUILD_INTRUSION_CONTROLTABLE_ID = 440500001; //帮会入侵 FactionActivity表id

enum
{
	GUILD_BONFIRE_STATE_BEGIN = 1,
	GUILD_BONFIRE_STATE_END = 2,
};

struct GuildBonfireRewardInfo
{
	uint64_t player_id;
	uint32_t exp;
	uint32_t coin;
	uint32_t donation;
	uint32_t treasure;
	uint32_t build_board;
};

void guild_land_active_manager::guild_ruqin_active_open()
{

	uint64_t mnow_time = time_helper::get_cached_time() / 1000;
	FactionActivity *faction_table = get_config_by_id(GUILD_INTRUSION_CONTROLTABLE_ID, &guild_activ_config);
	if(faction_table == NULL)
		return;

	for(std::map<uint64_t, guild_land_raid_struct *>::iterator itr =  guild_land_raid_manager_all_raid_id.begin(); itr != guild_land_raid_manager_all_raid_id.end(); itr++)			
	{
		if(itr->second->ruqin_data.guild_ruqin == false)
		{
			//先判断当前时间在帮会领地是否有别的活动正在进行
			if(itr->second->GUILD_LAND_DATA.activity_id != 0)
			{
				LOG_ERR("[%s:%d] 开启门宗入侵活动失败,当前在门宗领地正有其他活动在进行,activity_id[%u],guild_id[%u]", __FUNCTION__, __LINE__, itr->second->GUILD_LAND_DATA.activity_id, itr->second->data->ai_data.guild_land_data.guild_id);
				continue;
			}
			//初始化数据
			itr->second->init_guild_ruqin_active_data();
			itr->second->ruqin_data.guild_ruqin = true;
			itr->second->ruqin_data.status = GUILD_RUQIN_ACTIVE_START;
			itr->second->GUILD_LAND_DATA.activity_id = GUILD_RUQIN_ACTIVITY_ID;
			//帮会入侵活动开启
			itr->second->ruqin_data.open_time = mnow_time;

			//这里要判断帮派阵营
			ProtoGuildInfo *info = get_guild_summary(itr->second->data->ai_data.guild_land_data.guild_id);	
			if(info == NULL)
				break;
			//发消息高guild服获取刷怪等级
			uint32_t *pData = (uint32_t*)conn_node_gamesrv::connecter.get_send_data();
			*pData++ = itr->second->data->ai_data.guild_land_data.guild_id;
			EXTERN_DATA extern_data;
			fast_send_msg_base(&conn_node_gamesrv::connecter, &extern_data, SERVER_PROTO_GUILD_RUQIN_CREAT_MONSTER_LEVEL_REQUEST, sizeof(uint32_t), 0);
			if(info->zhenying == 1)//人族
			{
				itr->second->ruqin_data.zhengying = 1;
				itr->second->init_common_script_data(faction_table->DungeonPass1, &(itr->second->data->ai_data.guild_land_data.script_data));
			}
			else if(info->zhenying == 2)
			{
				itr->second->ruqin_data.zhengying = 2;
				itr->second->init_common_script_data(faction_table->DungeonPass2, &(itr->second->data->ai_data.guild_land_data.script_data));
			}
			else 
			{
				break;
			}
			do_script_raid_init_cond(itr->second, &itr->second->data->ai_data.guild_land_data.script_data);
		}
	}
}
void guild_land_active_manager::guild_ruqin_active_stop()
{
	for(std::map<uint64_t, guild_land_raid_struct *>::iterator itr =  guild_land_raid_manager_all_raid_id.begin(); itr != guild_land_raid_manager_all_raid_id.end(); itr++)			
	{
		if(itr->second->ruqin_data.guild_ruqin == true )
		{
			itr->second->ruqin_data.status = GUILD_RUQIN_ACTIVE_FAILD;
		}
	}

}

//发放篝火奖励
void send_guild_bonfire_reward_to_guildsrv(uint32_t guild_id, std::vector<GuildBonfireRewardInfo> &rewards)
{
	if (rewards.size() == 0)
	{
		return;
	}

	GUILD_BONFIRE_REWARD *req = (GUILD_BONFIRE_REWARD*)conn_node_gamesrv::get_send_data();
	uint32_t data_len = sizeof(GUILD_BONFIRE_REWARD);
	memset(req, 0, data_len);
	req->activity_id = GUILD_BONFIRE_ACTIVITY_ID;
	req->guild_id = guild_id;
	req->player_num = 0;
	for (std::vector<GuildBonfireRewardInfo>::iterator iter = rewards.begin(); iter != rewards.end(); ++iter)
	{
		GuildBonfireRewardInfo &info = *iter;
		req->player_id[req->player_num] = info.player_id;
		req->donation[req->player_num] = info.donation;
		req->treasure[req->player_num] = info.treasure;
		req->build_board[req->player_num] = info.build_board;
		req->player_num++;
	}
	EXTERN_DATA ext_data;
	fast_send_msg_base(&conn_node_gamesrv::connecter, &ext_data, SERVER_PROTO_GUILD_BONFIRE_REWARD, data_len, 0);
}

void refresh_guild_bonfire_collection(guild_land_raid_struct *raid)
{
	if (sg_guild_bonfire_collections.size() == 0)
	{
		return;
	}

	uint32_t rand_idx = rand() % sg_guild_bonfire_collections.size();
	RandomCollectionTable *config = sg_guild_bonfire_collections[rand_idx];
	if (config->n_PointX == 0 || config->n_PointZ == 0)
	{
		LOG_ERR("[%s:%d] random collection pos miss, id:%lu", __FUNCTION__, __LINE__, config->ID);
		return;
	}

	rand_idx = rand() % config->n_PointX;
	Collect *pCollect = Collect::CreateCollectByPos(raid, config->CollectionID, config->PointX[rand_idx], 10000, config->PointZ[rand_idx], 0);
	if (!pCollect)
	{
		LOG_ERR("[%s:%d] create collect fail, random_id:%lu, collect_id:%lu", __FUNCTION__, __LINE__, config->ID, config->CollectionID);
	}
//	LOG_INFO("[%s:%d] collect_uuid:%lu, random_id:%lu, collect_id:%lu, pos_x:%f, pos_z:%f", __FUNCTION__, __LINE__, pCollect->m_uuid, config->ID, config->CollectionID, config->PointX[rand_idx], config->PointZ[rand_idx]);
}

void guild_land_active_manager::on_tick_10()
{
	struct tm tm;
	uint64_t mnow_time = time_helper::get_cached_time() / 1000;
	time_t now_time = mnow_time;
	localtime_r(&now_time, &tm);


	FactionActivity *faction_table = get_config_by_id(GUILD_INTRUSION_CONTROLTABLE_ID, &guild_activ_config);
	if(faction_table != NULL)
	{
		ControlTable *table = get_config_by_id(faction_table->ControlID, &all_control_config);
		if(table != NULL)
		{
			bool open = false;
			for (uint32_t i = 0; i < table->n_OpenDay; ++i)
			{
				if (time_helper::getWeek() == table->OpenDay[i])
				{
					open = true;
					break;
				}
			}

			if(open)
			{
				//开始帮会入侵活动
				for(size_t i = 0; i < table->n_OpenTime; i++)
				{
					tm.tm_hour = table->OpenTime[i] / 100;
					tm.tm_min = table->OpenTime[i] % 100;
					tm.tm_sec = 0;
					uint64_t st = mktime(&tm);
					if(st == mnow_time)
					{
						guild_ruqin_active_open();
					}
				}

				//关闭帮会入侵活动
				for(size_t i = 0; i < table->n_CloseTime; i++)
				{
					tm.tm_hour = table->CloseTime[i] / 100;
					tm.tm_min = table->CloseTime[i] % 100;
					tm.tm_sec = 0;
					uint64_t st = mktime(&tm);
					if(st == mnow_time)
					{
						guild_ruqin_active_stop();
					}
				}
			}
		}
	}

	if (sg_guild_bonfire_reward_interval == 0)
	{
		sg_guild_bonfire_reward_interval = 1;
	}

	//门宗篝火
	for (std::map<uint64_t, guild_land_raid_struct *>::iterator iter = guild_land_raid_manager_all_raid_id.begin(); iter != guild_land_raid_manager_all_raid_id.end(); ++iter)
	{
		guild_land_raid_struct *raid = iter->second;
		if (!(raid->GUILD_LAND_DATA.activity_id == GUILD_BONFIRE_ACTIVITY_ID && raid->GUILD_LAND_DATA.activity_state == GUILD_BONFIRE_STATE_BEGIN))
		{
			continue;
		}

		//检查角色是否在篝火旁边
		std::vector<GuildBonfireRewardInfo> rewards;
		position *bonfire_pos = raid->GUILD_LAND_DATA.activity_data.bonfire_data.bonfire->get_pos();
		for (std::map<uint64_t, player_struct *>::iterator iter_player = raid->m_players.begin(); iter_player != raid->m_players.end(); ++iter_player)
		{
			player_struct *player = iter_player->second;
			if (player->data->guild_bonfire_activity_time != raid->GUILD_LAND_DATA.activity_data.bonfire_data.begin_time)
			{
				player->data->guild_bonfire_last_ts = 0;
				player->data->guild_bonfire_activity_time = raid->GUILD_LAND_DATA.activity_data.bonfire_data.begin_time;
			}

			if (!check_circle_in_range(bonfire_pos, player->get_pos(), sg_guild_bonfire_radius))
			{
				player->data->guild_bonfire_last_ts = 0;
				continue;
			}

			if (player->data->guild_bonfire_last_ts == 0)
			{
				player->data->guild_bonfire_last_ts = mnow_time;
			}
			else if (player->data->guild_bonfire_reward_time < sg_guild_bonfire_player_reward_time && mnow_time > player->data->guild_bonfire_last_ts)
			{
				uint32_t elapse = mnow_time - player->data->guild_bonfire_last_ts;
				uint32_t valid = player->data->guild_bonfire_reward_time % sg_guild_bonfire_reward_interval;
				if (player->data->guild_bonfire_reward_time + elapse > sg_guild_bonfire_player_reward_time)
				{ //如果超出最大奖励时间，将奖励时间缩短
					elapse = (player->data->guild_bonfire_reward_time + elapse - sg_guild_bonfire_player_reward_time);
				}
				valid += elapse;
				player->data->guild_bonfire_reward_time += elapse;
				player->data->guild_bonfire_last_ts = mnow_time;
				if (valid >= sg_guild_bonfire_reward_interval)
				{
					uint32_t reward_count = valid / sg_guild_bonfire_reward_interval;
					float rate = player->get_buff_effect_rate(170000031) + 1.0;
					GuildBonfireRewardInfo info;
					memset(&info, 0, sizeof(GuildBonfireRewardInfo));
					info.player_id = player->get_uuid();
					info.exp = sg_guild_bonfire_reward[0] * reward_count * rate;
					info.coin = sg_guild_bonfire_reward[1] * reward_count * rate;
					info.donation = sg_guild_bonfire_reward[2] * reward_count * rate;
					info.treasure = sg_guild_bonfire_reward[3] * reward_count * rate;
					info.build_board = sg_guild_bonfire_reward[4] * reward_count * rate;

					if (info.exp > 0)
					{
						player->add_exp((double)info.exp * player->get_exp_rate(), MAGIC_TYPE_GUILD_BONFIRE_REWARD);
					}
					if (info.coin > 0)
					{
						player->add_coin((double)info.coin * player->get_coin_rate(), MAGIC_TYPE_GUILD_BONFIRE_REWARD);
					}
					rewards.push_back(info);

//					LOG_DEBUG("[%s:%d] player[%lu] bonfire, reward_time[%u], reward_count[%u]", __FUNCTION__, __LINE__, player->get_uuid(), player->data->guild_bonfire_reward_time, reward_count);
				}
			}
		}

		send_guild_bonfire_reward_to_guildsrv(raid->GUILD_LAND_DATA.guild_id, rewards);

		if (raid->GUILD_LAND_DATA.activity_data.bonfire_data.collect_refresh_time == 0)
		{
			raid->GUILD_LAND_DATA.activity_data.bonfire_data.collect_refresh_time = mnow_time;
		}
		else if (mnow_time - raid->GUILD_LAND_DATA.activity_data.bonfire_data.collect_refresh_time >= sg_guild_bonfire_refresh_collection_interval)
		{
			refresh_guild_bonfire_collection(raid);
			raid->GUILD_LAND_DATA.activity_data.bonfire_data.collect_refresh_time += sg_guild_bonfire_refresh_collection_interval;
		}

		//检查活动是否结束
		if ((uint32_t)mnow_time >= raid->GUILD_LAND_DATA.activity_data.bonfire_data.end_time)
		{
			guild_bonfire_close(raid);
		}
	}
}

void broadcast_guild_bonfire_notify(std::vector<uint64_t> &playerIds, uint32_t msg_id)
{
	if (playerIds.size() == 0)
	{
		return;
	}

	uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(msg_id, NULL, NULL);
	for (std::vector<uint64_t>::iterator iter = playerIds.begin(); iter != playerIds.end(); ++iter)
	{
		ppp = conn_node_gamesrv::broadcast_msg_add_players(*iter, ppp);
	}
	conn_node_gamesrv::broadcast_msg_send();
}

void broadcast_all_guild_player_bonfire_switch(uint32_t guild_id, uint32_t msg_id)
{
	std::vector<uint64_t> playerIds;
	for (std::map<uint64_t, player_struct *>::iterator iter = player_manager_all_players_id.begin(); iter != player_manager_all_players_id.end(); ++iter)
	{
		if (get_entity_type(iter->first) == ENTITY_TYPE_AI_PLAYER)
			continue;
		
		player_struct *player = iter->second;
		if (!player->is_online())
		{
			continue;
		}
		if (player->data->guild_id != guild_id)
		{
			continue;
		}
//		if (player->is_in_raid())
//		{
//			continue;
//		}
//		if (!player->is_alive())
//		{
//			continue;
//		}
		playerIds.push_back(iter->first);
	}

	broadcast_guild_bonfire_notify(playerIds, msg_id);
}

void send_bonfire_info_to_land_players(guild_land_raid_struct *raid)
{
	EXTERN_DATA ext_data;
	for (std::map<uint64_t, player_struct *>::iterator iter = player_manager_all_players_id.begin(); iter != player_manager_all_players_id.end(); ++iter)
	{
		if (get_entity_type(iter->first) == ENTITY_TYPE_AI_PLAYER)
			continue;
		
		player_struct *player = iter->second;
		if (!player->is_online())
		{
			continue;
		}
		if (player->data->guild_id != raid->GUILD_LAND_DATA.guild_id)
		{
			continue;
		}

		GuildBonfireInfoNotify nty;
		guild_bonfire_info_notify__init(&nty);
		position *bonfire_pos = raid->GUILD_LAND_DATA.activity_data.bonfire_data.bonfire->get_pos();
		nty.endtime = raid->GUILD_LAND_DATA.activity_data.bonfire_data.end_time;
		nty.rewardtime = player->data->guild_bonfire_reward_time;
		nty.posx = bonfire_pos->pos_x;
		nty.posz = bonfire_pos->pos_z;

		ext_data.player_id = player->get_uuid();
		fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_GUILD_BONFIRE_INFO_NOTIFY, guild_bonfire_info_notify__pack, nty);
	}
}

int guild_land_active_manager::guild_bonfire_open(uint32_t guild_id, bool bRepeat)
{
	guild_land_raid_struct *raid = guild_land_raid_manager::get_guild_land_raid(guild_id);
	if (!raid)
	{
		LOG_ERR("[%s,%d] guild[%u] get land raid failed", __FUNCTION__, __LINE__, guild_id);
		return -1;
	}

	uint32_t now = time_helper::get_cached_time() / 1000;
	if (bRepeat)
	{ //可以重复开
		if (raid->GUILD_LAND_DATA.activity_id != 0 && raid->GUILD_LAND_DATA.activity_id != GUILD_BONFIRE_ACTIVITY_ID)
		{
			LOG_ERR("[%s,%d] guild[%u] land raid having activity[%u]", __FUNCTION__, __LINE__, guild_id, raid->GUILD_LAND_DATA.activity_id);
			return -2;
		}
		if (raid->GUILD_LAND_DATA.activity_id == GUILD_BONFIRE_ACTIVITY_ID)
		{ //如果上一场没结束，关闭上一场
			guild_bonfire_close(raid);
		}
	}
	else
	{ //不可以重复开
		if (raid->GUILD_LAND_DATA.activity_id != 0)
		{
			LOG_ERR("[%s,%d] guild[%u] land raid having activity[%u]", __FUNCTION__, __LINE__, guild_id, raid->GUILD_LAND_DATA.activity_id);
			return -2;
		}
	}

	monster_struct *monster = monster_manager::create_monster_at_pos(raid, sg_guild_bonfire_id, 1, sg_guild_bonfire_pos_x, sg_guild_bonfire_pos_z, 0, NULL, 0);
	if (!monster)
	{
		LOG_ERR("[%s,%d] guild[%u] create bonfire failed, bonfire_id:%u, pos_x:%f, pos_z:%f", __FUNCTION__, __LINE__, guild_id, sg_guild_bonfire_id, sg_guild_bonfire_pos_x, sg_guild_bonfire_pos_z);
		return -3;
	}

	raid->GUILD_LAND_DATA.activity_id = GUILD_BONFIRE_ACTIVITY_ID;
	raid->GUILD_LAND_DATA.activity_state = GUILD_BONFIRE_STATE_BEGIN;
	raid->GUILD_LAND_DATA.activity_data.bonfire_data.bonfire = monster;
	raid->GUILD_LAND_DATA.activity_data.bonfire_data.begin_time = now;
	raid->GUILD_LAND_DATA.activity_data.bonfire_data.end_time = now + sg_guild_bonfire_time;

	broadcast_all_guild_player_bonfire_switch(guild_id, MSG_ID_GUILD_BONFIRE_OPEN_NOTIFY);
	send_bonfire_info_to_land_players(raid);
	
	LOG_INFO("[%s,%d] guild[%u] bonfire_id:%u, pos_x:%f, pos_z:%f", __FUNCTION__, __LINE__, guild_id, sg_guild_bonfire_id, sg_guild_bonfire_pos_x, sg_guild_bonfire_pos_z);

	return 0;
}

int guild_land_active_manager::guild_bonfire_close(guild_land_raid_struct *raid)
{
	LOG_INFO("[%s,%d] guild[%u] activity_id:%u", __FUNCTION__, __LINE__, raid->GUILD_LAND_DATA.guild_id, raid->GUILD_LAND_DATA.activity_id);

	broadcast_all_guild_player_bonfire_switch(raid->GUILD_LAND_DATA.guild_id, MSG_ID_GUILD_BONFIRE_CLOSE_NOTIFY);

	//清除所有活动产生的怪物和采集物
	raid->clear_monster();
	for (std::set<uint64_t>::iterator it = raid->m_collect.begin(); it != raid->m_collect.end(); )
	{
		std::set<uint64_t>::iterator next_it = it;
		next_it++;
		Collect *pCollect = Collect::GetById(*it);
		if (pCollect)
		{
			Collect::RemoveFromSceneAndDestroyCollect(pCollect, true);
		}
		it = next_it;
	}

	raid->GUILD_LAND_DATA.activity_id = 0;
	raid->GUILD_LAND_DATA.activity_state = 0;
	memset(&raid->GUILD_LAND_DATA.activity_data, 0, sizeof(raid->GUILD_LAND_DATA.activity_data));

	return 0;
}

