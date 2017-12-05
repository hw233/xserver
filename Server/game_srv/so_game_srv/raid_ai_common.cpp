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
#include "server_level.h"

extern void set_leixinye_type(monster_struct *monster, uint32_t type);
static player_struct *get_script_raid_event_player(raid_struct *raid)
{
	do
	{
		if (raid->m_config->DengeonType == 1)
		{
			if (!raid->m_raid_team)
			{
				break;
			}
			player_struct *player = raid->m_raid_team->GetLead();
			if (!player)
			{
				break;
			}

			int pos = -1;
			raid->get_raid_player_info(player->get_uuid(), &pos);
			if (pos < 0)
			{
				break;
			}

			return player;
		}
		else if (raid->m_config->DengeonType == 2)
		{
			return raid->m_player[0];
		}
	} while(0);

	return NULL;
}

static bool jump_to_another_script(raid_struct *raid, char *script_name, struct raid_script_data *script_data)
{
	std::vector<struct RaidScriptTable *> *configs = get_config_by_name(script_name, &all_raid_script_config);
	if (!configs)
	{
		return false;
	}

	for (int i = 0; i < MAX_SCRIPT_COND_NUM; ++i)
		script_data->cur_finished_num[i] = 0;
	script_data->cur_index = 0xffff;
	script_data->script_config = configs;

	return true;
}

