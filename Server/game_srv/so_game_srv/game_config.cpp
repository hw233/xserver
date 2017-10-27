#include <stdio.h>
#include <assert.h>
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
#include "time_helper.h"
extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include "sproto.h"
#include "sprotoc_common.h"
#include "game_config.h"
#include "game_event.h"
#include "scene_manager.h"
#include "lua_load.h"
#include "raid.h"
#include "lua_config.h"

//将参数表里的参数读到内部变量里
static void generate_parameters(void)
{
	ParameterTable *config = NULL;
	config = get_config_by_id(162000001, &parameter_config);
	if (config && config->n_parameter1 >= 2)
	{
		sg_bag_unlock_base_price = config->parameter1[0];
		sg_bag_unlock_incr_factor = config->parameter1[1];
	}
	config = get_config_by_id(161000002, &parameter_config);
	if (config && config->n_parameter1 >= 2)
	{
		sg_rename_item_id = config->parameter1[0];
		sg_rename_item_num = config->parameter1[1];
	}
	sg_fight_rand = get_config_by_id(161000312, &parameter_config)->parameter1[0];
	assert(sg_fight_rand);
		
	sg_relive_free_times = get_config_by_id(161000008, &parameter_config)->parameter1[0];
	sg_relive_first_cost = get_config_by_id(161000009, &parameter_config)->parameter1[0];
	sg_relive_grow_cost = get_config_by_id(161000010, &parameter_config)->parameter1[0];
	sg_relive_max_cost = get_config_by_id(161000011, &parameter_config)->parameter1[0];
	sg_gem_strip_coin = get_config_by_id(161000019, &parameter_config)->parameter1[0];
	sg_player_level_limit = get_config_by_id(161000020, &parameter_config)->parameter1[0];

	sg_wanyaogu_range = get_config_by_id(161000028, &parameter_config)->parameter1[0];
	sg_wanyaogu_reward = get_config_by_id(161000029, &parameter_config)->parameter1[0];
	sg_wanyaogu_time_delta = get_config_by_id(161000030, &parameter_config)->parameter1[0];
	sg_wanyaogu_time_total = get_config_by_id(161000031, &parameter_config)->parameter1[0];
	sg_wanyaogu_start_time = get_config_by_id(161000073, &parameter_config)->parameter1[0];	

	sg_pk_level = get_config_by_id(161000034, &parameter_config)->parameter1[0];
	sg_set_pk_type_cd = get_config_by_id(161000035, &parameter_config)->parameter1[0];
	sg_muder_num_max = get_config_by_id(161000036, &parameter_config)->parameter1[0];
	sg_muder_add_num = get_config_by_id(161000037, &parameter_config)->parameter1[0];
	sg_muder_sub_num = get_config_by_id(161000038, &parameter_config)->parameter1[0];
	sg_muder_item_value = get_config_by_id(161000039, &parameter_config)->parameter1[0];
	sg_qiecuo_out_range_timeout = get_config_by_id(161000040, &parameter_config)->parameter1[0];
	sg_qiecuo_range = get_config_by_id(161000067, &parameter_config)->parameter1[0];
	sg_qiecuo_god_time = get_config_by_id(161000126, &parameter_config)->parameter1[0];
	sg_qiecuo_god_time *= 1000;

	sg_hp_pool_percent = get_config_by_id(161000177, &parameter_config)->parameter1[0];
	sg_hp_pool_percent /= 10000.0;

	sg_hp_pool_max = get_config_by_id(161000176, &parameter_config)->parameter1[0];

	config = get_config_by_id(161000007, &parameter_config);
	assert(config && config->n_parameter1 > 0);
	sg_raid_keep_time = config->parameter1[0];

	config = get_config_by_id(161000041, &parameter_config);
	if (config && config->n_parameter1 >= 2)
	{
		sg_yuqidao_break_item_id = config->parameter1[0];
		sg_yuqidao_break_item_num = config->parameter1[1];
	}

	ParameterTable *pvp_raid_param = get_config_by_id(161000058, &parameter_config);
	for (int i = 0; i < 6; ++i)
		sg_3v3_pvp_raid_param1[i] = (int)(pvp_raid_param->parameter1[i]);
	sg_3v3_pvp_raid_param1[5] *= 100;
	
	pvp_raid_param = get_config_by_id(161000059, &parameter_config);
	for (int i = 0; i < 6; ++i)
		sg_3v3_pvp_raid_param2[i] = (int)(pvp_raid_param->parameter1[i]);
	sg_3v3_pvp_raid_param2[5] *= 100;

/// 工会战
	pvp_raid_param = get_config_by_id(161000178, &parameter_config);
	for (int i = 0; i < 6; ++i)
		sg_guild_raid_param1[i] = (int)(pvp_raid_param->parameter1[i]);
	sg_guild_raid_param1[5] *= 100;
	pvp_raid_param = get_config_by_id(161000179, &parameter_config);
	for (int i = 0; i < 6; ++i)
		sg_guild_raid_param2[i] = (int)(pvp_raid_param->parameter1[i]);
	sg_guild_raid_param2[5] *= 100;
	pvp_raid_param = get_config_by_id(161000180, &parameter_config);
	for (int i = 0; i < 6; ++i)
		sg_guild_raid_final_param1[i] = (int)(pvp_raid_param->parameter1[i]);
	sg_guild_raid_final_param1[5] *= 100;
	pvp_raid_param = get_config_by_id(161000181, &parameter_config);
	for (int i = 0; i < 6; ++i)
		sg_guild_raid_final_param2[i] = (int)(pvp_raid_param->parameter1[i]);
	sg_guild_raid_final_param2[5] *= 100;
	pvp_raid_param = get_config_by_id(161000182, &parameter_config);
	for (int i = 0; i < 6; ++i)
		sg_guild_raid_final_param3[i] = (int)(pvp_raid_param->parameter1[i]);
	sg_guild_raid_final_param3[5] *= 100;
	pvp_raid_param = get_config_by_id(161000183, &parameter_config);
	for (int i = 0; i < 6; ++i)
		sg_guild_raid_final_param4[i] = (int)(pvp_raid_param->parameter1[i]);
	sg_guild_raid_final_param4[5] *= 100;
///	
	
	struct DungeonTable *m = get_config_by_id(sg_3v3_pvp_raid_param1[0], &all_raid_config);
	assert(m);
	sg_pvp_control_config_3 = get_config_by_id(m->ActivityControl, &all_control_config);
	assert(sg_pvp_control_config_3);
	
	pvp_raid_param = get_config_by_id(161000060, &parameter_config);
	for (int i = 0; i < 6; ++i)
		sg_5v5_pvp_raid_param1[i] = (int)(pvp_raid_param->parameter1[i]);
	sg_5v5_pvp_raid_param1[5] *= 100;
	
	pvp_raid_param = get_config_by_id(161000061, &parameter_config);	
	for (int i = 0; i < 6; ++i)
		sg_5v5_pvp_raid_param2[i] = (int)(pvp_raid_param->parameter1[i]);
	sg_5v5_pvp_raid_param2[5] *= 100;
	m = get_config_by_id(sg_5v5_pvp_raid_param1[0], &all_raid_config);
	assert(m);
	sg_pvp_control_config_5 = get_config_by_id(m->ActivityControl, &all_control_config);	
	assert(sg_pvp_control_config_5);
	
	pvp_raid_param = get_config_by_id(161000070, &parameter_config);
	sg_pvp_raid_relive_cd = (int)(pvp_raid_param->parameter1[0]);

	pvp_raid_param = get_config_by_id(161000107, &parameter_config);
	sg_muder_punish_point = (int)(pvp_raid_param->parameter1[0]);
	pvp_raid_param = get_config_by_id(161000108, &parameter_config);
	sg_muder_punish_base = (int)(pvp_raid_param->parameter1[0]);
	pvp_raid_param = get_config_by_id(161000109, &parameter_config);
	sg_muder_punish_inc[0] = (int)(pvp_raid_param->parameter1[0]);
	sg_muder_punish_inc[1] = (int)(pvp_raid_param->parameter1[1]);	
	pvp_raid_param = get_config_by_id(161000112, &parameter_config);
	sg_muder_punish_max = (int)(pvp_raid_param->parameter1[0]);

	pvp_raid_param = get_config_by_id(161000130, &parameter_config);
	sg_muder_cant_set_pktype = (int)(pvp_raid_param->parameter1[0]);
	pvp_raid_param = get_config_by_id(161000131, &parameter_config);
	sg_muder_debuff[0] = (int)(pvp_raid_param->parameter1[0]);
	sg_muder_debuff[1] = (int)(pvp_raid_param->parameter1[1]);		

	pvp_raid_param = get_config_by_id(161000076, &parameter_config);	
	sg_pvp_raid_blue_region_buff_id[0] = (int)(pvp_raid_param->parameter1[0]);
	sg_pvp_raid_blue_region_buff_id[1] = (int)(pvp_raid_param->parameter1[1]);	
	struct BuffTable *t_config = get_config_by_id((uint64_t)(sg_pvp_raid_blue_region_buff_id[0]), &buff_config);
	assert(t_config);
	sg_pvp_raid_blue_region_buff_rate = t_config->NeedPro;

	MATCH_LEVEL_DIFF = (get_config_by_id(161000055, &parameter_config)->parameter1[0]);
	TEAM_LEVEL1_DIFF_L = (get_config_by_id(161000056, &parameter_config)->parameter1[0]);
	TEAM_LEVEL1_DIFF_R = (get_config_by_id(161000056, &parameter_config)->parameter1[1]);
	TEAM_LEVEL2_DIFF_L = (get_config_by_id(161000057, &parameter_config)->parameter1[0]);
	TEAM_LEVEL2_DIFF_R = (get_config_by_id(161000057, &parameter_config)->parameter1[1]);
	
	pvp_raid_param = get_config_by_id(161000077, &parameter_config);	
	sg_pvp_raid_red_region_buff_id[0] = (int)(pvp_raid_param->parameter1[0]);
	sg_pvp_raid_red_region_buff_id[1] = (int)(pvp_raid_param->parameter1[1]);		

	pvp_raid_param = get_config_by_id(161000066, &parameter_config);	
	sg_pvp_raid_monster_refresh_time = (int)(pvp_raid_param->parameter1[0]);

	pvp_raid_param = get_config_by_id(161000124, &parameter_config);	
	sg_pvp_raid_monster_first_refresh_time = (int)(pvp_raid_param->parameter1[0]);	

	pvp_raid_param = get_config_by_id(161000123, &parameter_config);		
	sg_pvp_raid_buff_relive_time = (int)(pvp_raid_param->parameter1[0]);
	pvp_raid_param = get_config_by_id(161000125, &parameter_config);		
	sg_pvp_center_buff_id[0] = (int)(pvp_raid_param->parameter1[0]);
	sg_pvp_center_buff_id[1] = (int)(pvp_raid_param->parameter1[1]);

	pvp_raid_param = get_config_by_id(161000223, &parameter_config);
	for (int i = 0; i < 4; ++i)
		sg_leiminggu_boss_pos[i] = pvp_raid_param->parameter1[i];
	pvp_raid_param = get_config_by_id(161000224, &parameter_config);
	for (int i = 0; i < 4; ++i)
		sg_leiminggu_pos[i] = pvp_raid_param->parameter1[i];
	sg_leiminggu_collect_id = (uint32_t)pvp_raid_param->parameter1[4];

	pvp_raid_param = get_config_by_id(161000219, &parameter_config);
	for (int i = 0; i < 4; ++i)
		sg_guild_raid_final_monster_id[i] = (uint32_t)pvp_raid_param->parameter1[i];

	pvp_raid_param = get_config_by_id(161000230, &parameter_config);
	sg_n_shishen_id = pvp_raid_param->n_parameter1;
	sg_shishen_shouling_id = (uint32_t *)malloc(sizeof(uint32_t) * sg_n_shishen_id);
	assert(sg_shishen_shouling_id);
	sg_shishen_xiaoguai_id = (uint32_t *)malloc(sizeof(uint32_t) * sg_n_shishen_id);
	assert(sg_shishen_xiaoguai_id);	
	for (int i = 0; i < sg_n_shishen_id; ++i)
		sg_shishen_xiaoguai_id[i] = (uint32_t)pvp_raid_param->parameter1[i];
	pvp_raid_param = get_config_by_id(161000231, &parameter_config);
	for (int i = 0; i < sg_n_shishen_id; ++i)
		sg_shishen_shouling_id[i] = (uint32_t)pvp_raid_param->parameter1[i];	
	
	pvp_raid_param = get_config_by_id(161000068, &parameter_config);
	for (int i = 0; i < 3; ++i)	
		sg_3v3_pvp_monster_id[i] = (int)(pvp_raid_param->parameter1[i]);			
	pvp_raid_param = get_config_by_id(161000069, &parameter_config);		
	for (int i = 0; i < 3; ++i)	
		sg_5v5_pvp_monster_id[i] = (int)(pvp_raid_param->parameter1[i]);			
	pvp_raid_param = get_config_by_id(161000071, &parameter_config);
	for (int i = 0; i < 13; ++i)	
		sg_3v3_pvp_monster_place[i] = (int)(pvp_raid_param->parameter1[i]);			
	pvp_raid_param = get_config_by_id(161000072, &parameter_config);
	for (int i = 0; i < 13; ++i)
		sg_5v5_pvp_monster_place[i] = (int)(pvp_raid_param->parameter1[i]);
	pvp_raid_param = get_config_by_id(161000085, &parameter_config);
	for (int i = 0; i < 3; ++i)	
		sg_3v3_pvp_buff_id[i] = (int)(pvp_raid_param->parameter1[i]);			
	pvp_raid_param = get_config_by_id(161000086, &parameter_config);		
	for (int i = 0; i < 3; ++i)	
		sg_5v5_pvp_buff_id[i] = (int)(pvp_raid_param->parameter1[i]);			

	pvp_raid_param = get_config_by_id(161000062, &parameter_config);	
	for (int i = 0; i < 5; ++i)
		sg_3v3_pvp_raid_red_buff_param[i] = (int)(pvp_raid_param->parameter1[i]);
	pvp_raid_param = get_config_by_id(161000063, &parameter_config);
	for (int i = 0; i < 5; ++i)
		sg_3v3_pvp_raid_blue_buff_param[i] = (int)(pvp_raid_param->parameter1[i]);
	pvp_raid_param = get_config_by_id(161000064, &parameter_config);
	for (int i = 0; i < 5; ++i)
		sg_5v5_pvp_raid_red_buff_param[i] = (int)(pvp_raid_param->parameter1[i]);
	pvp_raid_param = get_config_by_id(161000065, &parameter_config);	
	for (int i = 0; i < 5; ++i)
		sg_5v5_pvp_raid_blue_buff_param[i] = (int)(pvp_raid_param->parameter1[i]);

	pvp_raid_param = get_config_by_id(161000075, &parameter_config);	
	sg_pvp_raid_fighting_capacity_range[0] = (int)(pvp_raid_param->parameter1[0]);
	sg_pvp_raid_fighting_capacity_range[1] = (int)(pvp_raid_param->parameter1[1]);

	pvp_raid_param = get_config_by_id(161000042, &parameter_config);
	sg_pvp_raid_win_score_param = pvp_raid_param->parameter1[0];
	pvp_raid_param = get_config_by_id(161000043, &parameter_config);
	sg_pvp_raid_lose_score_param = pvp_raid_param->parameter1[0];
	pvp_raid_param = get_config_by_id(161000044, &parameter_config);
	sg_pvp_raid_tie_score_param = pvp_raid_param->parameter1[0];
	pvp_raid_param = get_config_by_id(161000045, &parameter_config);
	sg_pvp_raid_reward_score_3v3_param = pvp_raid_param->parameter1[0];
	pvp_raid_param = get_config_by_id(161000046, &parameter_config);
	sg_pvp_raid_reward_score_5v5_param = pvp_raid_param->parameter1[0];

	pvp_raid_param = get_config_by_id(161000047, &parameter_config);
	sg_pvp_raid_win_gold_param = pvp_raid_param->parameter1[0];
	pvp_raid_param = get_config_by_id(161000048, &parameter_config);
	sg_pvp_raid_lose_gold_param = pvp_raid_param->parameter1[0];
	pvp_raid_param = get_config_by_id(161000049, &parameter_config);
	sg_pvp_raid_tie_gold_param = pvp_raid_param->parameter1[0];
	pvp_raid_param = get_config_by_id(161000050, &parameter_config);
	sg_pvp_raid_kill_param = pvp_raid_param->parameter1[0];
	pvp_raid_param = get_config_by_id(161000051, &parameter_config);
	sg_pvp_raid_assist_param = pvp_raid_param->parameter1[0];
	pvp_raid_param = get_config_by_id(161000052, &parameter_config);
	sg_pvp_raid_basic_param = pvp_raid_param->parameter1[0];

	sg_transfer_out_stuck_cd_time = get_config_by_id(161000110, &parameter_config)->parameter1[0];
	config = get_config_by_id(161000147, &parameter_config);
	if (config && config->n_parameter1 >= 1)
	{
		sg_guild_scene_id = config->parameter1[0];
	}

	sg_guild_battle_match_time = get_config_by_id(161000184, &parameter_config)->parameter1[0];
	sg_guild_battle_fight_time = get_config_by_id(161000185, &parameter_config)->parameter1[0];
	sg_guild_battle_settle_time = get_config_by_id(161000186, &parameter_config)->parameter1[0];
	sg_guild_battle_final_match_time = get_config_by_id(161000187, &parameter_config)->parameter1[0];
	sg_guild_battle_final_fight_time = get_config_by_id(161000188, &parameter_config)->parameter1[0];
	sg_guild_battle_final_settle_time = get_config_by_id(161000189, &parameter_config)->parameter1[0];
	sg_guild_battle_wait_award_interval = get_config_by_id(161000203, &parameter_config)->parameter1[0];
	ParameterTable *guild_battle_wait_id_param = get_config_by_id(161000204, &parameter_config);
	ParameterTable *guild_battle_wait_num_param = get_config_by_id(161000205, &parameter_config);
	sg_guild_battle_wait_award.clear();
	if (guild_battle_wait_id_param && guild_battle_wait_num_param)
	{
		for (uint32_t i = 0; i < guild_battle_wait_id_param->n_parameter1 && i < guild_battle_wait_num_param->n_parameter1; ++i)
		{
			sg_guild_battle_wait_award[guild_battle_wait_id_param->parameter1[i]] += guild_battle_wait_num_param->parameter1[i];
		}
	}
	sg_guild_battle_brave_init = get_config_by_id(161000206, &parameter_config)->parameter1[0];

	config = get_config_by_id(161000207, &parameter_config);
	if (config && config->n_parameter1 >= 4)
	{
		for (uint32_t i = 0; i < 4; ++i)
		{
			sg_guild_battle_fight_auto_win_reward[i] = config->parameter1[i];
		}
	}
	config = get_config_by_id(161000208, &parameter_config);
	if (config && config->n_parameter1 >= 4)
	{
		for (uint32_t i = 0; i < 4; ++i)
		{
			sg_guild_battle_fight_win_reward[i] = config->parameter1[i];
		}
	}
	config = get_config_by_id(161000209, &parameter_config);
	if (config && config->n_parameter1 >= 4)
	{
		for (uint32_t i = 0; i < 4; ++i)
		{
			sg_guild_battle_fight_lose_reward[i] = config->parameter1[i];
		}
	}
	config = get_config_by_id(161000210, &parameter_config);
	if (config && config->n_parameter1 >= 4)
	{
		for (uint32_t i = 0; i < 4; ++i)
		{
			sg_guild_battle_fight_draw_reward[i] = config->parameter1[i];
		}
	}
	config = get_config_by_id(161000211, &parameter_config);
	if (config && config->n_parameter1 >= 3)
	{
		for (uint32_t i = 0; i < 3; ++i)
		{
			sg_guild_battle_treasure_factor[i] = config->parameter1[i];
		}
	}
	config = get_config_by_id(161000212, &parameter_config);
	if (config && config->n_parameter1 >= 2)
	{
		sg_guild_battle_round_num = config->parameter1[0];
		sg_guild_battle_final_round_num = config->parameter1[1];
	}
	config = get_config_by_id(161000213, &parameter_config);
	if (config && config->n_parameter1 >= 4)
	{
		for (uint32_t i = 0; i < 4; ++i)
		{
			sg_guild_battle_final_fight_reward_0[i] = config->parameter1[i];
		}
	}
	config = get_config_by_id(161000214, &parameter_config);
	if (config && config->n_parameter1 >= 3)
	{
		for (uint32_t i = 0; i < 3; ++i)
		{
			sg_guild_battle_final_fight_reward_1[i] = config->parameter1[i];
		}
	}
	config = get_config_by_id(161000215, &parameter_config);
	if (config && config->n_parameter1 >= 3)
	{
		for (uint32_t i = 0; i < 3; ++i)
		{
			sg_guild_battle_final_fight_reward_2[i] = config->parameter1[i];
		}
	}
	config = get_config_by_id(161000216, &parameter_config);
	if (config && config->n_parameter1 >= 3)
	{
		for (uint32_t i = 0; i < 3; ++i)
		{
			sg_guild_battle_final_fight_reward_3[i] = config->parameter1[i];
		}
	}
	config = get_config_by_id(161000217, &parameter_config);
	if (config && config->n_parameter1 >= 3)
	{
		for (uint32_t i = 0; i < 3; ++i)
		{
			sg_guild_battle_final_fight_reward_4[i] = config->parameter1[i];
		}
	}
	config = get_config_by_id(161000218, &parameter_config);
	if (config && config->n_parameter1 >= 4)
	{
		for (uint32_t i = 0; i < 4; ++i)
		{
			sg_guild_battle_final_score_factor[i] = config->parameter1[i];
		}
	}
	config = get_config_by_id(161000220, &parameter_config);
	if (config && config->n_parameter1 >= 1)
	{
		for (uint32_t i = 0; i < 1; ++i)
		{
			sg_guild_battle_final_treasure_factor[i] = config->parameter1[i];
		}
	}

//	sg_guild_battle_match_time = 20;
//	sg_guild_battle_fight_time = 20;
//	sg_guild_battle_settle_time = 20;
//	sg_guild_battle_round_num = 6;
//	sg_guild_battle_final_match_time = 20;
//	sg_guild_battle_final_fight_time = 60;
//	sg_guild_battle_final_settle_time = 20;

	config = get_config_by_id(161000235, &parameter_config);
	if (config && config->n_parameter1 >= 1)
	{
		sg_partner_assist_percent = (double)config->parameter1[0] / (double)100;
	}
	config = get_config_by_id(161000237, &parameter_config);
	if (config && config->n_parameter1 >= 1)
	{
		sg_partner_anger_max = config->parameter1[0];
	}
	config = get_config_by_id(161000255, &parameter_config);
	if (config && config->n_parameter1 >= 1)
	{
		sg_partner_relive_time = config->parameter1[0];
	}
	config = get_config_by_id(161000298, &parameter_config);
	if (config && config->n_parameter1 >= 3)
	{
		sg_partner_sanshenshi_id = config->parameter1[0];
		sg_partner_sanshenshi_score = config->parameter1[1];
		sg_partner_sanshenshi_coin = config->parameter1[2];
	}
	config = get_config_by_id(161000299, &parameter_config);
	if (config && config->n_parameter1 >= 3)
	{
		sg_partner_qiyaoshi_id = config->parameter1[0];
		sg_partner_qiyaoshi_score = config->parameter1[1];
		sg_partner_qiyaoshi_coin = config->parameter1[2];
	}

	sg_fight_param_161000274 = get_config_by_id(161000274, &parameter_config)->parameter1[0]; 
	sg_fight_param_161000275 = get_config_by_id(161000275, &parameter_config)->parameter1[0];  
	sg_fight_param_161000276 = get_config_by_id(161000276, &parameter_config)->parameter1[0];  
	sg_fight_param_161000277 = get_config_by_id(161000277, &parameter_config)->parameter1[0];  	
	sg_fight_param_161000278 = get_config_by_id(161000278, &parameter_config)->parameter1[0];  
	sg_fight_param_161000279 = get_config_by_id(161000279, &parameter_config)->parameter1[0];  
	sg_fight_param_161000280 = get_config_by_id(161000280, &parameter_config)->parameter1[0];  
	sg_fight_param_161000281 = get_config_by_id(161000281, &parameter_config)->parameter1[0];  
	sg_fight_param_161000282 = get_config_by_id(161000282, &parameter_config)->parameter1[0];  
	sg_fight_param_161000283 = get_config_by_id(161000283, &parameter_config)->parameter1[0];  
	sg_fight_param_161000284 = get_config_by_id(161000284, &parameter_config)->parameter1[0];  
	sg_fight_param_161000285 = get_config_by_id(161000285, &parameter_config)->parameter1[0];  
	sg_fight_param_161000286 = get_config_by_id(161000286, &parameter_config)->parameter1[0];  
	sg_fight_param_161000287 = get_config_by_id(161000287, &parameter_config)->parameter1[0];  
	sg_fight_param_161000288 = get_config_by_id(161000288, &parameter_config)->parameter1[0];  
	sg_fight_param_161000289 = get_config_by_id(161000289, &parameter_config)->parameter1[0];  	
	sg_fight_param_161000290 = get_config_by_id(161000290, &parameter_config)->parameter1[0];
	sg_fight_param_161000291 = get_config_by_id(161000291, &parameter_config)->parameter1[0];
	sg_fight_param_161000292 = get_config_by_id(161000292, &parameter_config)->parameter1[0];
	sg_fight_param_161000293 = get_config_by_id(161000293, &parameter_config)->parameter1[0];

	sg_doufachang_ai[0] = get_config_by_id(161000315, &parameter_config)->parameter1[0];
	sg_doufachang_ai[1] = get_config_by_id(161000315, &parameter_config)->parameter1[1];
	sg_doufachang_raid_id = get_config_by_id(161000316, &parameter_config)->parameter1[0];
	sg_doufachang_raid_win_reward[0] = get_config_by_id(161000320, &parameter_config)->parameter1[0];
	sg_doufachang_raid_win_reward[1] = get_config_by_id(161000320, &parameter_config)->parameter1[1];
	sg_doufachang_raid_lose_reward[0] = get_config_by_id(161000321, &parameter_config)->parameter1[0];
	sg_doufachang_raid_lose_reward[1] = get_config_by_id(161000321, &parameter_config)->parameter1[1];

	DEFAULT_HORSE = get_config_by_id(161000302, &parameter_config)->parameter1[0];

	config = get_config_by_id(161000308, &parameter_config);
	if (config && config->n_parameter1 >= 2)
	{
		sg_server_level_reward_item_id = config->parameter1[0];
		sg_server_level_reward_item_num = config->parameter1[1];
	}
	config = get_config_by_id(161000329, &parameter_config);
	if (config && config->n_parameter1 >= 1)
	{
		sg_exp_turn_zhenqi_percent = config->parameter1[0];
	}
	config = get_config_by_id(161000342, &parameter_config);
	if (config && config->n_parameter1 >= 1)
	{
		sg_on_shelf_fee_percent = config->parameter1[0];
	}
	sg_guild_ruqin_huodui_monster_id = get_config_by_id(161000357, &parameter_config)->parameter1[0];
	sg_guild_ruqin_huodui_fanwei = get_config_by_id(161000358, &parameter_config)->parameter1[0];
	sg_guild_ruqin_huodui_exp = get_config_by_id(161000359, &parameter_config)->parameter1[0];
	sg_guild_ruqin_huodui_jiange = get_config_by_id(161000360, &parameter_config)->parameter1[0];
	sg_guild_ruqin_huodui_chixutime = get_config_by_id(161000361, &parameter_config)->parameter1[0];
	sg_guild_ruqin_yaozu_bossid = get_config_by_id(161000348, &parameter_config)->parameter1[0];
	sg_guild_ruqin_renzu_bossid = get_config_by_id(161000348, &parameter_config)->parameter1[1];

	sg_maogui_diaoxiang_stop_buff = get_config_by_id(161000139, &parameter_config)->parameter1[0];
	sg_maogui_guiwang_wudi_buff = get_config_by_id(161000138, &parameter_config)->parameter1[0];
}

	// 读取刷怪配置
