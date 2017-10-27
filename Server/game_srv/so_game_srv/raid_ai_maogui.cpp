#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <fcntl.h>
#include <stdlib.h>
#include "game_event.h"
#include "raid_ai.h"
#include "raid_manager.h"
#include "map_config.h"
#include "time_helper.h"
#include "player_manager.h"
#include "check_range.h"
#include "unit.h"
#include "msgid.h"
#include "raid.pb-c.h"
#include "pvp_raid.pb-c.h"
#include "scene_transfer.pb-c.h"
#include "app_data_statis.h"
#include "buff_manager.h"
#include "monster_manager.h"
#include "guild_battle_manager.h"
#include "relive.pb-c.h"
#include "guild_battle.pb-c.h"
#include "raid_ai_common.h"
#include "guild_land_raid.h"
#include "wanyaogu.pb-c.h"
#include "../proto/cast_skill.pb-c.h"
static void magui_raid_creat_zhengning_maogui(raid_struct *raid, monster_struct *monster)
{
	if(raid == NULL || raid->data == NULL || monster == NULL || monster->data == NULL)
		return;

	std::map<uint64_t, struct MGLYmaoguiwangTable*>::iterator itr =  maogui_maogui_wang_config.find(raid->data->ai_data.maogui_data.gui_wang_id);
	if(itr == maogui_maogui_wang_config.end())
		return;

	if(itr->second->n_Monster1 < 2)
		return;

	uint32_t shouling_xiabiao = rand() % itr->second->n_Monster1;
	uint32_t shouling_id = itr->second->Monster1[shouling_xiabiao];
	std::map<uint64_t, struct MGLYyanseTable*>::iterator yanseitr = maogui_shouling_colour_config.find(shouling_id);
	if(yanseitr == maogui_shouling_colour_config.end())
	{
		LOG_ERR("猫鬼乐园刷出首领和小怪失败,在MGLYyanseTable配置表中未找到首领怪的配置shouling_monster_id[%u]", shouling_id);
		return;
	}
	uint32_t xiaoguai_id = 0;
	while(1)
	{
		uint32_t monster2_xiabiao = rand() % itr->second->n_Monster2;
		xiaoguai_id = itr->second->Monster2[monster2_xiabiao];
		std::map<uint64_t, struct MGLYyanseTable*>::iterator xiaoguai = maogui_xiaoguai_colour_config.find(xiaoguai_id);
		if(xiaoguai == maogui_xiaoguai_colour_config.end())
		{
			LOG_ERR("猫鬼乐园刷出首领和小怪失败,在MGLYyanseTable配置表中未找到小怪的配置xiaoguai_id[%u]", xiaoguai_id);
			return;
		}
		if(xiaoguai->second->Colour != yanseitr->second->Colour)
		break;
	}

	struct position *pos = monster->get_pos();
	int32_t pos_x =  pos->pos_x + itr->second->SeparateRange - rand()% (2*itr->second->SeparateRange);
	int32_t pos_z =  pos->pos_z + itr->second->SeparateRange - rand()% (2*itr->second->SeparateRange);
	if(itr->second->Effects != NULL && itr->second->n_EffectsParameter >= 2)
	{
		double parama[5];
		parama[0] = pos->pos_x;
		parama[1] = 10000;
		parama[2] = pos->pos_z;
		parama[3] = itr->second->EffectsParameter[0];
		parama[4] = itr->second->EffectsParameter[1];
		RaidEventNotify nty;
		raid_event_notify__init(&nty);
		nty.type = 45;
		nty.param1 = parama;
		nty.n_param1 = 5;
		nty.param2 = &itr->second->Effects;
		nty.n_param2 = 1;
		raid->broadcast_to_raid(MSG_ID_RAID_EVENT_NOTIFY, &nty, (pack_func)raid_event_notify__pack);
	}
	monster_manager::create_monster_at_pos(raid, shouling_id, raid->data->monster_level, pos_x, pos_z, 0, NULL, 0);

	for(size_t j = 0; j < itr->second->n_Monster1 - 1; j++)
	{
		int32_t pos_x =  pos->pos_x + itr->second->SeparateRange - rand()% (2*itr->second->SeparateRange);
		int32_t pos_z =  pos->pos_z + itr->second->SeparateRange - rand()% (2*itr->second->SeparateRange);
		if(itr->second->Effects != NULL && itr->second->n_EffectsParameter >= 2)
		{
			double parama[5];
			parama[0] = pos->pos_x;
			parama[1] = 10000;
			parama[2] = pos->pos_z;
			parama[3] = itr->second->EffectsParameter[0];
			parama[4] = itr->second->EffectsParameter[1];
			RaidEventNotify nty;
			raid_event_notify__init(&nty);
			nty.type = 45;
			nty.param1 = parama;
			nty.n_param1 = 5;
			nty.param2 = &itr->second->Effects;
			nty.n_param2 = 1;
			raid->broadcast_to_raid(MSG_ID_RAID_EVENT_NOTIFY, &nty, (pack_func)raid_event_notify__pack);
		}
		monster_manager::create_monster_at_pos(raid, xiaoguai_id, raid->data->monster_level, pos_x, pos_z, 0, NULL, 0);
	}

}
static void maogui_raid_ai_add_buff_to_guiwang(raid_struct *raid)
{
	if(raid == NULL || raid->data == NULL)
	{
		LOG_ERR("[%s:%d] 猫鬼乐园副本给鬼王加buff失败", __FUNCTION__, __LINE__);
		return;
	}
	if(time_helper::get_cached_time() < raid->data->ai_data.maogui_data.buff_time || raid->data->ai_data.maogui_data.buff_time == 0)
		return;
	LOG_ERR("鬼王加buff 当前时间[%lu], buff持续时间[%lu]", time_helper::get_cached_time(), raid->data->ai_data.maogui_data.buff_time);

	raid->data->ai_data.maogui_data.buff_time = 0;
	for(std::set<monster_struct *>::iterator itr =  raid->m_monster.begin(); itr != raid->m_monster.end(); itr++)
	{
		monster_struct *monster = *itr;
		if(monster->data->monster_id == raid->data->ai_data.maogui_data.gui_wang_id)
		{
			buff_manager::create_default_buff(sg_maogui_guiwang_wudi_buff, monster, monster, true);
			magui_raid_creat_zhengning_maogui(raid, monster);
			break;
		}	
	}

}



