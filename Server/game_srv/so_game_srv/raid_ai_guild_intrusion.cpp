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
#include <global_param.h>
#include "guild.pb-c.h"
#include "lua_config.h"

static bool guild_ruqin_damage_cmp_func(struct guild_ruqin_player_data l, struct guild_ruqin_player_data r)
{
	if(l.damage_to_monster <= r.damage_to_monster)
	{
		return true;
	}
	return false;
}

//帮会入侵成功或者失败通知帮会服发奖
static void guild_ruqin_reward_info_to_guildsrv(raid_struct *raid, bool succe)
{

	if(raid == NULL || raid->data == NULL)
	{
		return;
	}
	//通知帮会服发奖
	guild_land_raid_struct* gulid_raid = (guild_land_raid_struct*)raid;
	if(gulid_raid == NULL)
		return;

	GuildRuqinActiveRewardNotify notify;
	GuildRuqinActivePlayerInfo player_data[MAX_GUILD_MEMBER_NUM];
	GuildRuqinActivePlayerInfo* player_data_point[MAX_GUILD_MEMBER_NUM];
	uint32_t player_num = 0;
	uint32_t all_damage = 0;
	guild_ruqin_active_reward_notify__init(&notify);
		//先排序
	std::vector<guild_ruqin_player_data> player_info;
	player_info.clear();
	for( std::map<uint64_t, guild_ruqin_player_data>::iterator itr = raid->ruqin_data.palyer_data.begin(); itr != raid->ruqin_data.palyer_data.end(); itr++)
	{
		player_info.push_back(itr->second);
	}
	std::sort(player_info.begin(), player_info.end(), guild_ruqin_damage_cmp_func);
	for( std::vector<guild_ruqin_player_data>::iterator itr = player_info.begin(); itr != player_info.end() && player_num < MAX_GUILD_MEMBER_NUM; itr++)
	{
		player_data_point[player_num] = &player_data[player_num];
		guild_ruqin_active_player_info__init(&player_data[player_num]);
		player_data[player_num].player_id = itr->player_id;
		player_data[player_num].damage = itr->damage_to_monster;
		if(gulid_raid->m_players.find(itr->player_id) != gulid_raid->m_players.end())
		{
			player_data[player_num].on_land = true;
		}
		else 
		{
			player_data[player_num].on_land = false;
		}
		all_damage += itr->damage_to_monster;
		player_num++;
	}
	notify.guild_id = raid->data->ai_data.guild_land_data.guild_id;
	if(succe)
	{
		notify.boshu    = raid->ruqin_data.monster_boshu + 1;  //bos死完，玩家消灭关务波数加一
	}
	else 
	{
		notify.boshu = raid->ruqin_data.monster_boshu;
	}
	notify.all_bushu = raid->ruqin_data.all_boshu;
	notify.all_damage = all_damage;
	notify.all_palyer = player_data_point;
	notify.n_all_palyer = player_num;
	EXTERN_DATA extern_data;
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, SERVER_PROTO_GUILD_RUQIN_REWARD_INFO_NOTIFY, guild_ruqin_active_reward_notify__pack, notify);
	
}


static bool guild_ruqin_player_near_bbq(player_struct *player, uint32_t x, uint32_t z)
{
	struct position *pos = player->get_pos();
	if (abs((int)(pos->pos_x - x ))> sg_guild_ruqin_huodui_fanwei)
		return false;
	if (abs((int)(pos->pos_z - z ))> sg_guild_ruqin_huodui_fanwei)
		return false;	
	return true;
}

static void bosss_creat_notice_guildsrv_send_msg(raid_struct *raid, monster_struct *monster)
{
	if(raid == NULL || monster == NULL || monster->data == NULL)
		return; 
	if(raid->ruqin_data.guild_ruqin == false)
		return;
	if(sg_guild_ruqin_renzu_bossid != monster->data->monster_id && sg_guild_ruqin_yaozu_bossid != monster->data->monster_id)
		return;

	GuildRuqinBossCreatNotify notify;
	guild_ruqin_boss_creat_notify__init(&notify);
	notify.guild_id =  raid->data->ai_data.guild_land_data.guild_id;
	EXTERN_DATA extern_data;
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, SERVER_PROTO_GUILD_RUQIN_BOSS_CREAT_NOTIFY, guild_ruqin_boss_creat_notify__pack, notify);
}


/////////////////////////////////////
static void guild_intrusion_raid_ai_init(raid_struct *raid, player_struct *player)
{

}


static void guild_intrusion_raid_ai_player_enter(raid_struct* raid, player_struct *player)
{

}

static void guild_intrusion_raid_ai_player_leave(raid_struct* raid, player_struct *player)
{

}

static void guild_intrusion_raid_ai_player_dead(raid_struct* raid, player_struct *player, unit_struct* unit)
{

}



