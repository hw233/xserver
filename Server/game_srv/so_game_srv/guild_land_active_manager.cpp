#include "guild_land_active_manager.h"



static const int GUILD_INTRUSION_CONTROLTABLE_ID = 440500001; //帮会入侵 FactionActivity表id

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
			//初始化数据
			itr->second->init_guild_ruqin_active_data();
			itr->second->ruqin_data.status = GUILD_RUQIN_ACTIVE_START;
			//帮会入侵活动开启
			itr->second->ruqin_data.guild_ruqin = true;
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

}