static bool do_check_script_raid_monster_hp(double percent, monster_struct* monster);
static bool script_raid_check_finished(raid_struct *raid, struct raid_script_data *script_data)
{
	assert(script_data->cur_index < script_data->script_config->size());
	struct RaidScriptTable *config = (*script_data->script_config)[script_data->cur_index];
	switch (config->TypeID)
	{
		case SCRIPT_EVENT_STOP_MONSTER_AI:
		case SCRIPT_EVENT_TIME_OUT: //副本计时
			if (time_helper::get_cached_time() / 1000 >= script_data->cur_finished_num[0])
				return true;
			return false;
//		case SCRIPT_EVENT_MONSTER_DEAD_NUM: //指定怪物死亡
//			return false;
		case SCRIPT_EVENT_MONSTER_DEAD_ALL: //所有指定怪物死亡 等同于检测副本内还存活指定怪物数量小于某值
		{
			for (size_t i = 0; i < config->n_Parameter1; i++)
			{
				if (raid->is_monster_alive(config->Parameter1[i]))
					return false;
			}
			return true;
		}
//		case SCRIPT_EVENT_COLLECT_NUM: //采集指定采集物
//			return false;
		case SCRIPT_EVENT_ALIVE_MONSTER_EQUEL: //检测副本内还存活指定怪物数量等于某值
		{
			for (size_t i = 0; i + 1 < config->n_Parameter1; i = i+2)
			{
				uint32_t num = raid->get_id_monster_num(config->Parameter1[i]);
				if (num != config->Parameter1[i + 1])
					return false;
			}
			return true;
		}
		case SCRIPT_EVENT_ALIVE_MONSTER_LESSER: //检测副本内还存活指定怪物数量小于某值
		{
			for (size_t i = 0; i + 1 < config->n_Parameter1; i = i+2)
			{
				uint32_t num = raid->get_id_monster_num(config->Parameter1[i]);
				if (num >= config->Parameter1[i + 1])
					return false;
			}
			return true;
		}
//		case SCRIPT_EVENT_ALIVE_MONSTER_GREATER: //检测副本内还存活指定怪物数量大于某值  这个有啥用？
//			return false;
		case SCRIPT_EVENT_ALIVE_COLLECT_EQUEL: //检测副本内还存活指定采集物数量等于某值
			for (size_t i = 0; i + 1 < config->n_Parameter1; i = i+2)
			{
				uint32_t num = raid->get_id_collect_num(config->Parameter1[i]);
				if (num != config->Parameter1[i + 1])
					return false;
			}
			return true;
		case SCRIPT_EVENT_ACCEPT_TASK:
		{
			do
			{
				player_struct *player = get_script_raid_event_player(raid);
				if (!player)
				{
					break;
				}

				for (size_t i = 0; i < config->n_Parameter1; ++i)
				{
					if (player->get_task_info(config->Parameter1[i]) == NULL)
					{
						return false;
					}
				}
				return true;
			} while(0);
		}
		return false;
		case SCRIPT_EVENT_MONSTER_ARRIVE_POSITION:
		{
			do
			{
				if (config->n_Parameter1 < 4)
				{
					break;
				}

				uint32_t monster_id = config->Parameter1[0];
				position target_pos;
				target_pos.pos_x = config->Parameter1[1];
				target_pos.pos_z = config->Parameter1[2];

				for (std::set<monster_struct*>::iterator iter = raid->m_monster.begin(); iter != raid->m_monster.end(); ++iter)
				{
					monster_struct *monster = *iter;
					if (monster->data->monster_id != monster_id)
					{
						continue;
					}
						
					position *cur_pos = monster->get_pos();
					if (check_circle_in_range(cur_pos, &target_pos, config->Parameter1[3]))
					{
						return true;
					}
				}
			} while(0);
		}
		return false;
		case SCRIPT_EVENT_PLAYER_ARRIVE_POSITION:
		{
			do
			{
				player_struct *player = get_script_raid_event_player(raid);
				if (!player)
				{
					break;
				}

				if (config->n_Parameter1 < 3)
				{
					break;
				}

				position *cur_pos = player->get_pos();
				position target_pos;
				target_pos.pos_x = config->Parameter1[0];
				target_pos.pos_z = config->Parameter1[1];
				if (check_circle_in_range(cur_pos, &target_pos, config->Parameter1[2]))
				{
					return true;
				}
			} while(0);
		}
		return false;
		case SCRIPT_EVENT_PLAYER_TASK_FORK:
		{
			do
			{
				player_struct *player = get_script_raid_event_player(raid);
				if (!player)
				{
					break;
				}

				for (size_t i = 0; i + 1 < config->n_Parameter1; i = i+2)
				{
					if (player->get_task_info(config->Parameter1[i]) != NULL)
					{
						uint32_t val = config->Parameter1[i + 1];
						if (val > 0)
						{
							if (config->n_Parameter2 < val)
							{
								return false;
							}

							return jump_to_another_script(raid, config->Parameter2[val - 1], script_data);
						}

						return true;
					}
				}
			} while(0);
		}
		return false;
		case SCRIPT_EVENT_MONSTER_DEAD_NUM:
		{
			if (script_data->dead_monster_id == 0)
				return false;
			bool flag = true;			
			for (size_t i = 0; i + 1 < config->n_Parameter1; i = i+2)
			{
				assert(i + 1 < config->n_Parameter1);
				assert(i / 2 < MAX_SCRIPT_COND_NUM);
		
				if (config->Parameter1[i] == script_data->dead_monster_id)
					++script_data->cur_finished_num[i / 2];
				if (script_data->cur_finished_num[i / 2] < config->Parameter1[i + 1])
					flag = false;
			}
			return flag;
		}
		case SCRIPT_EVENT_WAIT_MONST_HP:
		{
			do
			{
				if(config->n_Parameter1 < 2)
				{	
					break;
				}	
				bool flag = true;
				for(size_t i = 0; i+1 < config->n_Parameter1; i = i+2)
				{
					uint32_t monster_id = config->Parameter1[i];
					uint32_t percent_hp = config->Parameter1[i+1];

					if (monster_id == script_data->dead_monster_id)
						continue;
						
					for(std::set<monster_struct*>::iterator target_monster = raid->m_monster.begin(); target_monster != raid->m_monster.end(); target_monster++)
					{
						monster_struct* cause_monster = *target_monster;
						if(monster_id != cause_monster->data->monster_id)
						{
							continue;
						}

						if(!do_check_script_raid_monster_hp(percent_hp,cause_monster))
						{
							flag = false;
						}
					}
				}
				return flag;
					
			}while(0);
		}
		return false;
		case SCRIPT_EVENT_WAIT_NPC_TALK:		//等待玩家主动点击npc对话完毕
		case SCRIPT_EVENT_AUTOMATIC_NPC_TALK:  //发送自动npc对话，并且等待对话完毕
		{
			if(raid != NULL && raid->data != NULL)
			{
				if(raid->data->raid_ai_event == SCRIPT_EVENT_AUTOMATIC_NPC_TALK || raid->data->raid_ai_event == SCRIPT_EVENT_WAIT_NPC_TALK)
				{
					struct DungeonTable *t_config = raid->get_raid_config();
					if (t_config)
					{
						if (t_config
							&& raid->data->pass_index < t_config->n_PassType
							&& t_config->PassType[raid->data->pass_index] == 6
							&& config->n_Parameter1 > 0
							&& t_config->PassValue[raid->data->pass_index] == config->Parameter1[0])
						{
							raid->add_raid_pass_value(6, t_config);
						}
					}
					
					raid->data->raid_ai_event = 0;
					return true;
				}
			}
		}
		return false;
		case SCRIPT_EVENT_CREATE_WAIT_MONSTER_LEVEL:
		{
			if(raid->ruqin_data.level != 0)
				return true;
		}
		return false;
		case SCRIPT_EVENT_MONSTER_DEAD_DELETE_NPC:
		{
			bool flag = true;			
			for (size_t i = 0; i + 1 < config->n_Parameter1; i = i+2)
			{
				assert(i + 1 < config->n_Parameter1);
				assert(i / 2 < MAX_SCRIPT_COND_NUM);
				int m = 0;
				for(size_t j = 0; j < MAX_SCRIPT_COND_NUM; j++)
				{
					if(script_data->cur_finished_num[j] == config->Parameter1[i+1])
					{
						m = 1;
					}
				
				}
				if(m == 0)
				{
					flag = false;	
					break;
				}
			}
			return flag;
		
		}
		default:
			return false;
	}
	return false;
}