static void maogui_raid_ai_player_enter(raid_struct* raid, player_struct *player)
{

}

static void maogui_raid_ai_player_leave(raid_struct* raid, player_struct *player)
{

}

static void maogui_raid_ai_player_dead(raid_struct* raid, player_struct *player, unit_struct* unit)
{

}



static void maogui_raid_ai_monster_dead(raid_struct *raid, monster_struct *monster, unit_struct *unit)
{
	std::map<uint64_t, struct MGLYyanseTable*>::iterator itr =  maogui_colour_config.find(monster->data->monster_id);
	if(itr != maogui_colour_config.end())
	{
		switch(itr->second->Type)
		{
			case 1:
				if((uint32_t)itr->second->Colour == raid->data->ai_data.maogui_data.diaoxiang_colour && raid->data->ai_data.maogui_data.diaoxiang_id != 0)
				{
					for(std::set<monster_struct *>::iterator ite =  raid->m_monster.begin(); ite != raid->m_monster.end();)
					{
						
						std::set<monster_struct *>::iterator next_itr = ite;
						++next_itr;
						if ((*ite)->data->monster_id == raid->data->ai_data.maogui_data.diaoxiang_id)
						{
							buff_manager::create_default_buff(sg_maogui_diaoxiang_stop_buff, *ite, *ite, true);
						}
						else
						{

							monster_struct *m = *ite;
							m->broadcast_one_attr_changed(PLAYER_ATTR_HP, -1, false, true);
							raid->delete_monster_from_scene(m, true);
							monster_manager::delete_monster(m);
						}
						ite = next_itr;
					}
				}
				break;
			case 2:
				//首领死了破除鬼王无敌buff
				for(std::set<monster_struct *>::iterator itr =  raid->m_monster.begin(); itr != raid->m_monster.end(); itr++)
				{
					monster_struct *m = *itr;
					if(raid->data->ai_data.maogui_data.gui_wang_id == m->data->monster_id)
					{
						raid->data->ai_data.maogui_data.buff_time = time_helper::get_cached_time() + raid->data->ai_data.maogui_data.po_buff_time;
						m->delete_one_buff(sg_maogui_guiwang_wudi_buff);
						AddBuffNotify notify;
						add_buff_notify__init(&notify);
						notify.buff_id = sg_maogui_guiwang_wudi_buff;
						notify.playerid = m->get_uuid();
						m->broadcast_to_sight(MSG_ID_DEL_BUFF_NOTIFY, &notify, (pack_func)add_buff_notify__pack, false);
					}
				}
				break;
			case 3:
				raid->data->ai_data.maogui_data.diaoxiang_id = 0;
				raid->data->ai_data.maogui_data.diaoxiang_colour = 0;
				break;
			default:
				break;
		
		}
	}
}