static int generate_create_monster_config(lua_State *L, struct sproto_type *type)
{
	char create_monster_config_name[PATH_MAX];
	std::map<uint64_t, struct SceneResTable *>::iterator iter;
	for (iter = scene_res_config.begin(); iter != scene_res_config.end(); ++iter)
	{
		struct SceneResTable *config = iter->second;

			//万妖谷主副本不需要刷怪
		struct DungeonTable *r_config = get_config_by_id(config->SceneID, &all_raid_config);
		if (r_config && r_config->DengeonRank == DUNGEON_TYPE_RAND_MASTER)
			continue;

		sprintf(create_monster_config_name, "../map_res/%s.dat", config->ResName);
		struct map_config *map_config = create_map_config(create_monster_config_name);
		if (!map_config)
			printf("%s: [%lu %s] read mapconfig %s fail\n", __FUNCTION__, config->SceneID, config->SceneName, create_monster_config_name);
		assert(map_config);
		scene_map_config[config->SceneID] = map_config;

		if (!config->PlaceNature || config->PlaceNature[0] == '\0')
		{
		}
		else
		{
			sprintf(create_monster_config_name, "../map_res/%s", config->PlaceNature);		
			struct region_config *region_config = create_region_config(create_monster_config_name);
			scene_region_config[config->SceneID] = region_config;
		}
		
		sprintf(create_monster_config_name, "../lua_data/%s.lua", config->RefreshPoint);
		struct stat stat_buf;
		if (stat(create_monster_config_name, &stat_buf) < 0)
		{
//			printf("%s: [%lu %s] read file %s fail\n", __FUNCTION__, config->sceneID, config->sceneName, create_monster_config_name);
			continue;
		}
		std::vector<struct SceneCreateMonsterTable *> *t = traverse_create_monster_table(L, type, create_monster_config_name);
		assert(t);
		all_scene_create_monster_config[config->SceneID] = t;
	}
	return (0);
}

bool is_wanyaogu_raid(uint32_t id)
{
	for (std::vector<uint32_t>::iterator ite = sg_vec_wanyaogu_raid_id.begin();
		 ite != sg_vec_wanyaogu_raid_id.end(); ++ite)
	{
		if (id == (*ite))
			return true;
	}
	return false;
}