static void script_event_raid_stop(raid_struct *raid, struct raid_script_data *script_data)
{
	raid->clear_monster();
	if (!raid->m_player[0])
	{
		LOG_ERR("%s: raid[%p][%lu] can not find m_player[0] player id [%lu]",
			__FUNCTION__, raid, raid->data->uuid, raid->data->player_info[0].player_id);
		return;
	}

	raid->player_leave_raid(raid->m_player[0]);
}

static bool script_raid_init_cur_cond(raid_struct *raid, struct raid_script_data *script_data)
{
	assert(script_data->cur_index < script_data->script_config->size());
	struct RaidScriptTable *config = (*script_data->script_config)[script_data->cur_index];
	LOG_INFO("%s: raid[%u][%lu] config id[%lu] index[%u]", __FUNCTION__, raid->data->ID, raid->data->uuid, config->TypeID, script_data->cur_index);
	switch (config->TypeID)
	{
		case SCRIPT_EVENT_CREATE_MONSTER_NUM: //刷新配置表内指定怪物
			for (size_t i = 0; i + 1 < config->n_Parameter1; i = i+2)
				monster_manager::create_monster_by_id(raid, config->Parameter1[i], config->Parameter1[i + 1], raid->lv);
			return true;
		case SCRIPT_EVENT_CREATE_MONSTER_ALL: //刷新配置表内所有指定怪物
			for (size_t i = 0; i < config->n_Parameter1; ++i)
				monster_manager::create_monster_by_id(raid, config->Parameter1[i], 9999, raid->lv);
			return true;
		case SCRIPT_EVENT_CREATE_COLLECT_NUM: //刷新配置表内指定采集物
			for (size_t i = 0; i + 1 < config->n_Parameter1; i = i+2)
				Collect::CreateCollectByID(raid, config->Parameter1[i], config->Parameter1[i + 1]);
			return true;
		case SCRIPT_EVENT_CREATE_COLLECT_ALL: //刷新配置表内所有指定采集物
			for (size_t i = 0; i < config->n_Parameter1; ++i)
				Collect::CreateCollectByID(raid, config->Parameter1[i], 9999);
			return true;
		case SCRIPT_EVENT_TRANSFER_PLAYER_POSITION:
			{
				if (config->n_Parameter1 < 4)
				{
					return false;
				}
				double pos_x = config->Parameter1[0];
//				double pos_y = config->Parameter1[1];
				double pos_z = config->Parameter1[2];
				double direct = config->Parameter1[3];
				for (int i = 0; i < MAX_TEAM_MEM; ++i)
				{
					player_struct *player = raid->m_player[i];
					if (!player)
					{
						continue;
					}
					player->cur_scene_jump(pos_x, pos_z, direct, NULL);
				}
			}
			return true;
		case SCRIPT_EVENT_FINISH_RAID:
			{
				raid->on_raid_finished();
			}
			return true;
		case SCRIPT_EVENT_FAIL_RAID:
		{
			raid->on_raid_failed(0);
		}
		return true;
		case SCRIPT_EVENT_MONSTER_13_TYPE:
			{
				if (config->n_Parameter1 < 1)
				{
					return false;
				}

				monster_struct *target_monster = NULL;
				for (std::set<monster_struct*>::iterator iter = raid->m_monster.begin(); iter != raid->m_monster.end(); ++iter)
				{
					monster_struct *monster = *iter;
					if (monster->ai_type != 13)
					{
						continue;
					}

					target_monster = monster;
					break;
				}

				if (!target_monster)
				{
					return false;
				}

				set_leixinye_type(target_monster, config->Parameter1[0]);
			}
			return true;
		case SCRIPT_EVENT_START_ESCORT:
			{
				if (config->n_Parameter1 < 1)
				{
					return false;
				}
				player_struct *player = get_script_raid_event_player(raid);
				if (!player)
				{
					return false;
				}

				player->start_escort(config->Parameter1[0]);
			}
			return true;
		case SCRIPT_EVENT_START_MONSTER_AI:
			{
				raid->start_monster_ai();
			}
			return true;
		case SCRIPT_EVENT_STOP_MONSTER_AI:
			{
				raid->stop_monster_ai();
				assert(config->n_Parameter1 >= 1);
				script_data->cur_finished_num[0] = time_helper::get_cached_time() / 1000 + config->Parameter1[0];
			}
			return false;
		case SCRIPT_EVENT_PLAY_ANIMATION:
		{
			player_struct *player = get_script_raid_event_player(raid);
			if (player)
			{
				player->stop_move();
			}
			RaidEventNotify nty;
			raid_event_notify__init(&nty);
			nty.type = config->TypeID;
			nty.param1 = config->Parameter1;
			nty.n_param1 = config->n_Parameter1;
			nty.param2 = config->Parameter2;
			nty.n_param2 = config->n_Parameter2;
			raid->broadcast_to_raid(MSG_ID_RAID_EVENT_NOTIFY, &nty, (pack_func)raid_event_notify__pack);
			return true;
		}
		case SCRIPT_EVENT_CREATE_NPC_NUM: //刷新配置表内指定NPC
		case SCRIPT_EVENT_CREATE_NPC_ALL: //刷新配置表内所有指定NPC
		case SCRIPT_EVENT_CREATE_TRANSFER_NUM: //刷新配置表内指定传送点
		case SCRIPT_EVENT_CREATE_TRANSFER_ALL: //刷新配置表内所有指定传送点
		case SCRIPT_EVENT_CREATE_AIR_WALL: //产生空气墙
		case SCRIPT_EVENT_REMOVE_AIR_WALL: //删除空气墙
		case SCRIPT_EVENT_REMOVE_NPC:
		case SCRIPT_EVENT_PLAY_NPC_ACTION:
		case SCRIPT_EVENT_START_GONGCHENGCHUI:
		case SCRIPT_EVENT_PLAY_EFFECT:
		case SCRIPT_EVENT_POPUP_TALK:
		case SCRIPT_EVENT_PLAYER_DUMIAO_CARTOON:
		{
			RaidEventNotify nty;
			raid_event_notify__init(&nty);
			nty.type = config->TypeID;
			nty.param1 = config->Parameter1;
			nty.n_param1 = config->n_Parameter1;
			nty.param2 = config->Parameter2;
			nty.n_param2 = config->n_Parameter2;
			raid->broadcast_to_raid(MSG_ID_RAID_EVENT_NOTIFY, &nty, (pack_func)raid_event_notify__pack);
			return true;
		}
		case SCRIPT_EVENT_RAID_STOP:
		{
			script_event_raid_stop(raid, script_data);
			return false;
		}
		case SCRIPT_EVENT_SET_PK_MODE:
		{
			assert(config->n_Parameter1 >= 1);
			player_struct *player = get_script_raid_event_player(raid);
			if (player)
			{
				player->set_attr(PLAYER_ATTR_PK_TYPE, config->Parameter1[0]);
				player->broadcast_one_attr_changed(PLAYER_ATTR_PK_TYPE, config->Parameter1[0], false, true);
			}
			return true;
		}
		case SCRIPT_EVENT_SET_CAMP:
		{
			assert(config->n_Parameter1 >= 1);			
			player_struct *player = get_script_raid_event_player(raid);
			if (player)
			{
				player->set_attr(PLAYER_ATTR_ZHENYING, config->Parameter1[0]);
				player->broadcast_one_attr_changed(PLAYER_ATTR_ZHENYING, config->Parameter1[0], false, true);
				if (config->Parameter1[0] != 0)
				{
					player->m_skill.OpenAllSkill();
					player->m_skill.SendAllSkill();
				}
				player->send_zhenying_info();
			}
			return true;
		}
		case SCRIPT_EVENT_ADD_BUFF:
		{
			assert(config->n_Parameter1 >= 1);			
			player_struct *player = get_script_raid_event_player(raid);
			if (player)
			{
				buff_manager::create_default_buff(config->Parameter1[0], player, player, true);
			}
			return true; 
		}
		case SCRIPT_EVENT_DEL_BUFF:
		{
			assert(config->n_Parameter1 >= 1);			
			player_struct *player = get_script_raid_event_player(raid);
			if (player)
			{
				player->delete_one_buff(config->Parameter1[0], true);								
			}
			return true; 
		}		
		case SCRIPT_EVENT_SET_NOVICERAID_FLAG:
		{
			assert(config->n_Parameter1 >= 1);			
			player_struct *player = get_script_raid_event_player(raid);
			if (player)
			{
				player->data->noviceraid_flag = config->Parameter1[0];

				player->m_skill.clear(); //重新初始化技能
				player->m_skill.SendAllSkill();
				player->m_skill.Init();
				player->m_skill.SendAllSkill();
			}
			return true;
		}
		case SCRIPT_EVENT_WAIT_NPC_TALK:		//等待玩家主动点击npc对话完毕
		case SCRIPT_EVENT_AUTOMATIC_NPC_TALK:  //发送自动npc对话，并且等待对话完毕
		{
			RaidEventNotify nty;
			raid_event_notify__init(&nty);
			nty.type = config->TypeID;
			nty.param1 = config->Parameter1;
			nty.n_param1 = config->n_Parameter1;
			nty.param2 = config->Parameter2;
			nty.n_param2 = config->n_Parameter2;
			raid->broadcast_to_raid(MSG_ID_RAID_EVENT_NOTIFY, &nty, (pack_func)raid_event_notify__pack);
		}
		case SCRIPT_EVENT_MONSTER_DEAD_ALL: //所有指定怪物死亡 等同于检测副本内还存活指定怪物数量小于某值
		case SCRIPT_EVENT_ALIVE_MONSTER_EQUEL: //检测副本内还存活指定怪物数量等于某值
		case SCRIPT_EVENT_ALIVE_MONSTER_LESSER: //检测副本内还存活指定怪物数量小于某值
		case SCRIPT_EVENT_ALIVE_MONSTER_GREATER: //检测副本内还存活指定怪物数量大于某值  这个有啥用？
		case SCRIPT_EVENT_ALIVE_COLLECT_EQUEL: //检测副本内还存活指定采集物数量等于某值
		case SCRIPT_EVENT_ACCEPT_TASK:
		case SCRIPT_EVENT_MONSTER_ARRIVE_POSITION:
		case SCRIPT_EVENT_PLAYER_ARRIVE_POSITION:
		case SCRIPT_EVENT_MONSTER_DEAD_NUM: //指定怪物死亡
		case SCRIPT_EVENT_WAIT_MONST_HP:
		case SCRIPT_EVENT_CREATE_WAIT_MONSTER_LEVEL:
		case SCRIPT_EVENT_MONSTER_DEAD_DELETE_NPC:
			return script_raid_check_finished(raid, script_data);
		case SCRIPT_EVENT_TIME_OUT: //副本计时
			assert(config->n_Parameter1 >= 1);
			script_data->cur_finished_num[0] = time_helper::get_cached_time() / 1000 + config->Parameter1[0];
		case SCRIPT_EVENT_COLLECT_NUM: //采集指定采集物
		case SCRIPT_EVENT_PLAYER_TASK_FORK:
		case SCRIPT_EVENT_ESCORT_RESULT:
			return false;
		case SCRIPT_EVENT_COLLECT_CALLBACK:
		{
			script_data->collect_callback_event = config->Parameter1[0];
			return true;
		}
		case SCRIPT_EVENT_ADD_BUFF_TO_MONSTER:
		{
			assert(config->n_Parameter1 >=1 && config->n_Parameter1%2 == 0);
			for(size_t i = 0; i < config->n_Parameter1; i = i+2)	
			{
				uint32_t monster_id = config->Parameter1[i];
				uint32_t buff_id = config->Parameter1[i+1];
				for(std::set<monster_struct*>::iterator itr = raid->m_monster.begin(); itr != raid->m_monster.end(); itr++)
				{
					monster_struct* monster = *itr;
					if(monster_id != monster->data->monster_id)
					{
						continue;
					}

					buff_manager::create_default_buff(buff_id, monster, monster, true);
				}
			}

		return true;
		}
		case SCRIPT_EVENT_DEL_BUFF_FROM_MONSTER:
		{
			assert(config->n_Parameter1 >=1 && config->n_Parameter1%2 == 0);
			for(size_t i = 0; i < config->n_Parameter1; i = i+2)	
			{
				uint32_t monster_id = config->Parameter1[i];
				uint32_t buff_id = config->Parameter1[i+1];
				for(std::set<monster_struct*>::iterator itr = raid->m_monster.begin(); itr != raid->m_monster.end(); itr++)
				{
					monster_struct* monster = *itr;
					if(monster_id != monster->data->monster_id)
					{
						continue;
					}

					monster->delete_one_buff(buff_id, true);
				}
			}
		}
			return true;
		case SCRIPT_EVENT_DELETE_MONSTER:
		{
			assert(config->n_Parameter1 >=1);
			for(size_t j = 0; j < config->n_Parameter1; j++)
			{
				uint32_t monster_id = config->Parameter1[j];
				for(std::set<monster_struct*>::iterator itr = raid->m_monster.begin(); itr != raid->m_monster.end();)
				{
					std::set<monster_struct *>::iterator next_itr = itr;
					++next_itr;
					if ((*itr)->data->monster_id == monster_id)
					{
						monster_struct *m = *itr;
						raid->delete_monster_from_scene(m, true);
						monster_manager::delete_monster(m);
					}
					itr = next_itr;
				}

			}
		return true;
		}
		case SCRIPT_EVENT_SET_PLAYER_ATTR:
		{
			assert(config->n_Parameter1 >= 2 && config->n_Parameter1 % 2 == 0);			
			player_struct *player = get_script_raid_event_player(raid);
			if (player)
			{

				for(size_t i = 0; i + 1 < config->n_Parameter1; i = i + 2)
				{
					uint32_t id = config->Parameter1[i];
					uint32_t val = player->data->attrData[id] + config->Parameter1[i+1];
					assert(id >= PLAYER_ATTR_MAXHP && id < PLAYER_ATTR_MAX);			
					if (id < MAX_BUFF_FIGHT_ATTR)
					{
						player->data->buff_fight_attr[id] = val;
					}
					player->set_attr(id, val);
					player->broadcast_one_attr_changed(id, val, false, true);
					if(id == PLAYER_ATTR_MAXHP)
					{
						player->set_attr(PLAYER_ATTR_HP, player->data->attrData[PLAYER_ATTR_MAXHP]);
						player->broadcast_one_attr_changed(PLAYER_ATTR_HP, player->data->attrData[PLAYER_ATTR_HP], false, true);
					}
				}

			}
			return true;
		}
		case  SCRIPT_EVENT_RECOVER_PLAYER_ATTR:
		{
			player_struct *player = get_script_raid_event_player(raid);
			if (player)
			{
				player->calculate_attribute(true);
				player->set_attr(PLAYER_ATTR_HP, player->data->attrData[PLAYER_ATTR_MAXHP]);
				player->data->buff_fight_attr[PLAYER_ATTR_HP] = player->data->attrData[PLAYER_ATTR_HP];
				player->broadcast_one_attr_changed(PLAYER_ATTR_HP, player->data->attrData[PLAYER_ATTR_HP], false, true);

			}
			return true;
		
		}
		case SCRIPT_EVENT_SET_REGION_BUFF:
		{
			assert(config->n_Parameter1 >= 3);
			script_data->region_config[script_data->cur_region_config++] = config;
			raid->check_all_monster_region_buff(config);
			return true;
		}
		case SCRIPT_EVENT_STOP_REGION_BUFF:
		{
			for (size_t i = 0; i < script_data->cur_region_config; ++i)
			{
				if (script_data->region_config[i]->Parameter1[0] == config->Parameter1[0])
				{
					script_data->region_config[i] = script_data->region_config[--script_data->cur_region_config];
					break;
				}
			}
			return true;
		}
		case SCRIPT_EVENT_ADD_RAID_PASS_VALUE:
		{
			if (config->n_Parameter1 >=1)
			{
				int tid = (int)config->Parameter1[0];
				if ((int)raid->data->pass_index != tid - 1)
					return true;
			}
			
			raid->add_raid_pass_value(5, raid->get_raid_config());
			return true;
		}
		case SCRIPT_EVENT_REPLACE_MONSTER_CONFIG:
		{
			assert(config->n_Parameter2 == 1);
			std::vector<struct SceneCreateMonsterTable *> *monster_config = get_config_by_name(config->Parameter2[0], &all_raid_ai_monster_config);
			if(monster_config != NULL)
			{
				raid->create_monster_config = monster_config;	
			}
		}
			return true;
		case SCRIPT_EVENT_CREATE_MONSTER_LEVEL_NUM: //刷新配置表内指定怪物(等级使用服务器等级)
		{
			uint64_t monster_level = 0;
			if(raid->ruqin_data.guild_ruqin == true)
			{
				monster_level =  raid->ruqin_data.level;
				raid->ruqin_data.monster_boshu += 1;
			}
			else 
			{
				monster_level = raid->lv;
			}

			for (size_t i = 0; i + 1 < config->n_Parameter1; i = i+2)
			{
				monster_manager::create_monster_by_id(raid, config->Parameter1[i], config->Parameter1[i + 1], monster_level);
			}
		}
			return true;
		case SCRIPT_EVENT_CREATE_MONSTER_LEVEL_ALL: //刷新配置表内指定所有怪物(等级使用服务器等级)
		{
			uint64_t monster_level = 0;
			if(raid->ruqin_data.guild_ruqin == true)
			{
				monster_level =  raid->ruqin_data.level;
				raid->ruqin_data.monster_boshu += 1;
			}
			else 
			{
				monster_level = raid->lv;
			}

			for (size_t i = 0; i < config->n_Parameter1; ++i)
			{
				monster_manager::create_monster_by_id(raid, config->Parameter1[i], 9999, monster_level);
			}
		}
			return true;
		case SCRIPT_EVENT_GUILD_RUQIN_MONSTER_BOSHU:
		{
			assert(config->n_Parameter1 == 1);
			if(raid->ruqin_data.guild_ruqin == true)
			{
				raid->ruqin_data.all_boshu = config->Parameter1[0];
			}
		}
			return true;
		case SCRIPT_EVENT_GUILD_RUQIN_FINISH_FLAG:
			if(raid->ruqin_data.guild_ruqin == true)
			{
				raid->ruqin_data.status = GUILD_RUQIN_ACTIVE_FINISH;
			}
			return true;
		case SCRIPT_EVENT_SUIJI_CREATE_MONSTER:
		{
			assert(config->n_Parameter1 > 1 && config->n_Parameter2 >= 3);
			uint32_t i = rand()%config->n_Parameter1;
			uint32_t monster_id = config->Parameter1[i];
			
			uint32_t pos_x = atoi(config->Parameter2[0]);
			uint32_t pos_z = atoi(config->Parameter2[1]);
			float direct = atoi(config->Parameter2[2]);
			monster_manager::create_monster_at_pos(raid, monster_id, raid->lv, pos_x, pos_z, 0, NULL, direct);
		}
			return true;
		default:
			return false;
	}
	return false;
}