static void maogui_raid_ai_player_ready(raid_struct* raid, player_struct *player)
{
	script_ai_common_player_ready(raid, player, &(raid->data->ai_data.maogui_data.script_data));
	if(!player->m_team)
	{
		raid->data->monster_level = player->get_attr(PLAYER_ATTR_LEVEL);
	}
	else 
	{
		uint32_t all_level = 0;
		uint32_t num = 0;
		for (int pos = 0; pos < player->m_team->m_data->m_memSize; ++pos)
		{
			player_struct *t_player = player_manager::get_player_by_id(player->m_team->m_data->m_mem[pos].id);
			if(t_player)
			{
				all_level += t_player->get_attr(PLAYER_ATTR_LEVEL);
				num++;
			}
		}
		if(num != 0)
		{
			raid->data->monster_level = all_level/num;
		}
	}

}

static void maogui_raid_ai_finished(raid_struct* raid)
{


}


static void maogui_raid_ai_failed(raid_struct *raid)
{

}

static void maogui_raid_ai_tick(raid_struct *raid)
{

	script_ai_common_tick(raid, &raid->data->ai_data.maogui_data.script_data);
	maogui_raid_ai_add_buff_to_guiwang(raid);
}

static void maogui_raid_ai_init(raid_struct *raid, player_struct *player)
{
	raid->init_common_script_data(raid->m_config->DungeonPass, &(raid->data->ai_data.maogui_data.script_data));
	//多人副本在副本初始化的时候，初始化AI
	if (raid->m_config->DengeonType == 1)
	{
		do_script_raid_init_cond(raid, &(raid->data->ai_data.maogui_data.script_data));
	}
}
static void maogui_raid_ai_monster_live(raid_struct *raid, monster_struct *monster)
{
	if(raid == NULL || raid->data == NULL || monster == NULL || monster->data == NULL)
		return;
	std::map<uint64_t, struct MGLYyanseTable*>::iterator ite =  maogui_diaoxiang_colour_config.find(monster->data->monster_id);
	if(ite !=  maogui_diaoxiang_colour_config.end())
	{
		raid->data->ai_data.maogui_data.diaoxiang_id = monster->data->monster_id;
		raid->data->ai_data.maogui_data.diaoxiang_colour = ite->second->Colour;
	}

	std::map<uint64_t, struct MGLYmaoguiwangTable*>::iterator itr =  maogui_maogui_wang_config.find(monster->data->monster_id);
	if(0 == raid->data->ai_data.maogui_data.gui_wang_id && itr != maogui_maogui_wang_config.end())
	{
		raid->data->ai_data.maogui_data.gui_wang_id = monster->data->monster_id;
		raid->data->ai_data.maogui_data.po_buff_time = itr->second->Time;
		buff_manager::create_default_buff(sg_maogui_guiwang_wudi_buff, monster, monster, true);
		magui_raid_creat_zhengning_maogui(raid, monster);
	}

}
struct raid_ai_interface raid_ai_maogui_interface =
{
	maogui_raid_ai_init, //初始化
	maogui_raid_ai_tick, //定时驱动
	maogui_raid_ai_player_enter, //玩家进入
	maogui_raid_ai_player_leave, //玩家离开
	maogui_raid_ai_player_dead,  //玩家死亡
	NULL,
	maogui_raid_ai_monster_dead, //怪物死亡
	NULL,
	maogui_raid_ai_player_ready, //玩家进入场景
	maogui_raid_ai_finished, //完成
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	maogui_raid_ai_failed, //失败
	NULL,
	NULL,
	NULL, //怪物被击
	maogui_raid_ai_monster_live, //怪物重生或者创建
};