int bagua_card_to_bind_item(uint32_t card_id)
{
	std::map<uint32_t, uint32_t>::iterator iter = sg_bagua_bind_item_map.find(card_id);
	if (iter != sg_bagua_bind_item_map.end())
	{
		return iter->second;
	}

	return 0;
}

bool is_guild_scene_id(uint32_t id)
{
	return (id == sg_guild_scene_id);
}

bool is_guild_wait_raid(uint32_t id)
{
	return (id == sg_guild_wait_raid_id);
}

bool is_guild_battle_raid(uint32_t id)
{
	return (id == (uint32_t)sg_guild_raid_param1[0] || id == (uint32_t)sg_guild_raid_final_param1[0]);
}

bool scene_can_make_team(uint32_t scene_id)
{
	if (get_scene_looks_type(scene_id) == SCENE_TYPE_WILD)
		return true;
	return false;
	
	// if (scene_id <= SCENCE_DEPART)
	// {
	// 	return true;
	// }

	// DungeonTable *table = get_config_by_id(scene_id, &all_raid_config);
	// if (table != NULL)
	// {
	// 	switch (table->DengeonRank)
	// 	{
	// 		case DUNGEON_TYPE_ZHENYING:
	// 		case DUNGEON_TYPE_GUILD_WAIT:
	// 		case DUNGEON_TYPE_GUILD_LAND:
	// 			return true;
	// 	} 
	// }

	// return false;
}

int get_scene_birth_pos(uint32_t scene_id, float *pos_x, float *pos_y, float *pos_z, float *face_y)
{
	SceneResTable *config = get_config_by_id(scene_id, &scene_res_config);
	if (!config)
	{
		return -1;
	}

	if (pos_x)
	{
		*pos_x = config->BirthPointX;
	}
	if (pos_y)
	{
		*pos_y = config->BirthPointY;
	}
	if (pos_z)
	{
		*pos_z = config->BirthPointZ;
	}
	if (face_y)
	{
		*face_y = config->FaceY;
	}

	return 0;
}


int add_all_scene()
{
	std::map<uint64_t, struct SceneResTable *>::iterator iter;
	for (iter = scene_res_config.begin(); iter != scene_res_config.end(); ++iter)
	{
		if (!is_raid_scene_id(iter->first) && !is_guild_scene_id(iter->first))
			scene_manager::add_scene(iter->first);
	}
	return (0);
}
/*
static void add_passive_skill(struct SkillTable *config)
{
	int index = 0;
	uint64_t id[10];
	std::map<uint64_t, struct PassiveSkillTable *>::iterator ite;
	for (ite = passive_skill_config.begin(); ite != passive_skill_config.end(); ++ite)
	{
		struct PassiveSkillTable *t_config = ite->second;
		for (size_t i = 0; i < t_config->n_SkillID; ++i)
		{
			if (t_config->SkillID[i] == config->ID)
			{
				id[index++] = t_config->ID;
			}
		}
	}
	if (index == 0)
		return;
	config->n_TriggerPassiveSkill = index;
	config->TriggerPassiveSkill = (uint64_t *)malloc(sizeof(uint64_t) * index);
	for (int i = 0; i < index; ++i)
	{
		config->TriggerPassiveSkill[i] = id[i];
	}
}
*/
static void adjust_skill_entry(struct SkillTable *config)
{
	if (config->RangeType == SKILL_RANGE_TYPE_FAN && fabsl(config->Angle - 180) <= __DBL_EPSILON__)
		config->RangeType = SKILL_RANGE_TYPE_CIRCLE;
	
	if (config->RangeType == SKILL_RANGE_TYPE_FAN)
	{
		config->Angle = config->Angle / 180 * M_PI;
	}

	std::map<uint64_t, struct SkillTable *>::iterator ite;	
	for (ite = skill_config.begin(); ite != skill_config.end(); ++ite)
	{
		struct SkillTable *t = ite->second;
		if (t->SkillType != 1 && t->SkillType != 2)
			continue;
		struct ActiveSkillTable *act_config = get_config_by_id(t->SkillAffectId, &active_skill_config);
		if (!act_config)
			continue;
		if (act_config->NextSkill == config->ID || act_config->AutoNextSkill == config->ID)
		{
			config->pre_skill = t->ID;
			return;
		}
	}

//	add_passive_skill(config);
}

static void adjust_skill_table()
{
	std::map<uint64_t, struct SkillTable *>::iterator ite;
	for (ite = skill_config.begin(); ite != skill_config.end(); ++ite)
	{
		struct SkillTable *config = ite->second;
		adjust_skill_entry(config);

		for (uint32_t i = 0; i < config->n_RuneID; ++i)
		{
			uint64_t fuwen_id = config->RuneID[i];
			if (fuwen_id == 0)
				continue;
			if (fuwen_config.find(fuwen_id) != fuwen_config.end())
			{
				LOG_ERR("%s: duplicate fuwen id %lu", __FUNCTION__, fuwen_id);
			}
			fuwen_config[fuwen_id] = config;
		}
	}
}

static void adjust_monster_talk_table()
{
	std::map<uint64_t, struct NpcTalkTable *>::iterator talk_ite;
	for (talk_ite = monster_talk_config.begin(); talk_ite != monster_talk_config.end(); ++talk_ite)
	{
		if (talk_ite->second->Type == 1)
			continue;
		if (talk_ite->second->n_EventNum2 < 1)
		{
			LOG_ERR("%s: %lu config wrong, ignore this config", __FUNCTION__, talk_ite->first);
			talk_ite->second->Type = 1;
		}
	}
}

static void adjust_monster_table()
{
	std::map<uint64_t, struct MonsterTable *>::iterator ite;
	std::map<uint64_t, struct NpcTalkTable *>::iterator talk_ite;	
	for (ite = monster_config.begin(); ite != monster_config.end(); ++ite)
	{
		struct MonsterTable *config = ite->second;
		for (talk_ite = monster_talk_config.begin(); talk_ite != monster_talk_config.end(); ++talk_ite)
		{
			if (talk_ite->second->Type == 1)
				continue;
			if (config->ID != talk_ite->second->NpcId)
				continue;
			struct NpcTalkTable *next = config->talk_config;
			talk_ite->second->next = next;
			config->talk_config = talk_ite->second;
		}
	}
}

static void adjust_scene_table()
{
	std::map<uint64_t, struct SceneResTable *>::iterator ite;
	for (ite = scene_res_config.begin(); ite != scene_res_config.end(); ++ite)
	{
		struct SceneResTable *config = ite->second;
		assert(config->n_RelivePointX == config->n_RelivePointZ);
		assert(config->n_RelivePointX == config->n_ReliveFaceY);
		assert(config->n_RelivePointX == config->n_ReliveRange);
	}
}

static void adjust_xunbao_table()
{
	std::map<uint64_t, struct SearchTable *>::iterator ite;
	for (ite = xunbao_config.begin(); ite != xunbao_config.end(); ++ite)
	{
		sg_xunbao.insert(std::make_pair(uint64_t(ite->second->ItemId), ite->second));
	}
}

static void adjust_xunbao_map_table()
{
	std::map<uint64_t, struct TreasureTable *>::iterator ite;
	for (ite = xunbao_map_config.begin(); ite != xunbao_map_config.end(); ++ite)
	{
		std::map<uint64_t, std::vector<uint64_t> >::iterator it = sg_xunbao_map.find(ite->second->MapId);
		if (it == sg_xunbao_map.end())
		{
			std::vector<uint64_t> tmp;
			tmp.push_back(ite->first);
			sg_xunbao_map.insert(std::make_pair(uint64_t(ite->second->MapId), tmp));
		}
		else
		{
			it->second.push_back(ite->first);
		}
	}
}

static void gen_show_collect()
{
	std::map<uint64_t, struct CollectTable *>::iterator ite;
	for (ite = collect_config.begin(); ite != collect_config.end(); ++ite)
	{
		for (uint32_t i = 0; i < ite->second->n_TaskIdShow; ++i)
		{
			sg_show_collect.insert(std::make_pair(ite->second->TaskIdShow[i], ite->first));
		}
	}
}

static void gen_question_arr()
{
	std::map<uint64_t, struct QuestionTable*>::iterator it = questions_config.begin();
	for (; it != questions_config.end(); ++it)
	{
		if (it->second->Type == 1)
		{
			sg_common_question.push_back(it->first);
		}
		else if (it->second->Type == 2)
		{
			sg_award_question.push_back(it->first);
		}
	}
}

uint32_t get_rand_question(std::vector<uint32_t> &vtQuestion)
{
	if (vtQuestion.size() == 0)
	{
		return 0;
	}
	return vtQuestion[rand() % vtQuestion.size()];
}

static void gen_random_monster_arr()
{
	std::map<uint64_t, struct RandomMonsterTable*>::iterator it = random_monster.begin();
	for (; it != random_monster.end(); ++it)
	{
		uint32_t comid = it->second->TypeLevel + it->second->Difficulty * 10;
		std::map<uint32_t, std::vector<RandomMonsterTable *> >::iterator itScecond = sg_random_monster_map.find(comid);
		if (itScecond == sg_random_monster_map.end())
		{
			std::vector<RandomMonsterTable *> tmp;
			tmp.push_back(it->second);
			sg_random_monster_map.insert(std::make_pair(comid, tmp));
		}
		else 
		{
			itScecond->second.push_back(it->second);
		}
	}
}

static void	adjust_robot_config(std::vector<struct ActorRobotTable*> *config)
{
//std::vector<struct ActorRobotTable*> robot_config[ROBOT_CONFIG_TYPE_SIZE]; //机器人
	for (size_t i = 0; i < config->size(); ++i)
	{
		struct ActorRobotTable *t = (*config)[i];
		int index = t->Type - 1;
		if (index >= ROBOT_CONFIG_TYPE_SIZE || index < 0)
		{
			LOG_ERR("%s: [%lu]type[%lu] wrong", __FUNCTION__, t->ID, t->Type);
			continue;
		}
		robot_config[index].push_back(t);
	}
}

static void adjust_zhenying_daily_truck()
{
	std::map<uint64_t, struct CampDefenseTable*>::iterator it = zhenying_daily_config.begin();
	for (int i = 0; it != zhenying_daily_config.end(); ++it,++i)
	{
		sg_zhenying_truck[i].n_TargetInfoList = it->second->n_TruckRouteX;
		TargetInfoEntry *pEntry = new struct TargetInfoEntry[it->second->n_TruckRouteX];
		sg_zhenying_truck[i].TargetInfoList = new struct TargetInfoEntry *[it->second->n_TruckRouteX];
		for (uint64_t p = 0; p < it->second->n_TruckRouteX; ++p)
		{
			sg_zhenying_truck[i].TargetInfoList[p] = &pEntry[p];
			pEntry[p].TargetPos = new struct TargetPos;
			pEntry[p].TargetPos->TargetPosX = it->second->TruckRouteX[p];
			pEntry[p].TargetPos->TargetPosZ = it->second->TruckRouteY[p];
			pEntry[p].RemainTime = 0;
		}
	}
}

static void gen_random_guoyu_fb_arr()
{
	std::map<uint64_t, struct RandomDungeonTable*>::iterator it = random_guoyu_dungenon_config.begin();
	for (; it != random_guoyu_dungenon_config.end(); ++it)
	{
		random_guoyu_fb_arr[it->second->TypeLevel].push_back(it->second);
	}
}
RandomDungeonTable * get_random_guoyu_fb_config(uint32_t type)
{
	if (type > MAX_GUOYU_TASK_TYPE)
	{
		return NULL;
	}
	int r = rand() % 10000;
	int all = 0;
	std::vector<RandomDungeonTable *>::iterator it = random_guoyu_fb_arr[type].begin();
	for (; it != random_guoyu_fb_arr[type].end(); ++it)
	{
		all += (*it)->ResProbability;
		if (r <= all)
		{
			return *it;
		}
	}
	return NULL;
}
void gen_yaoshi_skill_config()
{
	std::map<uint64_t, struct SpecialtySkillTable*>::iterator it = yaoshi_skill_config.begin();
	for (; it != yaoshi_skill_config.end(); ++it)
	{
		sg_yaoshi_skill_map.insert(std::make_pair(it->second->SkillEffect + it->second->SpecialtyLevel * 100, it->second));
		if (it->second->SpecialtyLevel > sg_yaoshi_level_limited[it->second->SkillEffect])
		{
			sg_yaoshi_level_limited[it->second->SkillEffect] = it->second->SpecialtyLevel;
		}
	}
}
SpecialtySkillTable* get_yaoshi_skill_config(int type, uint64_t level)
{
	if (level > sg_yaoshi_level_limited[type])
	{
		level = sg_yaoshi_level_limited[type];
	}
	std::map<uint32_t, struct SpecialtySkillTable*>::iterator it = sg_yaoshi_skill_map.find(type + level * 100);
	if (it == sg_yaoshi_skill_map.end())
	{
		return NULL;
	}
	return it->second;
}
RandomMonsterTable *get_random_monster_config(uint32_t type, uint64_t level)
{
	if (type > MAX_GUOYU_TASK_TYPE)
	{
		return NULL;
	}
	SpecialtySkillTable *killTable = get_yaoshi_skill_config(GUOYU_NINE, level);
	if (killTable == NULL)
	{
		return NULL;
	}
	uint32_t r = rand() % 10000;
	int star = 1;
	uint32_t all = 0;
	for (uint32_t i = 0; i < killTable->n_EffectValue; ++i)
	{
		all += killTable->EffectValue[i];
		if (r <= all)
		{
			star += i;
			break;
		}
	}
	uint32_t comid = type + star * 10;
	std::map<uint32_t, std::vector<RandomMonsterTable *> >::iterator itScecond = sg_random_monster_map.find(comid);
	if (itScecond == sg_random_monster_map.end())
	{
		return NULL;
	}
	if (itScecond->second.size() == 0)
	{
		return NULL;
	}
	r = rand() % itScecond->second.size();
	return itScecond->second.at(r);
}

WeekTable *get_rand_week_config()
{
	if (zhenying_week_config.size() > 0)
	{
		//return zhenying_week_config[360300003];
		return zhenying_week_config[360300001 + rand() % zhenying_week_config.size()];
	}

	return NULL;
}

bool check_active_open(uint32_t id, uint32_t &cd)
{
	uint64_t times = time_helper::get_micro_time();
	time_helper::set_cached_time(times / 1000);

	cd = 0;
	ControlTable *table = get_config_by_id(id, &all_control_config);
	if (table == NULL)
	{
		return false;
	}
	bool open = false;
	for (uint32_t i = 0; i < table->n_OpenDay; ++i)
	{
		if (time_helper::getWeek() == table->OpenDay[i])
		{
			open = true;
			break;
		}
	}
	if (!open)
	{
		return false;
	}
	open = false;
	struct tm tm;
	time_t tmp = time_helper::get_cached_time() / 1000;
	localtime_r(&tmp, &tm);
	for (uint32_t i = 0; i < table->n_OpenTime; ++i)
	{
		tm.tm_hour = table->OpenTime[i] / 100;
		tm.tm_min = table->OpenTime[i] % 100;
		tm.tm_sec = 0;
		uint64_t st = mktime(&tm);
		tm.tm_hour = table->CloseTime[i] / 100;
		tm.tm_min = table->CloseTime[i] % 100;
		tm.tm_sec = 59;
		uint64_t end = mktime(&tm);
		if (time_helper::get_cached_time() / 1000 >= st && time_helper::get_cached_time() / 1000 <= end)
		{
			open = true;
			cd = end - time_helper::get_cached_time() / 1000;
			break;
		}
	}
	if (!open)
	{
		cd = 0;
		return false;
	}
	return true;
}