static void script_raid_next(raid_struct *raid, struct raid_script_data *script_data);
void do_script_raid_init_cond(raid_struct *raid, struct raid_script_data *script_data)
{
	LOG_INFO("%s: raid[%p][%lu][%lu] cur_index = %d", __FUNCTION__, raid, raid->m_config->DungeonID, raid->data->uuid, script_data->cur_index);
	if (script_data->cur_index >= script_data->script_config->size())
	{
			// TODO: 副本完成
		return;
	}

	for (int i = 0; i < MAX_SCRIPT_COND_NUM; ++i)
		script_data->cur_finished_num[i] = 0;

	if (script_raid_init_cur_cond(raid, script_data))
		script_raid_next(raid, script_data);
}

static void script_raid_next(raid_struct *raid, struct raid_script_data *script_data)
{
	script_data->cur_index++;
	do_script_raid_init_cond(raid, script_data);
}

/*void script_ai_common_npc_talk(raid_struct *raid, uint32_t npc_id, struct raid_script_data *script_data)
{
	LOG_DEBUG("%s: npc id[%u] raid[%p][%lu]", __FUNCTION__, npc_id, raid, raid->data->uuid);
	struct RaidScriptTable *config = (*script_data->script_config)[script_data->cur_index];
	if (config->TypeID != SCRIPT_EVENT_WAIT_NPC_TALK)
		return;
	if (config->n_Parameter1 != 1)
	{
		script_raid_next(raid, script_data);		
		return;
	}
	if (npc_id != config->Parameter1[0])
		return;
	script_raid_next(raid, script_data);
	return;
}*/

