#include "guild_config.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
};
#include "time_helper.h"
#include "sproto.h"
#include "sprotoc_common.h"
#include "game_event.h"
#include "lua_load.h"

uint32_t sg_guild_create_level = 0;
uint32_t sg_guild_create_gold = 0;
uint32_t sg_guild_rename_item_id = 0;
uint32_t sg_guild_rename_item_num = 0;
uint32_t sg_guild_join_cd = 0;
uint32_t sg_guild_apply_join_cd = 0;
uint32_t sg_guild_rename_cd = 0;
uint32_t sg_guild_invite_cd = 0;
char* sg_guild_recruit_notice = NULL;
char* sg_guild_announcement = NULL;
std::vector<uint32_t> sg_guild_question;

uint32_t sg_guild_init_popularity = 0;
uint32_t sg_guild_donate_popularity[3];
uint32_t sg_guild_task_popularity = 0;
uint32_t sg_guild_battle_preliminary_popularity[4];
uint32_t sg_guild_battle_final_popularity[5];

std::map<uint32_t, GangsTable*> building_config_map;
std::map<uint32_t, GangsSkillTable*> skill_config_map;
std::map<uint64_t, DonationTable*> donate_config_map;

std::map<uint64_t, struct QuestionTable*> questions_config; //考题表
std::map<uint64_t, struct ParameterTable *> parameter_config;
std::map<uint64_t, struct GangsTable*> guild_building_config; //帮会建筑表
std::map<uint64_t, struct GangsJurisdictionTable*> guild_office_config; //帮会职权表
std::map<uint64_t, struct GangsSkillTable*> guild_skill_config; //帮会技能表
std::map<uint64_t, struct ShopTable*> shop_config; //商品配置
std::map<uint64_t, struct GangsDungeonTable*> guild_battle_reward_config; //帮会战奖励表
std::map<uint64_t, struct EventCalendarTable*> activity_config; //活动配置
std::map<uint64_t, struct ControlTable*> all_control_config; //副本进入条件收益次数配置
std::map<uint64_t, struct ActorLevelTable *> actor_level_config; //角色等级配置
std::map<uint64_t, struct FactionActivity *> guild_land_active_config; //帮会领地活动表
std::map<uint64_t, struct GangsBuildTaskTable*> guild_build_task_config; //帮会建设任务表
std::map<uint64_t, struct DonationTable*> guild_donate_config; //帮会捐献表

static void gen_question_arr()
{
	std::map<uint64_t, struct QuestionTable*>::iterator it = questions_config.begin();
	for (; it != questions_config.end(); ++it)
	{
		if (it->second->Type == 3)
		{
			sg_guild_question.push_back(it->first);
		}
	}
}

static void generate_parameters(void)
{
	ParameterTable *config = NULL;
	ParameterTable *guild_create_level_param = get_config_by_id(161000116, &parameter_config);
	if (guild_create_level_param && guild_create_level_param->n_parameter1 >= 1)
	{
		sg_guild_create_level = guild_create_level_param->parameter1[0];
	}
	ParameterTable *guild_create_coin_param = get_config_by_id(161000117, &parameter_config);
	if (guild_create_coin_param && guild_create_coin_param->n_parameter1 >= 1)
	{
		sg_guild_create_gold = guild_create_coin_param->parameter1[0];
	}
	ParameterTable *guild_rename_item_param = get_config_by_id(161000118, &parameter_config);
	if (guild_rename_item_param && guild_rename_item_param->n_parameter1 >= 2)
	{
		sg_guild_rename_item_id = guild_rename_item_param->parameter1[0];
		sg_guild_rename_item_num = guild_rename_item_param->parameter1[1];
	}
	ParameterTable *guild_join_cd_param = get_config_by_id(161000119, &parameter_config);
	if (guild_join_cd_param && guild_join_cd_param->n_parameter1 >= 3)
	{
		sg_guild_join_cd = guild_join_cd_param->parameter1[0];
		sg_guild_apply_join_cd = guild_join_cd_param->parameter1[1];
		sg_guild_rename_cd = guild_join_cd_param->parameter1[2];
	}
	ParameterTable *guild_recruit_notice_param = get_config_by_id(161000166, &parameter_config);
	if (guild_recruit_notice_param && guild_recruit_notice_param->parameter2)
	{
		sg_guild_recruit_notice = guild_recruit_notice_param->parameter2;
	}
	ParameterTable *guild_announcement_param = get_config_by_id(161000167, &parameter_config);
	if (guild_announcement_param && guild_announcement_param->parameter2)
	{
		sg_guild_announcement = guild_announcement_param->parameter2;
	}
	config = get_config_by_id(161000389, &parameter_config);
	if (config && config->n_parameter1 >= 1)
	{
		sg_guild_invite_cd = config->parameter1[0];
	}
	config = get_config_by_id(161000424, &parameter_config);
	if (config && config->n_parameter1 >= 1)
	{
		sg_guild_init_popularity = config->parameter1[0];
	}
	config = get_config_by_id(161000426, &parameter_config);
	if (config && config->n_parameter1 >= 3)
	{
		for (int i = 0; i < 3; ++i)
		{
			sg_guild_donate_popularity[i] = config->parameter1[i];
		}
	}
	config = get_config_by_id(161000428, &parameter_config);
	if (config && config->n_parameter1 >= 1)
	{
		sg_guild_task_popularity = config->parameter1[0];
	}
	config = get_config_by_id(161000431, &parameter_config);
	if (config && config->n_parameter1 >= 4)
	{
		for (int i = 0; i < 4; ++i)
		{
			sg_guild_battle_preliminary_popularity[i] = config->parameter1[i];
		}
	}
	config = get_config_by_id(161000432, &parameter_config);
	if (config && config->n_parameter1 >= 5)
	{
		for (int i = 0; i < 5; ++i)
		{
			sg_guild_battle_final_popularity[i] = config->parameter1[i];
		}
	}
}