bool task_is_team(uint32_t task_id)
{
	TaskTable *main_config = get_config_by_id(task_id, &task_config);
	if (main_config)
	{
		return (main_config->Team == 1);
	}

	return false;
}

bool escort_is_team(uint32_t escort_id)
{
	EscortTask *main_config = get_config_by_id(escort_id, &escort_config);
	if (main_config)
	{
		return (main_config->Team == 1);
	}

	return false;
}

uint64_t get_task_chapter_id(uint32_t task_id)
{
	TaskTable *config = get_config_by_id(task_id, &task_config);
	if (config)
	{
		return config->ChapterId;
	}

	return 0;
}

TaskChapterTable *get_task_chapter_config(uint32_t chapter_id)
{
	return get_config_by_id(chapter_id, &task_chapter_map);
}

void get_task_reward_item_from_config(uint32_t reward_id, std::map<uint32_t, uint32_t> &item_list)
{
	TaskRewardTable *reward_config = get_config_by_id(reward_id, &task_reward_config);
	if (!reward_config)
	{
		return;
	}

	for (uint32_t j = 0; j < reward_config->n_RewardType && j < reward_config->n_RewardTarget
			&& j < reward_config->n_RewardNum; ++j)
	{
		if (reward_config->RewardType[j] == TRT_ITEM)
		{
			item_list[reward_config->RewardTarget[j]] += reward_config->RewardNum[j];
		}
	}
}

static void adjust_lv_skill_entry(struct SkillLvTable *config)
{
	if (config->n_BuffIdEnemy == 1 && config->BuffIdEnemy[0] == 0)
		config->n_BuffIdEnemy = 0;
	if (config->n_BuffIdFriend == 1 && config->BuffIdFriend[0] == 0)
		config->n_BuffIdFriend = 0;
	
	if (config->n_EffectIdEnemy == 1 && config->EffectIdEnemy[0] == 0)
		config->n_EffectIdEnemy = 0;
	if (config->n_EffectIdFriend == 1 && config->EffectIdFriend[0] == 0)
		config->n_EffectIdFriend = 0;	
}
static void adjust_lv_skill_table()
{
	std::map<uint64_t, struct SkillLvTable *>::iterator ite;
	for (ite = skill_lv_config.begin(); ite != skill_lv_config.end(); ++ite)
	{
		struct SkillLvTable *config = ite->second;
		adjust_lv_skill_entry(config);
	}
}

static void adjust_baseai_table()
{
	// std::map<uint64_t, struct BaseAITable *>::iterator ite;
	// for (ite = base_ai_config.begin(); ite != base_ai_config.end(); ++ite)
	// {
	// 	struct BaseAITable *config = ite->second;
	// 	config->StopMax *= 1000;
	// 	config->StopMin *= 1000;		
	// }	
}

static bool robot_patrol_cmp_func(const struct RobotPatrolTable *l, const struct RobotPatrolTable *r)
{
	if (l->ID <= r->ID)
	{
		return true;
	}
	return false;
}

static bool script_cmp_func(const struct RaidScriptTable *l, const struct RaidScriptTable *r)
{
	if (l->ID <= r->ID)
	{
		return true;
	}
	return false;
}

static bool raid_ai_monster_cmp_func(const struct SceneCreateMonsterTable *l, const struct SceneCreateMonsterTable *r)
{
	if (l->uid <= r->uid)
	{
		return true;
	}
	return false;
}

static void	adjust_robotpatrol_entry(struct RobotPatrolTable *config, uint64_t *data)
{
	config->patrol[config->n_patrol]->pos_x = data[0];
	config->patrol[config->n_patrol]->pos_z = data[1];	
//		(*ite)->patrol[(*ite)->n_patrol]->pos_x = (*ite)->patrol1[0];
//		(*ite)->patrol[(*ite)->n_patrol]->pos_z = (*ite)->patrol1[1];		
}

static void adjust_robotzhenyingzhan_table()
{
	for (std::vector<struct RobotPatrolTable*>::iterator ite = robot_zhenyingzhan_config.begin();
		 ite != robot_zhenyingzhan_config.end(); ++ite)
	{
		struct sproto_config_pos *t = (struct sproto_config_pos *)malloc(22 * sizeof(struct sproto_config_pos));
		(*ite)->patrol = (struct sproto_config_pos **)malloc(22 * sizeof(void *));
		for (size_t i = 0; i < 22; ++i)
		{
			(*ite)->patrol[i] = &t[i];
		}		
		
		if ((*ite)->n_patrol1 != 2)
		{
				//至少要一个巡逻点
			assert(0);
			continue;
		}
		adjust_robotpatrol_entry(*ite, (*ite)->patrol1);			
		(*ite)->n_patrol += 1;
		
		if ((*ite)->n_patrol2 != 2)
		{
			continue;
		}
		adjust_robotpatrol_entry(*ite, (*ite)->patrol2);
		(*ite)->n_patrol += 1;
		
		if ((*ite)->n_patrol3 != 2)
		{
			continue;
		}
		adjust_robotpatrol_entry(*ite, (*ite)->patrol3);
		(*ite)->n_patrol += 1;

		if ((*ite)->n_patrol4 != 2)
		{
			continue;
		}
		adjust_robotpatrol_entry(*ite, (*ite)->patrol4);
		(*ite)->n_patrol += 1;

		if ((*ite)->n_patrol5 != 2)
		{
			continue;
		}
		adjust_robotpatrol_entry(*ite, (*ite)->patrol5);
		(*ite)->n_patrol += 1;

		if ((*ite)->n_patrol6 != 2)
		{
			continue;
		}
		adjust_robotpatrol_entry(*ite, (*ite)->patrol6);
		(*ite)->n_patrol += 1;

		if ((*ite)->n_patrol7 != 2)
		{
			continue;
		}
		adjust_robotpatrol_entry(*ite, (*ite)->patrol7);
		(*ite)->n_patrol += 1;

		if ((*ite)->n_patrol8 != 2)
		{
			continue;
		}
		adjust_robotpatrol_entry(*ite, (*ite)->patrol8);
		(*ite)->n_patrol += 1;

		if ((*ite)->n_patrol9 != 2)
		{
			continue;
		}
		adjust_robotpatrol_entry(*ite, (*ite)->patrol9);
		(*ite)->n_patrol += 1;

		if ((*ite)->n_patrol10 != 2)
		{
			continue;
		}
		adjust_robotpatrol_entry(*ite, (*ite)->patrol10);
		(*ite)->n_patrol += 1;

		if ((*ite)->n_patrol11 != 2)
		{
			continue;
		}
		adjust_robotpatrol_entry(*ite, (*ite)->patrol11);
		(*ite)->n_patrol += 1;

		if ((*ite)->n_patrol12 != 2)
		{
			continue;
		}
		adjust_robotpatrol_entry(*ite, (*ite)->patrol12);
		(*ite)->n_patrol += 1;
	}
	std::sort(robot_zhenyingzhan_config.begin(), robot_zhenyingzhan_config.end(), robot_patrol_cmp_func);			
}

static void adjust_robotpatrol_table()
{
	for (std::vector<struct RobotPatrolTable*>::iterator ite = robot_patrol_config.begin();
		 ite != robot_patrol_config.end(); ++ite)
	{
			//去掉两头的端点，形成环，12 + 10 = 22
		struct sproto_config_pos *t = (struct sproto_config_pos *)malloc(22 * sizeof(struct sproto_config_pos));
		(*ite)->patrol = (struct sproto_config_pos **)malloc(22 * sizeof(void *));
		for (size_t i = 0; i < 22; ++i)
		{
			(*ite)->patrol[i] = &t[i];
		}		
		
		if ((*ite)->n_patrol1 != 2)
		{
				//至少要一个巡逻点
			assert(0);
			continue;
		}
		adjust_robotpatrol_entry(*ite, (*ite)->patrol1);			
		(*ite)->n_patrol += 1;
		
		if ((*ite)->n_patrol2 != 2)
		{
			continue;
		}
		adjust_robotpatrol_entry(*ite, (*ite)->patrol2);
		(*ite)->n_patrol += 1;
		
		if ((*ite)->n_patrol3 != 2)
		{
			continue;
		}
		adjust_robotpatrol_entry(*ite, (*ite)->patrol3);
		(*ite)->n_patrol += 1;

		if ((*ite)->n_patrol4 != 2)
		{
			continue;
		}
		adjust_robotpatrol_entry(*ite, (*ite)->patrol4);
		(*ite)->n_patrol += 1;

		if ((*ite)->n_patrol5 != 2)
		{
			continue;
		}
		adjust_robotpatrol_entry(*ite, (*ite)->patrol5);
		(*ite)->n_patrol += 1;

		if ((*ite)->n_patrol6 != 2)
		{
			continue;
		}
		adjust_robotpatrol_entry(*ite, (*ite)->patrol6);
		(*ite)->n_patrol += 1;

		if ((*ite)->n_patrol7 != 2)
		{
			continue;
		}
		adjust_robotpatrol_entry(*ite, (*ite)->patrol7);
		(*ite)->n_patrol += 1;

		if ((*ite)->n_patrol8 != 2)
		{
			continue;
		}
		adjust_robotpatrol_entry(*ite, (*ite)->patrol8);
		(*ite)->n_patrol += 1;

		if ((*ite)->n_patrol9 != 2)
		{
			continue;
		}
		adjust_robotpatrol_entry(*ite, (*ite)->patrol9);
		(*ite)->n_patrol += 1;

		if ((*ite)->n_patrol10 != 2)
		{
			continue;
		}
		adjust_robotpatrol_entry(*ite, (*ite)->patrol10);
		(*ite)->n_patrol += 1;

		if ((*ite)->n_patrol11 != 2)
		{
			continue;
		}
		adjust_robotpatrol_entry(*ite, (*ite)->patrol11);
		(*ite)->n_patrol += 1;

		if ((*ite)->n_patrol12 != 2)
		{
			continue;
		}
		adjust_robotpatrol_entry(*ite, (*ite)->patrol12);
		(*ite)->n_patrol += 1;
	}

	for (std::vector<struct RobotPatrolTable*>::iterator ite = robot_patrol_config.begin();
		 ite != robot_patrol_config.end(); ++ite)
	{
		int max_num = (*ite)->n_patrol;
		int add_num = (*ite)->n_patrol - 2;
		if (add_num <= 0)
			continue;

		for (int i = 0; i < add_num; ++i)
		{
			int t1 = max_num + i;
			int t2 = max_num - i - 2;
			assert(t2 >= 0);
			(*ite)->patrol[t1]->pos_x = (*ite)->patrol[t2]->pos_x;
			(*ite)->patrol[t1]->pos_z = (*ite)->patrol[t2]->pos_z;			
		}
		(*ite)->n_patrol += (add_num);
		
		// int index = (*ite)->n_patrol - 2;
		// for (size_t i = 1; i < (*ite)->n_patrol - 1; ++i)
		// {
		// 	assert(index > 0);
		// 	(*ite)->patrol[i]->pos_x = (*ite)->patrol[index]->pos_x;
		// 	(*ite)->patrol[i]->pos_z = (*ite)->patrol[index]->pos_z;
		// 	--index;
		// }
		// if ((*ite)->n_patrol > 2)
		// 	(*ite)->n_patrol += ((*ite)->n_patrol - 2);
	}
	
	std::sort(robot_patrol_config.begin(), robot_patrol_config.end(), robot_patrol_cmp_func);		
}

static void	add_wanyaoka_id(struct DungeonTable *config)
{
	uint64_t id[32];
	int index = 0;
	int max_cond_param = 0;
	for (std::map<uint64_t, struct RandomCardTable*>::iterator iter = wanyaoka_config.begin(); iter != wanyaoka_config.end(); ++iter)
	{
		if (iter->second->CardDengeon == config->DungeonID)
		{
			id[index++] = iter->second->CardID;
			max_cond_param += iter->second->n_Condition;
			assert(iter->second->n_Condition == iter->second->n_Parameter1);
			assert(iter->second->n_Condition == iter->second->n_Parameter2);			
		}
	}
	if (index == 0)
	{
		return;
	}
	config->n_wanyaoka = index;
	config->wanyaoka = (uint64_t *)malloc(sizeof(uint64_t) * index);
	assert(config->wanyaoka);
	memcpy(config->wanyaoka, &id[0], sizeof(uint64_t) * index);
	assert(max_cond_param <= MAX_WANYAOKA_COND_PARAM);
}

static void add_raid_ai_monster_config(lua_State *L, struct sproto_type *type)
{
	for(std::set<char*>::iterator itr = some_monster_config_name.begin(); itr != some_monster_config_name.end(); itr++)
	{
		char* config_name = *itr;
		if(config_name == NULL)
			continue;

		std::vector<struct SceneCreateMonsterTable *> *monster_config = get_config_by_name(config_name, &all_raid_ai_monster_config);
		if(monster_config)
		{
			return;
		}
		char full_name[255];
		sprintf(full_name, "../lua_data/%s.lua", config_name);
		monster_config = new std::vector<struct SceneCreateMonsterTable *>;
		int ret = traverse_vector_table(L, type, full_name, (std::vector<void *> *)monster_config);
		std::sort(monster_config->begin(), monster_config->end(), raid_ai_monster_cmp_func);
		assert(ret == 0);
		all_raid_ai_monster_config[config_name] = monster_config;
	}
}
static void add_raid_script_config(lua_State *L, struct sproto_type *type, struct DungeonTable *config, char *config_name)
{
	assert(config_name);
	std::vector<struct RaidScriptTable *> *script_config = get_config_by_name(config_name, &all_raid_script_config);
	if (script_config)
	{
		return;
	}

	char full_name[255];
	sprintf(full_name, "../lua_data/%s.lua", config_name);
	script_config = new std::vector<struct RaidScriptTable *>;
	int ret = traverse_vector_table(L, type, full_name, (std::vector<void *> *)script_config);
	std::sort(script_config->begin(), script_config->end(), script_cmp_func);
	assert(ret == 0);
	all_raid_script_config[config_name] = script_config;

	//将跳表的配置读进来
	for (std::vector<struct RaidScriptTable *>::iterator iter = script_config->begin(); iter != script_config->end(); ++iter)
	{
		RaidScriptTable *tmp_config = *iter;
		switch (tmp_config->TypeID)
		{
			case 26:
			case 30:
				break;
			case 53:
				for (uint32_t i = 0; i < tmp_config->n_Parameter2; ++i)
				{
					some_monster_config_name.insert(tmp_config->Parameter2[i]);
				}
				continue;
			default:
				continue;
		}

		for (uint32_t i = 0; i < tmp_config->n_Parameter2; ++i)
		{
			add_raid_script_config(L, type, config, tmp_config->Parameter2[i]);
		}
	}
}