void script_ai_common_monster_dead(raid_struct *raid, monster_struct *monster, unit_struct *killer, struct raid_script_data *script_data)
{
	LOG_DEBUG("%s: monster id[%u] raid[%p][%lu]", __FUNCTION__, monster->data->monster_id, raid, raid->data->uuid);
	bool pass = true;
	if (script_data->cur_index >= script_data->script_config->size())
		return;
	
	struct RaidScriptTable *config = (*script_data->script_config)[script_data->cur_index];

	if (config->TypeID == SCRIPT_EVENT_WAIT_MONST_HP)
	{
		uint32_t old_dead_monster_id = script_data->dead_monster_id;
		script_data->dead_monster_id = monster->data->monster_id;
		if (script_raid_check_finished(raid, script_data))
			script_raid_next(raid, script_data);			
		script_data->dead_monster_id = old_dead_monster_id;
		return;
	}
	
	if(config->TypeID == SCRIPT_EVENT_MONSTER_DEAD_DELETE_NPC)
	{	
		for(size_t i = 0; i + 1 < config->n_Parameter1; i = i+2)
		{
			assert(i + 1 < config->n_Parameter1);
			assert(i / 2 < MAX_SCRIPT_COND_NUM);
			if(monster->data->monster_id == config->Parameter1[i] && script_data->cur_finished_num[i / 2] == 0)
			{
				double npc_id[1] = {0};
				npc_id[0] = config->Parameter1[i+1];
				RaidEventNotify nty;
				raid_event_notify__init(&nty);
				nty.type = config->TypeID;
				nty.param1 = npc_id;
				nty.n_param1 = 1;
				nty.param2 = config->Parameter2;
				nty.n_param2 = config->n_Parameter2;
				raid->broadcast_to_raid(MSG_ID_RAID_EVENT_NOTIFY, &nty, (pack_func)raid_event_notify__pack);
				script_data->cur_finished_num[i / 2] = config->Parameter1[i+1];
				break;		
			}
		}
		return;
	}
	if (config->TypeID != SCRIPT_EVENT_MONSTER_DEAD_NUM)
		return;
	for (size_t i = 0; i + 1 < config->n_Parameter1; i = i+2)
	{
		assert(i + 1 < config->n_Parameter1);
		assert(i / 2 < MAX_SCRIPT_COND_NUM);
		
		if (config->Parameter1[i] == monster->data->monster_id)
			++script_data->cur_finished_num[i / 2];
		if (script_data->cur_finished_num[i / 2] < config->Parameter1[i + 1])
			pass = false;
	}

	if (pass)
		script_raid_next(raid, script_data);
}

