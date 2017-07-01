#include "guild_config.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
extern "C"
{
#include "lua5.2/lua.h"
#include "lua5.2/lualib.h"
#include "lua5.2/lauxlib.h"
};
#include "sproto.h"
#include "sprotoc_common.h"
#include "game_event.h"
#include "lua_load.h"

uint32_t sg_guild_create_level = 0;
uint32_t sg_guild_create_coin = 0;
uint32_t sg_guild_rename_item_id = 0;
uint32_t sg_guild_rename_item_num = 0;
uint32_t sg_guild_join_cd = 0;
uint32_t sg_guild_apply_join_cd = 0;
uint32_t sg_guild_rename_cd = 0;
char* sg_guild_recruit_notice = NULL;
char* sg_guild_announcement = NULL;
std::vector<uint32_t> sg_guild_question;

std::map<uint32_t, GangsTable*> building_config_map;
std::map<uint32_t, GangsSkillTable*> skill_config_map;

std::map<uint64_t, struct QuestionTable*> questions_config; //考题表
std::map<uint64_t, struct ParameterTable *> parameter_config;
std::map<uint64_t, struct GangsTable*> guild_building_config; //帮会建筑表
std::map<uint64_t, struct GangsJurisdictionTable*> guild_office_config; //帮会职权表
std::map<uint64_t, struct GangsSkillTable*> guild_skill_config; //帮会技能表
std::map<uint64_t, struct ShopTable*> shop_config; //商品配置
std::map<uint64_t, struct GangsDungeonTable*> guild_battle_reward_config; //帮会战奖励表
std::map<uint64_t, struct EventCalendarTable*> activity_config; //活动配置
std::map<uint64_t, struct ControlTable*> all_control_config; //副本进入条件收益次数配置

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
	ParameterTable *guild_create_level_param = get_config_by_id(161000116, &parameter_config);
	if (guild_create_level_param && guild_create_level_param->n_parameter1 >= 1)
	{
		sg_guild_create_level = guild_create_level_param->parameter1[0];
	}
	ParameterTable *guild_create_coin_param = get_config_by_id(161000117, &parameter_config);
	if (guild_create_coin_param && guild_create_coin_param->n_parameter1 >= 1)
	{
		sg_guild_create_coin = guild_create_coin_param->parameter1[0];
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