static void adjust_dungeon_table(lua_State *L, struct sproto_type *type)
{
	for (std::map<uint64_t, struct DungeonTable*>::iterator iter = all_raid_config.begin(); iter != all_raid_config.end(); ++iter)
	{
		struct DungeonTable *config = iter->second;
		assert(config->n_Score == config->n_ScoreValue);
		assert(config->n_Score == config->n_ScoreValue1);		
		if (config->DengeonRank == DUNGEON_TYPE_RAND_MASTER)
		{
			if (config->n_RandomID < config->RandomNum || config->RandomNum == 0)
			{
				LOG_ERR("%s: raid %lu", __FUNCTION__, config->DungeonID);
				assert(0);
			}
			sg_vec_wanyaogu_raid_id.push_back(config->DungeonID);
		}
		else if (config->DengeonRank == DUNGEON_TYPE_RAND_SLAVE)
		{
			add_wanyaoka_id(config);
			if (config->DungeonPass[0] != '\0')
				add_raid_script_config(L, type, config, config->DungeonPass);			
		}
		else if (config->DengeonRank == DUNGEON_TYPE_SCRIPT || config->DengeonRank ==DUNGEON_TYPE_MAOGUI_LEYUAN)
		{
			add_raid_script_config(L, type, config, config->DungeonPass);
		}
		// else if (config->DengeonRank == DUNGEON_TYPE_GUILD_WAIT)
		// {
		// 	config->DengeonType = 1;
		// }
	}
}

static void add_guild_raid_ai_config(lua_State *L, struct sproto_type *type)
{
	for(std::map<uint64_t, struct FactionActivity*>::iterator itr = guild_activ_config.begin(); itr != guild_activ_config.end(); itr++)
	{
		if(itr->second->DungeonPass1)
		{
			add_raid_script_config(L, type, NULL, itr->second->DungeonPass1);
		}
		if(itr->second->DungeonPass2)
		{
			add_raid_script_config(L, type, NULL, itr->second->DungeonPass2);
		}
	}

}
static void adjust_task_tables(void)
{
	sg_first_trunk_task_id = 0;
	for (std::map<uint64_t, struct TaskTable *>::iterator iter = task_config.begin(); iter != task_config.end(); ++iter)
	{
		TaskTable *config = iter->second;

		bool has_pre_task = false;
		for (uint32_t i = 0; i < config->n_AccessConID; ++i)
		{
			TaskConditionTable *cond_config = get_config_by_id(config->AccessConID[i], &task_condition_config);
			if (!cond_config)
			{
				continue;
			}

			if (cond_config->ConditionType == TCT_ACCEPT)
			{
				TaskTable *pre_config = get_config_by_id(cond_config->ConditionTarget, &task_config);
				if (pre_config)
				{
					pre_config->n_PostTask++;
					pre_config->PostTask = (uint64_t*)realloc(pre_config->PostTask, pre_config->n_PostTask * sizeof(uint64_t));
					pre_config->PostTask[pre_config->n_PostTask - 1] = config->ID;
				}
				has_pre_task = true;
			}
		}

		if (sg_first_trunk_task_id == 0 && config->TaskType == TT_TRUNK && !has_pre_task)
		{
			sg_first_trunk_task_id = config->ID;
		}
	}

	task_chapter_map.clear();
	for (std::map<uint64_t, struct TaskChapterTable *>::iterator iter = task_chapter_config.begin(); iter != task_chapter_config.end(); ++iter)
	{
		TaskChapterTable *config = iter->second;
		task_chapter_map[config->ChapterID] = config;
	}
}

static void adjust_equip_lock_table(void)
{
	for (std::map<uint64_t, EquipLock *>::iterator iter = equip_lock_config.begin(); iter != equip_lock_config.end(); ++iter)
	{
		EquipLock *config = iter->second;
		assert(config->n_LockLv == MAX_EQUIP_INLAY_NUM);
		assert(config->n_LockQuality == MAX_EQUIP_INLAY_NUM);
		assert(config->n_LockStar == MAX_EQUIP_INLAY_NUM);
		assert(config->n_LockItem == MAX_EQUIP_INLAY_NUM);
		assert(config->n_LockItemNum == MAX_EQUIP_INLAY_NUM);
		assert(config->n_MosaicType == MAX_EQUIP_INLAY_NUM);
		assert(config->n_EnchantQualityLock == MAX_EQUIP_ENCHANT_NUM);
		assert(config->n_EnchantStarLock == MAX_EQUIP_ENCHANT_NUM);
		assert(config->n_EnchantItem == MAX_EQUIP_ENCHANT_NUM);
		assert(config->n_EnchantItemNum == MAX_EQUIP_ENCHANT_NUM);
		assert(config->n_EnchantCoin == MAX_EQUIP_ENCHANT_NUM);
	}
}

static void adjust_fighting_capacity_coefficient(void)
{
	for (int i = 1; i < PLAYER_ATTR_FIGHT_MAX; ++i)
	{
		AttributeTypeTable *config = get_config_by_id(i, &attribute_type_config);
		if (!config)
		{
			continue;
		}

		sg_fighting_capacity_coefficient[i] = config->FightRatio;
	}
}

static void adjust_stage_tables()
{
	uint64_t total = 0;
	uint64_t last = 0;
	for (std::map<uint64_t, struct StageTable*>::iterator ite = pvp_raid_config.begin();
		 ite != pvp_raid_config.end(); ++ite)
	{
		struct StageTable *config = ite->second;
		total += config->StageScore;
		config->stageLastScore = last;
		config->stageTotalScore = total;
		last = total;
	}
}

static void adjust_yuqidao_tables(void)
{
	for (std::map<uint64_t, struct AcupunctureTable*>::iterator iter = yuqidao_acupoint_config.begin(); iter != yuqidao_acupoint_config.end(); ++iter)
	{
		AcupunctureTable *config = iter->second;
		assert(config->n_AcupunctureAttribute == config->n_AttributeCeiling);
	}

	for (std::map<uint64_t, struct BreakTable*>::iterator iter = yuqidao_break_config.begin(); iter != yuqidao_break_config.end(); ++iter)
	{
		BreakTable *config = iter->second;
		assert(config->n_PulseAttribute == MAX_YUQIDAO_BREAK_ATTR_NUM);
		assert(config->n_AttributeLower == MAX_YUQIDAO_BREAK_ATTR_NUM);
		assert(config->n_AttributeUpper == MAX_YUQIDAO_BREAK_ATTR_NUM);
		assert(config->n_AttributeColor == config->n_MeridiansProbability);
		assert(config->n_AttributeColor == config->n_BloodProbability);
		assert(config->n_AttributeColor == config->n_VitalProbability);
		assert(config->n_AttributeColor == config->n_MarrowProbability);
		assert(config->n_AttributeColor == config->n_MeridiansMinimum);
		assert(config->n_AttributeColor == config->n_BloodMinimum);
		assert(config->n_AttributeColor == config->n_VitalMinimum);
		assert(config->n_AttributeColor == config->n_MarrowMinimum);
	}
}

static void adjust_pktype_table()
{
//	static int pk_type_to_fight_type[21][21];
//	UNUSED(pk_type_to_fight_type);
	assert(pk_type_config.size() == 21);
	std::map<uint64_t, struct MonsterPkTypeTable *>::iterator ite;
	for (ite = pk_type_config.begin(); ite != pk_type_config.end(); ++ite)
	{
		int j = 0;
		struct MonsterPkTypeTable *config = ite->second;
		assert(config->ID < 21);
		pk_type_to_fight_type[config->ID][j++] = (UNIT_FIGHT_TYPE)(ite->second->PkType0);
		pk_type_to_fight_type[config->ID][j++] = (UNIT_FIGHT_TYPE)(ite->second->PkType1);
		pk_type_to_fight_type[config->ID][j++] = (UNIT_FIGHT_TYPE)(ite->second->PkType2);
		pk_type_to_fight_type[config->ID][j++] = (UNIT_FIGHT_TYPE)(ite->second->PkType3);
		pk_type_to_fight_type[config->ID][j++] = (UNIT_FIGHT_TYPE)(ite->second->PkType4);
		pk_type_to_fight_type[config->ID][j++] = (UNIT_FIGHT_TYPE)(ite->second->PkType5);
		pk_type_to_fight_type[config->ID][j++] = (UNIT_FIGHT_TYPE)(ite->second->PkType6);
		pk_type_to_fight_type[config->ID][j++] = (UNIT_FIGHT_TYPE)(ite->second->PkType7);
		pk_type_to_fight_type[config->ID][j++] = (UNIT_FIGHT_TYPE)(ite->second->PkType8);
		pk_type_to_fight_type[config->ID][j++] = (UNIT_FIGHT_TYPE)(ite->second->PkType9);
		pk_type_to_fight_type[config->ID][j++] = (UNIT_FIGHT_TYPE)(ite->second->PkType10);
		pk_type_to_fight_type[config->ID][j++] = (UNIT_FIGHT_TYPE)(ite->second->PkType11);
		pk_type_to_fight_type[config->ID][j++] = (UNIT_FIGHT_TYPE)(ite->second->PkType12);
		pk_type_to_fight_type[config->ID][j++] = (UNIT_FIGHT_TYPE)(ite->second->PkType13);
		pk_type_to_fight_type[config->ID][j++] = (UNIT_FIGHT_TYPE)(ite->second->PkType14);
		pk_type_to_fight_type[config->ID][j++] = (UNIT_FIGHT_TYPE)(ite->second->PkType15);
		pk_type_to_fight_type[config->ID][j++] = (UNIT_FIGHT_TYPE)(ite->second->PkType16);
		pk_type_to_fight_type[config->ID][j++] = (UNIT_FIGHT_TYPE)(ite->second->PkType17);
		pk_type_to_fight_type[config->ID][j++] = (UNIT_FIGHT_TYPE)(ite->second->PkType18);
		pk_type_to_fight_type[config->ID][j++] = (UNIT_FIGHT_TYPE)(ite->second->PkType19);
		pk_type_to_fight_type[config->ID][j++] = (UNIT_FIGHT_TYPE)(ite->second->PkType20);				
	}
}

static void adjust_active_skill_tables(void)
{
	for (std::map<uint64_t, struct ActiveSkillTable*>::iterator iter = active_skill_config.begin(); iter != active_skill_config.end(); ++iter)
	{
		struct ActiveSkillTable *config = iter->second;
		for (size_t i = 0; i < config->n_SkillLength; ++i)
			config->TotalSkillDelay += config->SkillLength[i];

		if (config->TotalSkillDelay <= config->ActionTime)
			config->TotalSkillDelay = 0;
		else
			config->TotalSkillDelay -= config->ActionTime;
	}
}

static void adjust_baguapai_tables(void)
{
	for (std::map<uint64_t, struct BaguaTable*>::iterator iter = bagua_config.begin(); iter != bagua_config.end(); ++iter)
	{
		BaguaTable *config = iter->second;
		assert(config->n_DecomposeItem == config->n_DecomposeNum);
		assert(config->n_RecastItem == config->n_RecastNum);
		assert(config->n_ClearItem == config->n_ClearNum);
	}

	for (std::map<uint64_t, struct BaguaStarTable*>::iterator iter = bagua_star_config.begin(); iter != bagua_star_config.end(); ++iter)
	{
		BaguaStarTable *config = iter->second;
		assert(config->n_StarItem == config->n_StarNum);
		assert(config->n_DecomposeCompensation == config->n_DecomposeCompensationNum);
	}

	for (std::map<uint64_t, struct BaguaSuitTable*>::iterator iter = bagua_suit_config.begin(); iter != bagua_suit_config.end(); ++iter)
	{
		BaguaSuitTable *config = iter->second;
		assert(config->n_SuitNum == config->n_SuitAttributeType);
		assert(config->n_SuitNum == config->n_Classification);
		assert(config->n_SuitNum == config->n_AttributeValue);
		assert(config->n_SuitNum == config->n_SuitPlus1);
		assert(config->n_SuitNum == config->n_SuitPlus2);
		assert(config->n_SuitNum == config->n_SuitPlus3);
		assert(config->n_SuitNum == config->n_SuitPlus4);
		assert(config->n_SuitNum == config->n_SuitPlus5);
		assert(config->n_SuitNum == config->n_SuitPlus6);
	}
}

static void generate_item_relative_info(void)
{
	for (std::map<uint64_t, struct ItemsConfigTable*>::iterator iter = item_config.begin(); iter != item_config.end(); ++iter)
	{
		ItemsConfigTable *config = iter->second;
		if (config->ItemType == 10 && config->BindType == 1 && config->n_ParameterEffect > 0)
		{
			sg_bagua_bind_item_map[config->ParameterEffect[0]] = config->ID;
		}
		if (iter->second->ItemEffect == 16)
		{
			sg_partner_item_map[iter->second->ParameterEffect[0]] = iter->first;
		}
	}
}

//std::map<uint64_t, struct GenerateMonster*> GenerateMonster_config;   //定时刷怪配置
static void	adjust_generatemonster_config()
{
	std::map<uint64_t, struct GenerateMonster*> tmp = GenerateMonster_config;
	GenerateMonster_config.clear();
	for (std::map<uint64_t, struct GenerateMonster*>::iterator ite = tmp.begin(); ite != tmp.end(); ++ite)
	{
		struct GenerateMonster* t = ite->second;
		assert(t->n_MovePointXZ == 2);
		GenerateMonster_config[ite->second->MonsterPointID] = t;
	}
}

static void adjust_escort_config(void)
{
	for (std::map<uint64_t, struct EscortTask*>::iterator iter = escort_config.begin(); iter != escort_config.end(); ++iter)
	{
		EscortTask *config = iter->second;
		std::map<uint64_t, std::vector<struct SceneCreateMonsterTable *> *>::iterator scene_iter = all_scene_create_monster_config.find(config->Scene);
		assert(scene_iter != all_scene_create_monster_config.end());
		bool find = false;
		for (size_t i = 0; i < scene_iter->second->size(); ++i)
		{
			if (scene_iter->second->at(i)->ID == config->NpcID)
			{
				config->MonsterIndex = i;
				find = true;
				break;
			}
		}
		assert(find);
	}
}

static void adjust_jijiangopen_table()
{
	std::map<uint64_t, struct FunctionUnlockTable *>::iterator ite;
	for (ite = function_unlock_config.begin(); ite != function_unlock_config.end(); ++ite)
	{
		if (ite->second->IsSoon == 0)
			continue;
		sg_jijiangopen.insert(std::make_pair((uint64_t)(ite->second->IsSoon), ite->second));
	}
}

static void adjust_worldboss_table()
{
	std::map<uint64_t, struct WorldBossTable *>::iterator ite;
	for (ite = world_boss_config.begin(); ite != world_boss_config.end(); ++ite)
	{
		monster_to_world_boss_config.insert(std::make_pair((uint64_t)(ite->second->MonsterID), ite->second));
	}
}
static void adjust_herochallenge_table()
{
	std::map<uint64_t, struct ChallengeTable *>::iterator ite;
	for (ite = hero_challenge_config.begin(); ite != hero_challenge_config.end(); ++ite)
	{
		raidid_to_hero_challenge_config.insert(std::make_pair((uint64_t)(ite->second->DungeonID), ite->second));
	}
}