static void adjust_guild_buiding_config(void)
{
	for (std::map<uint64_t, GangsTable*>::iterator iter = guild_building_config.begin(); iter != guild_building_config.end(); ++iter)
	{
		GangsTable *config = iter->second;
		uint32_t key_new = config->BuildingType * 1e3 + config->BuildingLeve;
		building_config_map[key_new] = config;
	}
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

static void adjust_guild_donate_config(void)
{
	donate_config_map.clear();
	for (std::map<uint64_t, DonationTable*>::iterator iter = guild_donate_config.begin(); iter != guild_donate_config.end(); ++iter)
	{
		DonationTable *config = iter->second;
		donate_config_map[config->Type] = config;
	}
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
	time_t tmp = times /1000 / 1000;
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



typedef std::map<uint64_t, void *> *config_type;
#define READ_SPB_MAX_LEN (1024 * 1024)
int read_all_excel_data()
{
	char *buf = (char *)malloc(READ_SPB_MAX_LEN);
	if (!buf)
		return -1;
	int fd = open("../excel_data/1.spb", O_RDONLY);
	if (fd <= 0) {
		LOG_ERR("open 1.spb failed, err = %d", errno);
		return (-1);
	}
	size_t size =  read(fd, buf, READ_SPB_MAX_LEN);
	struct sproto *sp = sproto_create(&buf[0], size);
	close(fd);
    lua_State *L = luaL_newstate();
	luaL_openlibs(L);

	int ret;
	struct sproto_type *type = NULL;
	type = sproto_type(sp, "ParameterTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/ParameterTable.lua", (config_type)&parameter_config);
	assert(ret == 0);
	generate_parameters();

	type = sproto_type(sp, "GangsTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/GangsTable.lua", (config_type)&guild_building_config);
	assert(ret == 0);	
	adjust_guild_buiding_config();

	type = sproto_type(sp, "GangsJurisdictionTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/GangsJurisdictionTable.lua", (config_type)&guild_office_config);
	assert(ret == 0);	

	type = sproto_type(sp, "GangsSkillTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/GangsSkillTable.lua", (config_type)&guild_skill_config);
	assert(ret == 0);	
	adjust_guild_skill_config();

	type = sproto_type(sp, "ShopTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/ShopTable.lua", (config_type)&shop_config);
	assert(ret == 0);	

	type = sproto_type(sp, "QuestionTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/QuestionTable.lua", (config_type)&questions_config);
	assert(ret == 0);
	gen_question_arr();

	type = sproto_type(sp, "GangsDungeonTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/GangsDungeonTable.lua", (config_type)&guild_battle_reward_config);
	assert(ret == 0);

	type = sproto_type(sp, "EventCalendarTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/EventCalendarTable.lua", (config_type)&activity_config);
	assert(ret == 0);

	type = sproto_type(sp, "ControlTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/ControlTable.lua", (config_type)&all_control_config);
	assert(ret == 0);

	type = sproto_type(sp, "ActorLevelTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/ActorLevelTable.lua", (config_type)&actor_level_config);
	assert(ret == 0);	

	type = sproto_type(sp, "FactionActivity");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/FactionActivity.lua", (config_type)&guild_land_active_config);
	assert(ret == 0);	

	type = sproto_type(sp, "GangsBuildTaskTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/GangsBuildTaskTable.lua", (config_type)&guild_build_task_config);
	assert(ret == 0);	

	type = sproto_type(sp, "DonationTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/DonationTable.lua", (config_type)&guild_donate_config);
	assert(ret == 0);	

	adjust_guild_donate_config();

	lua_close(L);	
	free(buf);

	return (0);
}

GangsTable *get_guild_building_config(uint32_t type, uint32_t level)
{
	uint32_t comb_id = type * 1e3 + level;
	std::map<uint32_t, GangsTable*>::iterator iter = building_config_map.find(comb_id);
	if (iter != building_config_map.end())
	{
		return iter->second;
	}

	return NULL;
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

DonationTable *get_guild_donate_config(uint32_t type)
{
	return get_config_by_id(type, &donate_config_map);
}

int get_guild_build_task_id(uint32_t player_lv)
{
	for (std::map<uint64_t, GangsBuildTaskTable*>::iterator iter = guild_build_task_config.begin(); iter != guild_build_task_config.end(); ++iter)
	{
		GangsBuildTaskTable *config = iter->second;
		if (config->n_Level >= 2 && player_lv >= config->Level[0] && player_lv <= config->Level[1])
		{
			return config->ID;
		}
	}

	return 0;
}

int get_guild_build_task_amount(uint32_t id)
{
	GangsBuildTaskTable *config = get_config_by_id(id, &guild_build_task_config);
	if (config)
	{
		return config->Times;
	}
	return 0;
}