static void guild_intrusion_raid_ai_monster_dead(raid_struct *raid, monster_struct *monster, unit_struct *unit)
{
	//帮会入侵活动期间,boss死亡刷火堆,并且停止本次活动
	if(raid->ruqin_data.guild_ruqin == true)
	{

		ParameterTable *parame = get_config_by_id(161000348, &parameter_config);
		if(parame == NULL ||  parame->n_parameter1 != 2 )
			return;
		
		if(monster->data->monster_id != parame->parameter1[0] && monster->data->monster_id != parame->parameter1[1])
			return;
		//boss死亡代表活动结束,清除剩余怪物
		raid->clear_monster(); //清除剩余怪物
		raid->ruqin_data.pos_x = monster->get_pos()->pos_x;
		raid->ruqin_data.pos_z = monster->get_pos()->pos_z;
		raid->ruqin_data.huodui_time = time_helper::get_cached_time() / 1000 + sg_guild_ruqin_huodui_chixutime;
		monster_struct *monster = monster_manager::add_monster(raid->ruqin_data.monster_id, 1, NULL);
		if (!monster)
			return;
		monster->born_pos.pos_x = raid->ruqin_data.pos_x;
		monster->born_pos.pos_z = raid->ruqin_data.pos_z;	
		monster->set_pos(raid->ruqin_data.pos_x, raid->ruqin_data.pos_z);
		if (raid->add_monster_to_scene(monster, 0) != 0)
		{
			LOG_ERR("[%s:%d] add monster to raid faild monster_id[%u]", __FUNCTION__, __LINE__, raid->ruqin_data.monster_id);
		}
		/*WanyaoguBbqNotify notify;
		wanyaogu_bbq_notify__init(&notify);
		notify.result = 1;
		raid->broadcast_to_raid(MSG_ID_WANYAOKA_BBQ_NOTIFY, &notify, (pack_func)wanyaogu_bbq_notify__pack);*/
	}

}

static void guild_intrusion_raid_ai_player_ready(raid_struct* raid, player_struct *player)
{
	if(raid == NULL || raid->data == NULL || player == NULL || player->data == NULL)
		return;

	//这里是判断帮会入侵活动有没有开启
	//if(raid->ruqin_data.guild_ruqin == false)
	//	return;

	if(raid->data->ai_data.guild_land_data.script_data.script_config)
	{
		script_ai_common_player_ready(raid, player, &(raid->data->ai_data.guild_land_data.script_data));
	}
}

static void guild_intrusion_raid_ai_finished(raid_struct* raid)
{

	if(raid == NULL || raid->data == NULL )
	{
		return;
	}

	//这里是判断帮会入侵活动有没有开启
	if(raid->ruqin_data.guild_ruqin == true)
	{
		raid->ruqin_data.status = GUILD_RUQIN_ACTIVE_BBQ;
		raid->ruqin_data.guild_ruqin = false;
		//帮会入侵走到这里说明在限定时间内守护成功
		guild_ruqin_reward_info_to_guildsrv(raid, true);
	}

}


static void guild_intrusion_raid_ai_failed(raid_struct *raid)
{
	if(raid->ruqin_data.guild_ruqin == true)
	{
		bool monster_alive = false;
		for(std::set<monster_struct *>::iterator itr = raid->m_monster.begin(); itr != raid->m_monster.end(); itr++)
		{
			if((*itr)->data->monster_id == sg_guild_ruqin_renzu_bossid || (*itr)->data->monster_id  == sg_guild_ruqin_yaozu_bossid)	
			{
				raid->ruqin_data.pos_x = (*itr)->get_pos()->pos_x;
				raid->ruqin_data.pos_z = (*itr)->get_pos()->pos_z;
				monster_alive = true;
			}
		}
		raid->clear_monster(); //清除剩余怪物
		raid->ruqin_data.guild_ruqin = false;
		raid->ruqin_data.status = GUILD_RUQIN_ACTIVE_BBQ;
		guild_ruqin_reward_info_to_guildsrv(raid, false);

		if(monster_alive == false)
		{
			int len = raid->create_monster_config->size();
			for (int i = 0; i < len; ++i)
			{
				struct SceneCreateMonsterTable *create_config = (*raid->create_monster_config)[i];
				if (!create_config)
					continue;
				if (create_config->ID == sg_guild_ruqin_renzu_bossid || create_config->ID == sg_guild_ruqin_yaozu_bossid)
				{
					raid->ruqin_data.pos_x = create_config->PointPosX;
					raid->ruqin_data.pos_z = create_config->PointPosZ;
					monster_alive = true;
					break;
				}
			}
		}
		if(monster_alive == false)
		{
			LOG_ERR("[%s:%d]帮派入侵刷新火堆失败,位置不对,pox[%u] poz[%u]",__FUNCTION__, __LINE__, raid->ruqin_data.pos_x, raid->ruqin_data.pos_z);
			return ;
		}
		raid->ruqin_data.huodui_time = time_helper::get_cached_time() / 1000 + sg_guild_ruqin_huodui_chixutime;
		monster_struct *monster = monster_manager::add_monster(raid->ruqin_data.monster_id, 1, NULL);
		if (!monster)
			return;
		monster->born_pos.pos_x = raid->ruqin_data.pos_x;
		monster->born_pos.pos_z = raid->ruqin_data.pos_z;	
		monster->set_pos(raid->ruqin_data.pos_x, raid->ruqin_data.pos_z);
		if (raid->add_monster_to_scene(monster, 0) != 0)
		{
			LOG_ERR("[%s:%d] add monster to raid faild monster_id[%u]", __FUNCTION__, __LINE__, raid->ruqin_data.monster_id);
		}
	}

}