static void adjust_mijingxiulian_table()
{
	std::map<uint64_t, struct UndergroundTask *>::iterator ite;
	for (ite = mijing_xiulian_config.begin(); ite != mijing_xiulian_config.end(); ++ite)
	{
		taskid_to_mijing_xiulian_config.insert(std::make_pair((uint64_t)(ite->second->TaskID), ite->second));
	}
}

static void adjust_achievement_config(void)
{
	std::map<uint64_t, AchievementFunctionTable*> func_hier_map;
	for (std::map<uint64_t, AchievementFunctionTable*>::iterator iter = achievement_function_config.begin(); iter != achievement_function_config.end(); ++iter)
	{
		func_hier_map[iter->second->HierarchyID] = iter->second;
	}

	for (std::map<uint64_t, AchievementHierarchyTable*>::iterator iter = achievement_hierarchy_config.begin(); iter != achievement_hierarchy_config.end(); ++iter)
	{
		AchievementHierarchyTable *config = iter->second;
		std::map<uint64_t, AchievementFunctionTable*>::iterator iter_func = func_hier_map.find(config->HierarchyID);
		if (iter_func != func_hier_map.end())
		{
			AchievementFunctionTable* func_config = iter_func->second;
			func_config->n_Hierarchys++;
			func_config->Hierarchys = (uint64_t*)realloc(func_config->Hierarchys, sizeof(uint64_t) * func_config->n_Hierarchys);
			func_config->Hierarchys[func_config->n_Hierarchys - 1] = iter->first;
		}
	}
}

ActorTable *get_actor_config(uint32_t job)
{
	uint64_t comb_id = 101 * 1e6 + job;
	std::map<uint64_t, ActorTable*>::iterator iter = actor_config.find(comb_id);
	if (iter != actor_config.end())
	{
		return iter->second; 
	}

	return NULL;
}


ActorLevelTable *get_actor_level_config(uint32_t job, uint32_t level)
{
	uint64_t comb_id = 104 * 1e6 + job * 1e5 + level;
	std::map<uint64_t, ActorLevelTable *>::iterator iter = actor_level_config.find(comb_id);
	if (iter != actor_level_config.end())
	{
		return iter->second; 
	}

	return NULL;
}

SpecialtyLevelTable *get_specialty_level_table(int type, int level)
{
	uint64_t comb_id = 321 * 1000000 + type * 1000 + level;
	std::map<uint64_t, SpecialtyLevelTable *>::iterator iter = specialty_level_config.find(comb_id);
	if (iter != specialty_level_config.end())
	{
		return iter->second;
	}
	return NULL;
}
SpecialTitleTable *get_yaoshi_title_table(int type, int level)
{
	SpecialtyLevelTable * table = get_specialty_level_table(type, level);
	if (table == NULL)
	{
		return NULL;
	}
	std::map<uint64_t, SpecialTitleTable *>::iterator iter = yaoshi_title_config.find(table->SpecialTitle);
	if (iter != yaoshi_title_config.end())
	{
		return iter->second;
	}
	return NULL;
}

FactionBattleTable *get_zhenying_battle_table(uint32_t level)
{
	std::vector<struct FactionBattleTable*>::iterator it = zhenying_battle_config.begin();
	for (; it != zhenying_battle_config.end(); ++it)
	{
		if ((*it)->LowerLimitLv <= level && (*it)->UpperLimitLv >= level)
		{
			return *it;
		}
	}
	return NULL;
}

LifeSkillTable *get_medicine_table(uint32_t type, uint32_t lv)
{
	uint64_t comb_id = 410200000 + type * 1000 + lv;
	std::map<uint64_t, LifeSkillTable *>::iterator iter = medicine_config.find(comb_id);
	if (iter != medicine_config.end())
	{
		return iter->second;
	}
	return NULL;
}

TypeLevelTable *get_guoyu_level_table(int level)
{
	uint64_t comb_id = 324000000 + level;
	std::map<uint64_t, TypeLevelTable *>::iterator iter = guoyu_level_config.find(comb_id);
	if (iter != guoyu_level_config.end())
	{
		return iter->second;
	}
	return NULL;
}

SceneCreateMonsterTable *get_daily_zhenying_truck_config(uint32_t id)
{
	return &sg_zhenying_truck[id % 10 - 1];
}

ChangeSpecialty *get_change_special_table(int level)
{
	uint64_t comb_id = 325000000 + level;
	std::map<uint64_t, ChangeSpecialty *>::iterator iter = change_special_config.find(comb_id);
	if (iter != change_special_config.end())
	{
		return iter->second;
	}
	return NULL;
}

RewardTable *get_chengjie_reward_table(int level)
{
	uint64_t comb_id = 327000000 + level;
	std::map<uint64_t, RewardTable *>::iterator iter = chengjie_reward_config.find(comb_id);
	if (iter != chengjie_reward_config.end())
	{
		return iter->second;
	}
	return NULL;
}

MoneyQuestTable *get_shangjin_task_table(uint32_t level)
{
	std::map<uint64_t, MoneyQuestTable *>::iterator iter = shangjin_task_config.begin();
	for (; iter != shangjin_task_config.end(); ++iter)
	{
		if (level == 1)
		{
			return iter->second;
		}
		if (level >= iter->second->LevelSection[0] && level <= iter->second->LevelSection[1])
		{
			return iter->second;
		}
	}
	return NULL;
}

CharmTable *get_charm_table(uint32_t level)
{
	uint64_t comb_id = 108000000 + level;
	return get_config_by_id(comb_id, &charm_config);
}

EquipmentTable *get_equip_config(uint32_t job, uint32_t type, uint32_t stair, uint32_t star)
{
	uint64_t comb_id = 262 * 1e6 + job * 1e5 + type * 1e3 + stair * 1e1 + star;
	std::map<uint64_t, EquipmentTable *>::iterator iter = equipment_config.find(comb_id);
	if (iter != equipment_config.end())
	{
		return iter->second;
	}

	return NULL;
}

EquipStarLv *get_equip_star_config(uint32_t job, uint32_t type, uint32_t stair, uint32_t star)
{
	EquipmentTable *config = get_equip_config(job, type, stair, star);
	if (config)
	{
		return get_config_by_id(config->StarLvID, &equip_star_config);
	}

	return NULL;
}

EquipLock *get_equip_lock_config(uint32_t job, uint32_t type)
{
	uint64_t comb_id = 263 * 1e6 + job * 1e5 + type;
	std::map<uint64_t, EquipLock *>::iterator iter = equip_lock_config.find(comb_id);
	if (iter != equip_lock_config.end())
	{
		return iter->second;
	}

	return NULL;
}

GemAttribute *get_gem_config(uint32_t item_id)
{
	ItemsConfigTable *config = get_config_by_id(item_id, &item_config);
	if (config && config->n_ParameterEffect > 0)
	{
		return get_config_by_id(config->ParameterEffect[0], &equip_gem_config);
	}

	return NULL;
}

BaguaStarTable *get_bagua_star_config(uint32_t star)
{
	uint64_t comb_id = 3102 * 1e5 + star * 1e3 + 1;
	return get_config_by_id(comb_id, &bagua_star_config);
}

PartnerLevelTable *get_partner_level_config(uint32_t level)
{
	uint64_t comb_id = 4203 * 1e5 + level;
	return get_config_by_id(comb_id, &partner_level_config);
}

SkillLvTable *get_skill_level_config(uint32_t skill_id, uint32_t level)
{
	SkillTable *main_config = get_config_by_id(skill_id, &skill_config);
	if (main_config)
	{
		return get_config_by_id(main_config->SkillLv + level - 1, &skill_lv_config);
	}

	return NULL;
}

RecruitTable *get_partner_recruit_config(uint32_t type)
{
	uint64_t comb_id = 4202 * 1e5 + type;
	return get_config_by_id(comb_id, &partner_recruit_config);
}

GangsSkillTable *get_guild_skill_config(uint32_t type, uint32_t level)
{
	uint32_t comb_id = type * 1e3 + level;
	std::map<uint32_t, GangsSkillTable*>::iterator iter = skill_config_map.find(comb_id);
	if (iter != skill_config_map.end())
	{
		return iter->second;
	}

	return NULL;
}

AchievementHierarchyTable *get_achievement_config(uint32_t achievement_id, uint32_t star)
{
	AchievementFunctionTable *func_config = get_config_by_id(achievement_id, &achievement_function_config);
	if (func_config && star < func_config->n_Hierarchys)
	{
		return get_config_by_id(func_config->Hierarchys[star], &achievement_hierarchy_config);
	}

	return NULL;
}

uint32_t get_item_relate_id(uint32_t id)
{
	std::map<uint64_t, ItemsConfigTable *>::iterator iter = item_config.find(id);
	if (iter != item_config.end())
	{
		return iter->second->ItemRelation;
	}

	return 0;
}

int get_item_bind_and_unbind_id(uint32_t id, uint32_t *bind_id, uint32_t *unbind_id)
{
	if (bind_id)
	{
		*bind_id = 0;
	}
	if (unbind_id)
	{
		*unbind_id = 0;
	}
	ItemsConfigTable *config = get_config_by_id(id, &item_config);
	if (!config)
	{
		return -1;
	}

	if (config->BindType == 0)
	{
		if (bind_id)
		{
			*bind_id = config->ItemRelation;
		}
		if (unbind_id)
		{
			*unbind_id = id;
		}
	}
	else
	{
		if (bind_id)
		{
			*bind_id = id;
		}
		if (unbind_id)
		{
			*unbind_id = config->ItemRelation;
		}
	}

	return 0;
}

uint32_t get_bag_total_num(uint32_t job, uint32_t level)
{
	uint32_t num = 0;
	ActorLevelTable *level_config = get_actor_level_config(job, level);
	if (level_config)
	{
		num = level_config->FreeGrid + level_config->LockGrid;
	}

	return num;
}

uint32_t get_item_stack_num(uint32_t id)
{
	std::map<uint64_t, ItemsConfigTable *>::iterator iter = item_config.find(id);
	if (iter != item_config.end())
	{
		if (iter->second->ItemLimit == 0)
		{
			return iter->second->Stackable;
		}
		else
		{
			return 1;
		}
	}

	return 0;
}

void add_drop_item(uint32_t item_id, uint32_t num_max, uint32_t num_min, std::map<uint32_t, uint32_t> &item_list, uint32_t stack)
{
	if (item_id > 200000000 && item_id < 209999999)
	{
		uint32_t rand_num = random() % (num_max - num_min + 1) + num_min;
		if (rand_num > 0)
		{
			item_list[item_id] += rand_num;
		}
	}
	else
	{
		get_drop_item(item_id, item_list, stack + 1);
	}
}

int get_drop_item(uint32_t drop_id, std::map<uint32_t, uint32_t> &item_list, uint32_t stack)
{
	if (stack >= 10)
	{
		LOG_ERR("[%s:%d] stack level too deep, drop_id:%u", __FUNCTION__, __LINE__, drop_id);
		return -1;
	}

	DropConfigTable *config = get_config_by_id(drop_id, &drop_config);
	if (!config)
	{
		LOG_ERR("[%s:%d] get drop config failed, drop_id:%u", __FUNCTION__, __LINE__, drop_id);
		return -1;
	}

	if (config->ProType == 0) //统一概率
	{
		uint64_t total_prob = 0;
		for (uint32_t i = 0; i < config->n_DropID && i < config->n_Probability && i < config->n_NumMin && i < config->n_NumMax; ++i)
		{
			total_prob += config->Probability[i];
		}

		if (total_prob == 0)
		{
			LOG_ERR("[%s:%d] total_probability is 0, drop_id:%u", __FUNCTION__, __LINE__, drop_id);
			return -1;
		}

		uint32_t rand_val = random() % total_prob;
		uint32_t add_val = 0;
		for (uint32_t i = 0; i < config->n_DropID && i < config->n_Probability && i < config->n_NumMin && i < config->n_NumMax; ++i)
		{
			if (add_val <= rand_val && rand_val < add_val + config->Probability[i])
			{
				add_drop_item(config->DropID[i], config->NumMax[i], config->NumMin[i], item_list, stack);
				break;
			}
			else
			{
				add_val += config->Probability[i];
			}
		}
	}
	else //单独概率
	{
		for (uint32_t i = 0; i < config->n_DropID && i < config->n_Probability && i < config->n_NumMin && i < config->n_NumMax; ++i)
		{
			uint32_t rand_val = random() % RAND_RATE_BASE;
			if (rand_val < config->Probability[i])
			{
				add_drop_item(config->DropID[i], config->NumMax[i], config->NumMin[i], item_list, stack);
			}
		}
	}

	return 0;
}

int get_player_sex(uint32_t job)
{
	switch(job)
	{
		case JOB_DEFINE_DAO:
		case JOB_DEFINE_BI:
			return SEX_DEFINE_MALE;
		case JOB_DEFINE_GONG:
		case JOB_DEFINE_QIANG:
		case JOB_DEFINE_FAZHANG:
			return SEX_DEFINE_FEMALE;
	}

	return 0;
}

int get_task_type(uint32_t task_id)
{
	TaskTable *config = get_config_by_id(task_id, &task_config);
	if (config)
	{
		return (config->TaskType);
	}

	return 0;
}

int task_is_trunk(uint32_t task_id)
{
	return (get_task_type(task_id) == TT_TRUNK);
}

int task_is_branch(uint32_t task_id)
{
	return (get_task_type(task_id) == TT_BRANCH);
}

bool item_is_bind(uint32_t item_id)
{
	std::map<uint64_t, ItemsConfigTable *>::iterator iter = item_config.find(item_id);
	if (iter != item_config.end())
	{
		return (iter->second->BindType != 0);
	}

	return false;
}

int get_transfer_point(uint32_t transfer_id, uint32_t *scene_id, double *pos_x, double *pos_y, double *pos_z, double *direct)
{
	struct TransferPointTable* t_config = get_config_by_id(transfer_id, &transfer_config);
	if (!t_config || t_config->n_MapId == 0)
	{
		LOG_ERR("[%s:%d] get transfer config failed, id:%u", __FUNCTION__, __LINE__, transfer_id);
		return (-1);
	}

	if (t_config->n_MapId >= 5)
	{
		if (scene_id)
			*scene_id = t_config->MapId[0];
		if (pos_x)
			*pos_x = (int64_t)t_config->MapId[1] / 1000.0;
		if (pos_y)
			*pos_y = (int64_t)t_config->MapId[2] / 1000.0;
		if (pos_z)
			*pos_z = (int64_t)t_config->MapId[3] / 1000.0;
		if (direct)
			*direct = (int64_t)t_config->MapId[4];
	}
	else
	{
		uint32_t SceneId = t_config->MapId[0];
		SceneResTable *scene_config = get_config_by_id(SceneId, &scene_res_config);
		if (!scene_config)
		{
			LOG_ERR("[%s:%d] get scene config failed, transfer_id:%u, scene_id:%u", __FUNCTION__, __LINE__, transfer_id, SceneId);
			return -1;
		}

		if (scene_id)
			*scene_id = SceneId;
		if (pos_x)
			*pos_x = (int64_t)scene_config->BirthPointX;
		if (pos_y)
			*pos_y = (int64_t)scene_config->BirthPointY;
		if (pos_z)
			*pos_z = (int64_t)scene_config->BirthPointZ;
		if (direct)
			*direct = (int64_t)scene_config->FaceY;
	}

	return 0;
}