extern void stop_leiminggu_skill(monster_struct *monster);
void script_ai_common_collect(raid_struct *raid, player_struct *player, Collect *collect, struct raid_script_data *script_data)
{
	if (script_data->cur_index >= script_data->script_config->size())
		return;
	
	switch (script_data->collect_callback_event)
	{
		case 1://打断雷鸣鼓
		{
			for (std::set<monster_struct *>::iterator ite = raid->m_monster.begin(); ite != raid->m_monster.end(); ++ite)
			{
				if ((*ite)->ai_type == 13)
				{
					stop_leiminggu_skill(*ite);					
					break;
				}
			}
			break;
		}
		default:
			break;
	}
	
	bool pass = true;
	struct RaidScriptTable *config = (*script_data->script_config)[script_data->cur_index];
	if (config->TypeID != SCRIPT_EVENT_COLLECT_NUM)
		return;
	for (size_t i = 0; i < config->n_Parameter1; i = i + 2)
	{
		assert(i + 1 < config->n_Parameter1);
		assert(i / 2 < MAX_SCRIPT_COND_NUM);
		if (config->Parameter1[i] == collect->m_collectId)
			++script_data->cur_finished_num[i / 2];
		if (script_data->cur_finished_num[i / 2] < config->Parameter1[i + 1])
			pass = false;
	}

	if (pass)
		script_raid_next(raid, script_data);
}

