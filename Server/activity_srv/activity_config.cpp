#include "activity_config.h"
#include "sproto.h"
#include "sprotoc_common.h"
#include "lua_load.h"
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <fcntl.h>
extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include "excel_data.h"
#include "activity_util.h"

typedef std::map<uint64_t, void *> *config_type;
std::map<uint64_t, struct WorldBossTable*> rank_world_boss_config; //世界boss表
std::map<uint64_t, struct ActorAttributeTable *> actor_attribute_config;
std::map<uint64_t, struct MonsterTable *> monster_config;
std::map<uint64_t, struct ParameterTable *> parameter_config;
std::map<uint64_t, struct ServerResTable *> server_res_config; //服务器资源配置
std::map<uint64_t, struct LimitActivityControlTable *> time_limit_control_config; //限时活动表
std::map<uint64_t, struct PowerMasterTable *> zhanlidaren_config; //战力达人活动表
std::map<uint64_t, std::vector<struct PowerMasterTable *> > zhanlidaren_batch_config; //战力达人批次表
std::map<uint64_t, struct Top10GangsTable *> shidamenzong_config; //十大门宗活动表
std::map<uint64_t, std::vector<struct Top10GangsTable *> > shidamenzong_batch_config; //十大门宗批次表

static void adjust_zhanlidaren_config(void)
{
	for (std::map<uint64_t, std::vector<struct PowerMasterTable *> >::iterator iter = zhanlidaren_batch_config.begin(); iter != zhanlidaren_batch_config.end(); ++iter)
	{
		iter->second.clear();
	}
	zhanlidaren_batch_config.clear();
	for (std::map<uint64_t, struct PowerMasterTable *>::iterator iter = zhanlidaren_config.begin(); iter != zhanlidaren_config.end(); ++iter)
	{
		PowerMasterTable *config = iter->second;
		assert(config->n_Reward == config->n_RewardNum);
		zhanlidaren_batch_config[config->Batch].push_back(config);
	}
}

static void adjust_shidamenzong_config(void)
{
	for (std::map<uint64_t, std::vector<struct Top10GangsTable *> >::iterator iter = shidamenzong_batch_config.begin(); iter != shidamenzong_batch_config.end(); ++iter)
	{
		iter->second.clear();
	}
	shidamenzong_batch_config.clear();
	for (std::map<uint64_t, struct Top10GangsTable *>::iterator iter = shidamenzong_config.begin(); iter != shidamenzong_config.end(); ++iter)
	{
		Top10GangsTable *config = iter->second;
		shidamenzong_batch_config[config->Batch].push_back(config);
	}
}

#define READ_SPB_MAX_LEN (1024 * 1024)
int read_all_activity_excel_data()
{
	char *buf = (char *)malloc(READ_SPB_MAX_LEN);
	if (!buf)
		return -1;
	int fd = open("../excel_data/1.spb", O_RDONLY);
	if (fd <= 0) {
		return (-1);
	}
	size_t size =  read(fd, buf, READ_SPB_MAX_LEN);
	struct sproto *sp = sproto_create(&buf[0], size);
	close(fd);
    lua_State *L = luaL_newstate();
	luaL_openlibs(L);

	int ret;
	struct sproto_type *type = sproto_type(sp, "WorldBossTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/WorldBossTable.lua", (config_type)&rank_world_boss_config);
	assert(ret == 0);

	type = sproto_type(sp, "ActorAttributeTable");
	assert(type);	
	ret = traverse_main_table(L, type, "../lua_data/ActorAttributeTable.lua", (config_type)&actor_attribute_config);
	assert(ret == 0);

	type = sproto_type(sp, "MonsterTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/MonsterTable.lua", (config_type)&monster_config);
	assert(ret == 0);

	type = sproto_type(sp, "ParameterTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/ParameterTable.lua", (config_type)&parameter_config);
	assert(ret == 0);

	type = sproto_type(sp, "ServerResTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/ServerResTable.lua", (config_type)&server_res_config);
	assert(ret == 0);

	type = sproto_type(sp, "LimitActivityControlTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/LimitActivityControlTable.lua", (config_type)&time_limit_control_config);
	assert(ret == 0);

	type = sproto_type(sp, "PowerMasterTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/PowerMasterTable.lua", (config_type)&zhanlidaren_config);
	assert(ret == 0);

	type = sproto_type(sp, "Top10GangsTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/Top10GangsTable.lua", (config_type)&shidamenzong_config);
	assert(ret == 0);

	adjust_zhanlidaren_config();
	adjust_shidamenzong_config();

	lua_close(L);	
	sproto_release(sp);
	free(buf);
	return (0);
}

PowerMasterTable *get_zhanlidaren_config(uint32_t activity_id, uint32_t gift_id)
{
	do
	{
		LimitActivityControlTable *control_config = get_config_by_id(activity_id, &time_limit_control_config);
		if (!control_config)
		{
			break;
		}
		if (control_config->Activity != ACTIVITY_ZHANLIDAREN)
		{
			break;
		}

		PowerMasterTable *zhanlidaren_tab = get_config_by_id(gift_id, &zhanlidaren_config);
		if (!zhanlidaren_tab)
		{
			break;
		}
		if (zhanlidaren_tab->Batch != control_config->Batch)
		{
			break;
		}

		return zhanlidaren_tab;
	} while(0);
	return NULL;
}