static void guild_intrusion_raid_ai_tick(raid_struct *raid)
{
	if(raid == NULL || raid->data == NULL)
		return;
	
	//这里是判断帮会入侵活动有没有开启
	switch (raid->ruqin_data.status)
	{
		case GUILD_RUQIN_ACTIVE_INIT:
			break;
		case GUILD_RUQIN_ACTIVE_START:
			if(raid->data->ai_data.guild_land_data.script_data.script_config)
			{
				script_ai_common_tick(raid, &(raid->data->ai_data.guild_land_data.script_data));
			}
			break;
		case GUILD_RUQIN_ACTIVE_FINISH:
			guild_intrusion_raid_ai_finished(raid);
			break;
		case GUILD_RUQIN_ACTIVE_FAILD:
			guild_intrusion_raid_ai_failed(raid);
			break;
		case GUILD_RUQIN_ACTIVE_BBQ:
		{
			if(raid->ruqin_data.huodui_time > time_helper::get_cached_time() / 1000)
			{
				if(time_helper::get_cached_time()  > raid->ruqin_data.space_time || raid->ruqin_data.space_time ==0)
				{
					raid->ruqin_data.space_time = time_helper::get_cached_time()  + sg_guild_ruqin_huodui_jiange *1000;
					guild_land_raid_struct* gulid_raid = (guild_land_raid_struct*)raid;
					if(gulid_raid == NULL)
						break;
					if(gulid_raid->m_players.size() == 0)
						break;
					for( std::map<uint64_t, player_struct *>::iterator itr = gulid_raid->m_players.begin(); itr != gulid_raid->m_players.end(); itr++)
					{
						if(guild_ruqin_player_near_bbq(itr->second, raid->ruqin_data.pos_x, raid->ruqin_data.pos_z))
						{
							itr->second->add_exp(itr->second->get_attr(PLAYER_ATTR_LEVEL) * sg_guild_ruqin_huodui_exp, MAGIC_TYPE_GUILD_RUQIN_BBQ, true);
						}
					}
				}
			}
			else 
			{
				raid->ruqin_data.status = GUILD_RUQIN_ACTIVE_INIT;
				uint32_t monster_id = raid->ruqin_data.monster_id;
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
		}
			break;
		default:
			break;
	}

}
static void guild_intrusion_raid_ai_monster_attacked(raid_struct *raid, monster_struct *monster, unit_struct *unit, int32_t damage, int32_t before_hp)
{
	if(raid == NULL || raid->data == NULL || monster == NULL || monster->data == NULL || unit == NULL)
	{
		return;
	}

	//这里是判断帮会入侵活动有没有开启
	if(raid->ruqin_data.guild_ruqin == true)
	{

		player_struct *player = NULL;
		if(unit->get_unit_type() == UNIT_TYPE_PLAYER)
		{
			player = (player_struct*)unit;
		}
		else if(unit->get_unit_type() == UNIT_TYPE_PARTNER)
		{

			player = ((partner_struct*)unit)->m_owner;
		}
		else
		{
			return;
		}
		if(player == NULL || player->data == NULL)
			return;

		int32_t real_damage = before_hp > damage ? damage:before_hp;
		if(real_damage <= 0)
			return;
		if(raid->ruqin_data.palyer_data.find(player->data->player_id) != (raid->ruqin_data.palyer_data).end())
		{
			raid->ruqin_data.palyer_data[player->data->player_id].damage_to_monster += real_damage;
		}
		else 
		{
			guild_ruqin_player_data data;
			memset(&data, 0, sizeof(guild_ruqin_player_data));
			data.player_id = player->data->player_id;
			data.damage_to_monster = real_damage;
			raid->ruqin_data.palyer_data.insert(std::make_pair(player->data->player_id, data));
		}
	}
}

static void guild_intrusion_raid_ai_monster_relive(raid_struct *raid, monster_struct *monster)
{
	bosss_creat_notice_guildsrv_send_msg(raid, monster);
}

struct raid_ai_interface raid_ai_guild_intrusion_interface =
{
	guild_intrusion_raid_ai_init, //初始化
	guild_intrusion_raid_ai_tick, //定时驱动
	guild_intrusion_raid_ai_player_enter, //玩家进入
	guild_intrusion_raid_ai_player_leave, //玩家离开
	guild_intrusion_raid_ai_player_dead,  //玩家死亡
	NULL,
	guild_intrusion_raid_ai_monster_dead, //怪物死亡
	NULL,
	guild_intrusion_raid_ai_player_ready, //玩家进入场景
	guild_intrusion_raid_ai_finished, //完成
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	guild_intrusion_raid_ai_failed, //失败
	NULL,
	NULL,
	guild_intrusion_raid_ai_monster_attacked, //怪物被击
	guild_intrusion_raid_ai_monster_relive, //怪物创建或者复活
};