void script_ai_common_tick(raid_struct *raid, struct raid_script_data *script_data)
{
	if (script_data->cur_index >= script_data->script_config->size())
	{
			// TODO: 副本完成
		return;
	}
	
	if (script_raid_check_finished(raid, script_data))
		script_raid_next(raid, script_data);

	if (raid->m_config->DengeonRank == DUNGEON_TYPE_SCRIPT)
		normal_raid_ai_tick(raid);
}

void script_ai_common_player_ready(raid_struct *raid, player_struct *player, struct raid_script_data *script_data)
{
	for (int i = 0; raid->m_config->DengeonType != 2 && i <= script_data->cur_index; ++i)
	{
		struct RaidScriptTable *config = (*script_data->script_config)[i];
		if (!config)
		{
			continue;
		}

		switch (config->TypeID)
		{
			case SCRIPT_EVENT_PLAY_ANIMATION:
			{
				player->stop_move();				
				RaidEventNotify nty;
				raid_event_notify__init(&nty);
				nty.type = config->TypeID;
				nty.param1 = config->Parameter1;
				nty.n_param1 = config->n_Parameter1;
				nty.param2 = config->Parameter2;
				nty.n_param2 = config->n_Parameter2;

				EXTERN_DATA ext_data;
				ext_data.player_id = player->get_uuid();

				fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_RAID_EVENT_NOTIFY, raid_event_notify__pack, nty);
				break;
			}
			case SCRIPT_EVENT_CREATE_NPC_NUM: //刷新配置表内指定NPC
			case SCRIPT_EVENT_CREATE_NPC_ALL: //刷新配置表内所有指定NPC
			case SCRIPT_EVENT_CREATE_TRANSFER_NUM: //刷新配置表内指定传送点
			case SCRIPT_EVENT_CREATE_TRANSFER_ALL: //刷新配置表内所有指定传送点
			case SCRIPT_EVENT_CREATE_AIR_WALL: //产生空气墙
			case SCRIPT_EVENT_REMOVE_AIR_WALL: //删除空气墙
			case SCRIPT_EVENT_REMOVE_NPC:
			case SCRIPT_EVENT_PLAY_NPC_ACTION:
			case SCRIPT_EVENT_AUTOMATIC_NPC_TALK:
			case SCRIPT_EVENT_PLAY_EFFECT:
			case SCRIPT_EVENT_PLAYER_DUMIAO_CARTOON:
			case SCRIPT_EVENT_WAIT_NPC_TALK:
				//case SCRIPT_EVENT_START_GONGCHENGCHUI:
			{
				RaidEventNotify nty;
				raid_event_notify__init(&nty);
				nty.type = config->TypeID;
				nty.param1 = config->Parameter1;
				nty.n_param1 = config->n_Parameter1;
				nty.param2 = config->Parameter2;
				nty.n_param2 = config->n_Parameter2;

				EXTERN_DATA ext_data;
				ext_data.player_id = player->get_uuid();

				fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_RAID_EVENT_NOTIFY, raid_event_notify__pack, nty);
			}
			break;
		}
	}

	//单人副本在玩家进入副本后初始化
	if (raid->m_config->DengeonType == 2 && raid->m_config->DengeonRank != DUNGEON_TYPE_GUILD_LAND)
	{
		do_script_raid_init_cond(raid, script_data);
	}
}