bool item_is_baguapai(uint32_t item_id)
{
	ItemsConfigTable *config = get_config_by_id(item_id, &item_config);
	if (!config)
	{
		return false;
	}

	return (config->ItemType == 10);
}

int bagua_item_to_card(uint32_t item_id)
{
	ItemsConfigTable *config = get_config_by_id(item_id, &item_config);
	if (!config)
	{
		return 0;
	}

	if (config->ItemType != 10 || config->n_ParameterEffect < 1)
	{
		return 0;
	}

	return config->ParameterEffect[0];
}

int get_actor_skill_index(uint32_t job, uint32_t skill_id)
{
	do
	{
		ActorTable *config = get_actor_config(job);
		if (!config)
		{
			break;
		}

		for (uint32_t i = 0; i < config->n_Skill; ++i)
		{
			if ((uint32_t)config->Skill[i] == skill_id)
			{
				return (i + 1);
			}
		}
	} while(0);

	return 0;
}

bool item_is_partner_fabao(uint32_t item_id)
{

	ItemsConfigTable *config = get_config_by_id(item_id, &item_config);
	if (!config)
	{
		return false;
	}

	return (config->ItemType == 14);
}

uint32_t get_friend_close_level(uint32_t closeness)
{
	uint32_t lv = 0;
	uint32_t pre_val = 0;
	for (std::map<uint64_t, DegreeTable*>::iterator iter = friend_close_config.begin(); iter != friend_close_config.end(); ++iter)
	{
		DegreeTable *config = iter->second;
		if (closeness > pre_val)
		{
			lv = config->Stage;
			if (closeness <= config->Value)
			{
				break;
			}
			else
			{
				pre_val = config->Value;
			}
		}
		else
		{
			break;
		}
	}
	return lv;
}

bool activity_is_open(uint32_t activity_id)
{
	uint32_t now = time_helper::get_cached_time() / 1000;

	bool in_time = false;
	do
	{
		EventCalendarTable *act_config = get_config_by_id(activity_id, &activity_config);
		if (!act_config)
		{
			break;
		}

		ControlTable *ctrl_config = get_config_by_id(act_config->RelationID, &all_control_config);
		if (!ctrl_config)
		{
			break;
		}

		//检查时间
		if (ctrl_config->n_OpenDay > 0)
		{
			bool pass = false;
			uint32_t week = time_helper::getWeek(now);
			for (size_t i = 0; i < ctrl_config->n_OpenDay; ++i)
			{
				if (week == ctrl_config->OpenDay[i])
				{
					pass = true;
					break;
				}
			}
			if (pass == false)
			{
				break;
			}
		}
		assert(ctrl_config->n_OpenTime == ctrl_config->n_CloseTime);
		if (ctrl_config->n_OpenTime > 0)
		{
			bool pass = false;
			for (size_t i = 0; i < ctrl_config->n_OpenTime; ++i)
			{
				uint32_t start = time_helper::get_timestamp_by_day(ctrl_config->OpenTime[i] / 100,
						ctrl_config->OpenTime[i] % 100, now);
				uint32_t end = time_helper::get_timestamp_by_day(ctrl_config->CloseTime[i] / 100,
						ctrl_config->CloseTime[i] % 100, now);
				if (now >= start && now <= end)
				{
					pass = true;
					break;
				}
			}
			if (pass == false)
			{
				break;
			}
		}

		in_time = true;
		break;
	} while(0);

	return in_time;
}

bool is_raid_scene_id(uint32_t id)
{
	if (id > SCENCE_DEPART)
		return true;
	return false;
}

SCENE_TYPE_DEFINE get_scene_looks_type(uint32_t scene_id)
{
	if (scene_id <= SCENCE_DEPART)
		return SCENE_TYPE_WILD;
	struct DungeonTable *r_config = get_config_by_id(scene_id, &all_raid_config);
	if (!r_config)
		return SCENE_TYPE_RAID;
	switch (r_config->DengeonRank)
	{
		case DUNGEON_TYPE_GUILD_LAND:
		case DUNGEON_TYPE_GUILD_WAIT:
		case DUNGEON_TYPE_ZHENYING:
			return SCENE_TYPE_WILD;
		default:
			return SCENE_TYPE_RAID;			
	}
	return SCENE_TYPE_RAID;
}

int get_dungeon_type(uint32_t raid_id)
{
	DungeonTable *config = get_config_by_id(raid_id, &all_raid_config);
	if (config)
	{
		return config->DengeonRank;
	}
	
	return 0;
}

int item_id_to_trade_id(uint32_t item_id)
{
	std::map<uint32_t, uint32_t>::iterator iter = sg_item_trade_map.find(item_id);
	if (iter != sg_item_trade_map.end())
	{
		return iter->second;
	}

	return 0;
}

int trade_id_to_item_id(uint32_t trade_id)
{
	TradingTable *config = get_config_by_id(trade_id, &trade_item_config);
	if (config)
	{
		return config->ItemID;
	}
	return 0;
}

static void adjust_guild_skill_config(void)
{
	for (std::map<uint64_t, GangsSkillTable*>::iterator iter = guild_skill_config.begin(); iter != guild_skill_config.end(); ++iter)
	{
		GangsSkillTable *config = iter->second;
		uint32_t key_new = config->skillType * 1e3 + config->skillLeve;
		skill_config_map[key_new] = config;
	}
}

static void adjust_strong_config(void)
{
	sg_strong_chapter_map.clear();
	for (std::map<uint64_t, GrowupTable*>::iterator iter = strong_config.begin(); iter != strong_config.end(); ++iter)
	{
		GrowupTable *config = iter->second;
		if (sg_strong_chapter_map.find(config->Type) == sg_strong_chapter_map.end())
		{
			sg_strong_chapter_map.insert(std::make_pair(config->Type, 1));
		}
		else
		{
			sg_strong_chapter_map[config->Type]++;
		}

		if (sg_strong_chapter_reward.find(config->Type) == sg_strong_chapter_reward.end())
		{
			sg_strong_chapter_reward.insert(std::make_pair(config->Type, config));
		}
	}
}

static void adjust_trade_config(void)
{
	sg_item_trade_map.clear();
	for (std::map<uint64_t, TradingTable*>::iterator iter = trade_item_config.begin(); iter != trade_item_config.end(); ++iter)
	{
		TradingTable *config = iter->second;
		if (!item_is_bind(config->ItemID))
		{
			sg_item_trade_map.insert(std::make_pair(config->ItemID, config->ID));
		}
	}
}

static void	adjust_maogui_diaoxiang_config()
{
	std::map<uint64_t, struct MGLYdiaoxiangTable*> tmp = maogui_diaoxiang_config;
	maogui_diaoxiang_config.clear();
	for (std::map<uint64_t, struct MGLYdiaoxiangTable*>::iterator ite = tmp.begin(); ite != tmp.end(); ++ite)
	{
		struct MGLYdiaoxiangTable* t = ite->second;
		maogui_diaoxiang_config[ite->second->MonsterPointID] = t;
	}
}

static void	adjust_maogui_monster_config()
{
	std::map<uint64_t, struct MGLYmaoguiTable*> tmp = maogui_monster_config;
	maogui_monster_config.clear();
	for (std::map<uint64_t, struct MGLYmaoguiTable*>::iterator ite = tmp.begin(); ite != tmp.end(); ++ite)
	{
		struct MGLYmaoguiTable* t = ite->second;
		maogui_monster_config[ite->second->MonsterID] = t;
	}
}


static void	adjust_maogui_colour_config()
{
	std::map<uint64_t, struct MGLYyanseTable*> tmp = maogui_colour_config;
	maogui_colour_config.clear();
	for (std::map<uint64_t, struct MGLYyanseTable*>::iterator ite = tmp.begin(); ite != tmp.end(); ++ite)
	{
		struct MGLYyanseTable* t = ite->second;
		maogui_colour_config[ite->second->MonsterID] = t;
		switch(ite->second->Type)
		{
			case 1:
				maogui_zhengning_colour_config[ite->second->MonsterID] = t;
			break;
			case 2:
				maogui_shouling_colour_config[ite->second->MonsterID] = t;
			break;
			case 3:
				maogui_diaoxiang_colour_config[ite->second->MonsterID] = t;
			break;
			case 4:
				maogui_xiaoguai_colour_config[ite->second->MonsterID] = t;
			default:
			break;
			
		}
	}
}

static void	adjust_maogui_maogui_wang_config()
{
	std::map<uint64_t, struct MGLYmaoguiwangTable*> tmp = maogui_maogui_wang_config;
	maogui_maogui_wang_config.clear();
	for (std::map<uint64_t, struct MGLYmaoguiwangTable*>::iterator ite = tmp.begin(); ite != tmp.end(); ++ite)
	{
		struct MGLYmaoguiwangTable* t = ite->second;
		maogui_maogui_wang_config[ite->second->BossID] = t;
	}
}

static void	adjust_maogui_shouling_to_xiaoguai_config()
{
	std::map<uint64_t, struct MGLYshoulingTable*> tmp = maogui_shouling_to_xiaoguai_config;
	maogui_shouling_to_xiaoguai_config.clear();
	for (std::map<uint64_t, struct MGLYshoulingTable*>::iterator ite = tmp.begin(); ite != tmp.end(); ++ite)
	{
		struct MGLYshoulingTable* t = ite->second;
		maogui_shouling_to_xiaoguai_config[ite->second->BossID] = t;
	}
}

typedef std::map<uint64_t, void *> *config_type;
#define READ_SPB_MAX_LEN (1024 * 1024)
int read_all_excel_data()
{
	char *buf = (char *)malloc(READ_SPB_MAX_LEN);
	if (!buf)
		return -1;
	int fd = open("../excel_data/1.spb", O_RDONLY);
	if (fd <= 0) {
		printf("open 1.spb failed, err = %d\n", errno);
		return (-1);
	}
	size_t size =  read(fd, buf, READ_SPB_MAX_LEN);
	struct sproto *sp = sproto_create(&buf[0], size);
	close(fd);
    lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	int ret;

	struct sproto_type *type = sproto_type(sp, "ActiveSkillTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/ActiveSkillTable.lua", (config_type)&active_skill_config);
	assert(ret == 0);
	adjust_active_skill_tables();

	type = sproto_type(sp, "SkillMoveTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/SkillMoveTable.lua", (config_type)&move_skill_config);
	assert(ret == 0);

	type = sproto_type(sp, "MonsterPkTypeTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/MonsterPkTypeTable.lua", (config_type)&pk_type_config);
	assert(ret == 0);
	adjust_pktype_table();		

	type = sproto_type(sp, "NpcTalkTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/NpcTalkTable.lua", (config_type)&monster_talk_config);
	assert(ret == 0);
	adjust_monster_talk_table();		

	type = sproto_type(sp, "MonsterTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/MonsterTable.lua", (config_type)&monster_config);
	assert(ret == 0);
	adjust_monster_table();	

	type = sproto_type(sp, "ActorTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/ActorTable.lua", (config_type)&actor_config);
	assert(ret == 0);

	type = sproto_type(sp, "ActorAttributeTable");
	assert(type);	
	ret = traverse_main_table(L, type, "../lua_data/ActorAttributeTable.lua", (config_type)&actor_attribute_config);
	assert(ret == 0);

	type = sproto_type(sp, "SceneResTable");
	assert(type);	
	ret = traverse_main_table(L, type, "../lua_data/SceneResTable.lua", (config_type)&scene_res_config);
	assert(ret == 0);	
	adjust_scene_table();

	type = sproto_type(sp, "PassiveSkillTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/PassiveSkillTable.lua", (config_type)&passive_skill_config);
	assert(ret == 0);	
	
	type = sproto_type(sp, "SkillTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/SkillTable.lua", (config_type)&skill_config);
	assert(ret == 0);	
	adjust_skill_table();

	type = sproto_type(sp, "SkillLvTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/SkillLvTable.lua", (config_type)&skill_lv_config);
	assert(ret == 0);	
	adjust_lv_skill_table();
	
	type = sproto_type(sp, "FlySkillTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/FlySkillTable.lua", (config_type)&fly_skill_config);
	assert(ret == 0);	
	type = sproto_type(sp, "BuffTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/BuffTable.lua", (config_type)&buff_config);
	assert(ret == 0);	

	type = sproto_type(sp, "SkillEffectTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/SkillEffectTable.lua", (config_type)&skill_effect_config);
	assert(ret == 0);	

	type = sproto_type(sp, "ActorLevelTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/ActorLevelTable.lua", (config_type)&actor_level_config);
	assert(ret == 0);	
	
	type = sproto_type(sp, "ItemsConfigTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/ItemsConfigTable.lua", (config_type)&item_config);
	assert(ret == 0);	
	generate_item_relative_info();

	type = sproto_type(sp, "ParameterTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/ParameterTable.lua", (config_type)&parameter_config);
	assert(ret == 0);	

	type = sproto_type(sp, "DropConfigTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/DropConfigTable.lua", (config_type)&drop_config);
	assert(ret == 0);
	
	type = sproto_type(sp, "BaseAITable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/BaseAI.lua", (config_type)&base_ai_config);
	assert(ret == 0);
	adjust_baseai_table();

	type = sproto_type(sp, "ActorHeadTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/ActorHeadTable.lua", (config_type)&actor_head_config);
	assert(ret == 0);
	
	type = sproto_type(sp, "CollectTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/CollectionTable.lua", (config_type)&collect_config);
	assert(ret == 0);
	gen_show_collect();
	
	type = sproto_type(sp, "TaskTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/TaskTable.lua", (config_type)&task_config);
	assert(ret == 0);
	
	type = sproto_type(sp, "TaskConditionTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/TaskConditionTable.lua", (config_type)&task_condition_config);
	assert(ret == 0);
	
	type = sproto_type(sp, "TaskEventTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/TaskEventTable.lua", (config_type)&task_event_config);
	assert(ret == 0);
	
	type = sproto_type(sp, "TaskDropTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/TaskDropTable.lua", (config_type)&task_drop_config);
	assert(ret == 0);
	
	type = sproto_type(sp, "TaskRewardTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/TaskRewardTable.lua", (config_type)&task_reward_config);
	assert(ret == 0);
	
	type = sproto_type(sp, "TaskMonsterTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/TaskMonsterTable.lua", (config_type)&task_monster_config);
	assert(ret == 0);
	
	type = sproto_type(sp, "TaskChapterTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/TaskChapterTable.lua", (config_type)&task_chapter_config);
	assert(ret == 0);

	type = sproto_type(sp, "RandomCardTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/RandomCardTable.lua", (config_type)&wanyaoka_config);
	assert(ret == 0);
	
	type = sproto_type(sp, "DungeonTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/DungeonTable.lua", (config_type)&all_raid_config);
	assert(ret == 0);	
	type = sproto_type(sp, "RaidScriptTable");
	assert(type);
	adjust_dungeon_table(L, type);

	type = sproto_type(sp, "ControlTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/ControlTable.lua", (config_type)&all_control_config);
	assert(ret == 0);
	
	adjust_task_tables(); //要放在任务配置读取完成之后
	generate_parameters();

	type = sproto_type(sp, "EquipmentTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/EquipmentTable.lua", (config_type)&equipment_config);
	assert(ret == 0);

	type = sproto_type(sp, "EquipStarLv");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/EquipStarLv.lua", (config_type)&equip_star_config);
	assert(ret == 0);
	
	type = sproto_type(sp, "EquipLock");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/EquipLock.lua", (config_type)&equip_lock_config);
	assert(ret == 0);
	adjust_equip_lock_table();

	type = sproto_type(sp, "EquipAttribute");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/EquipAttribute.lua", (config_type)&equip_attr_config);
	assert(ret == 0);

	type = sproto_type(sp, "GemAttribute");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/GemAttribute.lua", (config_type)&equip_gem_config);
	assert(ret == 0);

	type = sproto_type(sp, "ActorFashionTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/ActorFashionTable.lua", (config_type)&fashion_config);
	assert(ret == 0);

	type = sproto_type(sp, "CharmTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/CharmTable.lua", (config_type)&charm_config);
	assert(ret == 0);

	type = sproto_type(sp, "WeaponsEffectTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/WeaponsEffectTable.lua", (config_type)&weapon_color_config);
	assert(ret == 0);

	type = sproto_type(sp, "AttributeTypeTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/AttributeTypeTable.lua", (config_type)&attribute_type_config);
	assert(ret == 0);
	adjust_fighting_capacity_coefficient();

	type = sproto_type(sp, "ColourTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/ColourTable.lua", (config_type)&color_table_config);
	assert(ret == 0);

	type = sproto_type(sp, "ShopListTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/ShopListTable.lua", (config_type)&shop_list_config);
	assert(ret == 0);

	type = sproto_type(sp, "ShopTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/ShopTable.lua", (config_type)&shop_config);
	assert(ret == 0);

	type = sproto_type(sp, "TransferPointTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/TransferPointTable.lua", (config_type)&transfer_config);
	assert(ret == 0);

	type = sproto_type(sp, "SpiritTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/SpiritTable.lua", (config_type)&spirit_config); 
	assert(ret == 0);

	type = sproto_type(sp, "MountsTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/MountsTable.lua", (config_type)&horse_config); 
	assert(ret == 0);

	type = sproto_type(sp, "CastSpiritTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/CastSpiritTable.lua", (config_type)&horse_soul_config);
	assert(ret == 0);

	type = sproto_type(sp, "PulseTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/PulseTable.lua", (config_type)&yuqidao_jingmai_config);
	assert(ret == 0);

	type = sproto_type(sp, "AcupunctureTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/AcupunctureTable.lua", (config_type)&yuqidao_acupoint_config);
	assert(ret == 0);

	type = sproto_type(sp, "BreakTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/BreakTable.lua", (config_type)&yuqidao_break_config);
	assert(ret == 0);
	adjust_yuqidao_tables();

	type = sproto_type(sp, "StageTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/StageTable.lua", (config_type)&pvp_raid_config);
	assert(ret == 0);
	adjust_stage_tables();

	type = sproto_type(sp, "BaguaTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/BaguaTable.lua", (config_type)&bagua_config);
	assert(ret == 0);

	type = sproto_type(sp, "BaguaStarTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/BaguaStarTable.lua", (config_type)&bagua_star_config);
	assert(ret == 0);

	type = sproto_type(sp, "BaguaViceAttributeTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/BaguaViceAttributeTable.lua", (config_type)&bagua_vice_attr_config);
	assert(ret == 0);

	type = sproto_type(sp, "BaguaSuitTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/BaguaSuitTable.lua", (config_type)&bagua_suit_config);
	assert(ret == 0);
	adjust_baguapai_tables();

	type = sproto_type(sp, "SpecialtyLevelTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/SpecialtyLevelTable.lua", (config_type)&specialty_level_config);
	assert(ret == 0);

	type = sproto_type(sp, "TypeLevelTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/TypeLevelTable.lua", (config_type)&guoyu_level_config);
	assert(ret == 0);

	type = sproto_type(sp, "ChangeSpecialty");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/ChangeSpecialty.lua", (config_type)&change_special_config);
	assert(ret == 0);

	type = sproto_type(sp, "BootNameTable");
	assert(type);
	ret = traverse_vector_table(L, type, "../lua_data/BootNameTable.lua", (std::vector<void *> *)&rand_name_config);	
	assert(ret == 0);

	type = sproto_type(sp, "RandomMonsterTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/RandomMonsterTable.lua", (config_type)&random_monster);
	assert(ret == 0);
	gen_random_monster_arr();

	type = sproto_type(sp, "RandomDungeonTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/RandomDungeonTable.lua", (config_type)&random_guoyu_dungenon_config);
	assert(ret == 0);
	gen_random_guoyu_fb_arr();

	type = sproto_type(sp, "RewardTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/RewardTable.lua", (config_type)&chengjie_reward_config);
	assert(ret == 0);

	type = sproto_type(sp, "SpecialTitleTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/SpecialTitleTable.lua", (config_type)&yaoshi_title_config);
	assert(ret == 0);

	type = sproto_type(sp, "MoneyQuestTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/MoneyQuestTable.lua", (config_type)&shangjin_task_config);
	assert(ret == 0);

	type = sproto_type(sp, "SpecialtySkillTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/SpecialtySkillTable.lua", (config_type)&yaoshi_skill_config);
	assert(ret == 0);
	gen_yaoshi_skill_config();

	type = sproto_type(sp, "ActorRobotTable");
	assert(type);
	std::vector<struct ActorRobotTable*> _robot_config;
	ret = traverse_vector_table(L, type, "../lua_data/ActorRobotTable.lua", (std::vector<void *> *)&_robot_config);
	assert(ret == 0);
	adjust_robot_config(&_robot_config);
	
	type = sproto_type(sp, "EventCalendarTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/EventCalendarTable.lua", (config_type)&activity_config);
	assert(ret == 0);
	
	type = sproto_type(sp, "ActiveTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/ActiveTable.lua", (config_type)&activity_activeness_config);
	assert(ret == 0);
	
	type = sproto_type(sp, "ChivalrousTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/ChivalrousTable.lua", (config_type)&activity_chivalry_config);
	assert(ret == 0);

	type = sproto_type(sp, "CampTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/CampTable.lua", (config_type)&zhenying_base_config);
	assert(ret == 0); 

	type = sproto_type(sp, "BattlefieldTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/BattlefieldTable.lua", (config_type)&zhenying_fight_config);
	assert(ret == 0);

	type = sproto_type(sp, "GradeTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/GradeTable.lua", (config_type)&zhenying_level_config);
	assert(ret == 0);

	type = sproto_type(sp, "WeekTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/WeekTable.lua", (config_type)&zhenying_week_config);
	assert(ret == 0);

	type = sproto_type(sp, "LifeSkillTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/LifeSkillTable.lua", (config_type)&medicine_config);
	assert(ret == 0);

	type = sproto_type(sp, "NoticeTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/NoticeTable.lua", (config_type)&notify_config);
	assert(ret == 0);

	type = sproto_type(sp, "SearchTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/SearchTable.lua", (config_type)&xunbao_config);
	assert(ret == 0);
	adjust_xunbao_table();

	type = sproto_type(sp, "TreasureTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/TreasureTable.lua", (config_type)&xunbao_map_config);
	assert(ret == 0);
	adjust_xunbao_map_table();

	type = sproto_type(sp, "QuestionTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/QuestionTable.lua", (config_type)&questions_config);
	assert(ret == 0);
	gen_question_arr();

	type = sproto_type(sp, "EscortTask");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/EscortTask.lua", (config_type)&escort_config);
	assert(ret == 0);

	type = sproto_type(sp, "PartnerTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/PartnerTable.lua", (config_type)&partner_config);
	assert(ret == 0);

	type = sproto_type(sp, "GodYaoAttributeTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/GodYaoAttributeTable.lua", (config_type)&partner_god_attr_config);
	assert(ret == 0);

	type = sproto_type(sp, "RecruitTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/RecruitTable.lua", (config_type)&partner_recruit_config);
	assert(ret == 0);

	type = sproto_type(sp, "PartnerLevelTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/PartnerLevelTable.lua", (config_type)&partner_level_config);
	assert(ret == 0);

	type = sproto_type(sp, "FetterTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/FetterTable.lua", (config_type)&partner_bond_config);
	assert(ret == 0);

	type = sproto_type(sp, "BiaocheTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/BiaocheTable.lua", (config_type)&cash_truck_config);
	assert(ret == 0);

	type = sproto_type(sp, "BiaocheRewardTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/BiaocheRewardTable.lua", (config_type)&cash_truck_reward_config);
	assert(ret == 0);

	type = sproto_type(sp, "RobotPatrolTable");
	assert(type);
	ret = traverse_vector_table(L, type, "../lua_data/RobotPatrolTable.lua", (std::vector<void *> *)&robot_patrol_config);	
	assert(ret == 0);
	adjust_robotpatrol_table();
	ret = traverse_vector_table(L, type, "../lua_data/RobotBattlrTable.lua", (std::vector<void *> *)&robot_zhenyingzhan_config);	
	assert(ret == 0);
	adjust_robotzhenyingzhan_table();

	type = sproto_type(sp, "FactionBattleTable");
	assert(type);
	ret = traverse_vector_table(L, type, "../lua_data/FactionBattleTable.lua", (std::vector<void *> *)&zhenying_battle_config);
	assert(ret == 0);

	type = sproto_type(sp, "FunctionUnlockTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/FunctionUnlockTable.lua", (config_type)&function_unlock_config);
	assert(ret == 0);
	adjust_jijiangopen_table();

	type = sproto_type(sp, "LifeMagicTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/LifeMagicTable.lua", (config_type)&lifemagic_config);
	assert(ret == 0);
	
	type = sproto_type(sp, "MagicTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/MagicTable.lua", (config_type)&MagicTable_config);
	assert(ret == 0);
	
	type = sproto_type(sp, "MagicAttributeTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/MagicAttributeTable.lua", (config_type)&MagicAttrbute_config);
	assert(ret == 0);

	type = sproto_type(sp, "ArenaRewardTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/ArenaRewardTable.lua", (config_type)&doufachang_reward_config);
	assert(ret == 0);

	type = sproto_type(sp, "SceneCreateMonsterTable");
	assert(type);
	if (generate_create_monster_config(L, type) != 0)
	{
		printf("generate create monster config fail\n");
		return (-1);
	}
	
	type = sproto_type(sp, "GenerateMonster");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/GenerateMonster.lua", (config_type)&GenerateMonster_config);
	assert(ret == 0);
	adjust_generatemonster_config();

	type = sproto_type(sp, "ServerResTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/ServerResTable.lua", (config_type)&server_res_config);
	assert(ret == 0);

	type = sproto_type(sp, "ServerLevelTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/ServerLevelTable.lua", (config_type)&server_level_config);
	assert(ret == 0);

	type = sproto_type(sp, "AchievementFunctionTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/AchievementFunctionTable.lua", (config_type)&achievement_function_config);
	assert(ret == 0);

	type = sproto_type(sp, "AchievementHierarchyTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/AchievementHierarchyTable.lua", (config_type)&achievement_hierarchy_config);
	assert(ret == 0);

	type = sproto_type(sp, "GangsSkillTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/GangsSkillTable.lua", (config_type)&guild_skill_config);
	assert(ret == 0);	

	type = sproto_type(sp, "DegreeTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/DegreeTable.lua", (config_type)&friend_close_config);
	assert(ret == 0);	

	type = sproto_type(sp, "TitleFunctionTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/TitleFunctionTable.lua", (config_type)&title_function_config);
	assert(ret == 0);	

	type = sproto_type(sp, "WorldBossTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/WorldBossTable.lua", (config_type)&world_boss_config);
	assert(ret == 0);	
	adjust_worldboss_table();

	type = sproto_type(sp, "ChallengeTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/ChallengeTable.lua", (config_type)&hero_challenge_config);
	assert(ret == 0);	
	adjust_herochallenge_table();

	type = sproto_type(sp, "UndergroundTask");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/UndergroundTask.lua", (config_type)&mijing_xiulian_config);
	assert(ret == 0);	
	adjust_mijingxiulian_table();

	type = sproto_type(sp, "CampDefenseTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/CampDefenseTable.lua", (config_type)&zhenying_daily_config);
	assert(ret == 0);
	adjust_zhenying_daily_truck();

	type = sproto_type(sp, "FishingTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/FishingTable.lua", (config_type)&fishing_config);
	assert(ret == 0);

	type = sproto_type(sp, "GrowupTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/GrowupTable.lua", (config_type)&strong_config);
	assert(ret == 0);

	type = sproto_type(sp, "FactionActivity");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/FactionActivity.lua", (config_type)&guild_activ_config);
	assert(ret == 0);	
	type = sproto_type(sp, "RaidScriptTable");
	assert(type);
	add_guild_raid_ai_config(L, type);
	type = sproto_type(sp, "SceneCreateMonsterTable");
	assert(type);
	add_raid_ai_monster_config(L, type);

	type = sproto_type(sp, "TradingTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/TradingTable.lua", (config_type)&trade_item_config);
	assert(ret == 0);	

	type = sproto_type(sp, "AuctionTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/AuctionTable.lua", (config_type)&auction_config);
	assert(ret == 0);

	type = sproto_type(sp, "MGLYdiaoxiangTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/MGLYdiaoxiangTable.lua", (config_type)&maogui_diaoxiang_config);
	assert(ret == 0);

	type = sproto_type(sp, "MGLYmaoguiTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/MGLYmaoguiTable.lua", (config_type)&maogui_monster_config);
	assert(ret == 0);

	type = sproto_type(sp, "MGLYyanseTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/MGLYyanseTable.lua", (config_type)&maogui_colour_config);
	assert(ret == 0);

	type = sproto_type(sp, "MGLYmaoguiwangTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/MGLYmaoguiwangTable.lua", (config_type)&maogui_maogui_wang_config);
	assert(ret == 0);

	type = sproto_type(sp, "MGLYshoulingTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/MGLYshoulingTable.lua", (config_type)&maogui_shouling_to_xiaoguai_config);
	assert(ret == 0);

	adjust_escort_config();
	adjust_achievement_config();
	adjust_guild_skill_config();
	adjust_strong_config();
	adjust_trade_config();
	adjust_maogui_diaoxiang_config();
	adjust_maogui_monster_config();
	adjust_maogui_colour_config();
	adjust_maogui_maogui_wang_config();
	adjust_maogui_shouling_to_xiaoguai_config();

	lua_close(L);
	sproto_release(sp);
	free(buf);
	return (0);
}

int reload_config()
{
	char *buf = (char *)malloc(READ_SPB_MAX_LEN);
	if (!buf)
		return -1;
	int fd = open("../excel_data/1.spb", O_RDONLY);
	if (fd <= 0) {
		printf("open 1.spb failed, err = %d\n", errno);
		return (-1);
	}
	size_t size =  read(fd, buf, READ_SPB_MAX_LEN);
	struct sproto *sp = sproto_create(&buf[0], size);
	close(fd);
    lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	int ret;
	struct sproto_type *type;

		// TODO: 需要reload，并且可以reload的配置再往这里加

	type = sproto_type(sp, "ParameterTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/ParameterTable.lua", (config_type)&parameter_config);
	assert(ret == 0);	
	
	generate_parameters();

	lua_close(L);
	sproto_release(sp);
	free(buf);
	return (0);
}