void script_ai_common_escort_stop(raid_struct *raid, player_struct *player, uint32_t escort_id, bool success, struct raid_script_data *script_data)
{
	if (script_data->cur_index >= script_data->script_config->size())
		return;
	
	struct RaidScriptTable *config = (*script_data->script_config)[script_data->cur_index];
	if (config->TypeID != SCRIPT_EVENT_ESCORT_RESULT)
		return;
	if (get_script_raid_event_player(raid) != player)
		return;

	if (config->n_Parameter1 < 3)
	{
		return;
	}
	if ((uint32_t)config->Parameter1[0] != escort_id)
	{
		return;
	}

	int val = -1;
	if (success)
	{
		val = config->Parameter1[1];
	}
	else
	{
		val = config->Parameter1[2];
	}

	if (val == 0)
	{
		script_raid_next(raid, script_data);
	}
	else
	{
		if (config->n_Parameter2 < (uint32_t)val)
		{
			return ;
		}

		if (jump_to_another_script(raid, config->Parameter2[val -1], script_data))
		{
			script_raid_next(raid, script_data);
		}
	}
}

//计算某个怪物的血量是否低于某百分比
static bool do_check_script_raid_monster_hp(double percent, monster_struct* monster)
{
	if(NULL == monster)
	{
		return false;
	}

	double cur_percent = monster->data->attrData[PLAYER_ATTR_HP]/monster->data->attrData[PLAYER_ATTR_MAXHP];
	cur_percent *= 100;
	
	if(cur_percent < percent)
	{
		return true;
	}
	return false;
}
