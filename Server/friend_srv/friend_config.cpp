#include "friend_config.h"
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

uint32_t sg_friend_recent_num = 0;
uint32_t sg_friend_contact_num = 0;
uint32_t sg_friend_contact_extend_num = 0;
uint32_t sg_friend_block_num = 0;
uint32_t sg_friend_enemy_num = 0;
uint32_t sg_friend_apply_num = 0;
uint32_t sg_friend_gift_send_num = 0;
uint32_t sg_friend_gift_accept_num = 0;
uint32_t sg_friend_group_num = 0;
std::set<uint32_t> sg_friend_gift_id;
uint32_t sg_friend_track_item_id;
uint32_t sg_friend_track_item_num;
uint32_t sg_friend_track_time;
uint32_t  MAX_TOWER_LEVEL;

std::map<uint64_t, struct RandomCardRewardTable*> wanyaoka_reward_config; //万妖卡奖励配置
std::map<uint64_t, struct ParameterTable *> parameter_config;
std::map<uint64_t, struct CampTable*> zhenying_base_config; //阵营基础信息表
std::map<uint64_t, struct GiftTable*> friend_gift_config; //好友礼物表
std::map<uint64_t, struct P20076Table*> tower_level_config; //冲塔表

static void generate_parameters(void)
{
	sg_friend_gift_id.clear();
	ParameterTable *config = NULL;
	{
		ParameterTable *gift_id_param = get_config_by_id(161000142, &parameter_config);
		if (gift_id_param)
		{
			for (uint32_t i = 0; i < gift_id_param->n_parameter1; ++i)
			{
				sg_friend_gift_id.insert(gift_id_param->parameter1[i]);
			}
		}
	}
	{
		ParameterTable *recent_num_param = get_config_by_id(161000150, &parameter_config);
		if (recent_num_param && recent_num_param->n_parameter1 >= 1)
		{
			sg_friend_recent_num = recent_num_param->parameter1[0];
		}
	}
	{
		ParameterTable *gift_send_num_param = get_config_by_id(161000151, &parameter_config);
		if (gift_send_num_param && gift_send_num_param->n_parameter1 >= 1)
		{
			sg_friend_gift_send_num = gift_send_num_param->parameter1[0];
		}
	}
	{
		ParameterTable *gift_accept_num_param = get_config_by_id(161000152, &parameter_config);
		if (gift_accept_num_param && gift_accept_num_param->n_parameter1 >= 1)
		{
			sg_friend_gift_accept_num = gift_accept_num_param->parameter1[0];
		}
	}
	{
		ParameterTable *contact_init_param = get_config_by_id(161000153, &parameter_config);
		if (contact_init_param && contact_init_param->n_parameter1 >= 2)
		{
			sg_friend_contact_num = contact_init_param->parameter1[0];
			sg_friend_contact_extend_num = contact_init_param->parameter1[1];
		}
	}
	{
		ParameterTable *block_num_param = get_config_by_id(161000156, &parameter_config);
		if (block_num_param && block_num_param->n_parameter1 >= 1)
		{
			sg_friend_block_num = block_num_param->parameter1[0];
		}
	}
	{
		ParameterTable *group_num_param = get_config_by_id(161000157, &parameter_config);
		if (group_num_param && group_num_param->n_parameter1 >= 1)
		{
			sg_friend_group_num = group_num_param->parameter1[0];
		}
	}
	{
		ParameterTable *enemy_num_param = get_config_by_id(161000158, &parameter_config);
		if (enemy_num_param && enemy_num_param->n_parameter1 >= 1)
		{
			sg_friend_enemy_num = enemy_num_param->parameter1[0];
		}
	}
	{
		ParameterTable *apply_num_param = get_config_by_id(161000161, &parameter_config);
		if (apply_num_param && apply_num_param->n_parameter1 >= 1)
		{
			sg_friend_apply_num = apply_num_param->parameter1[0];
		}
	}
	config = get_config_by_id(161001020, &parameter_config);
	if (config && config->n_parameter1 >= 3)
	{
		sg_friend_track_item_id = config->parameter1[0];
		sg_friend_track_item_num = config->parameter1[1];
		sg_friend_track_time = config->parameter1[2];
	}

	MAX_TOWER_LEVEL = tower_level_config.size();
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
	struct sproto_type *type = sproto_type(sp, "RandomCardRewardTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/RandomCardRewardTable.lua", (config_type)&wanyaoka_reward_config);
	assert(ret == 0);

	type = sproto_type(sp, "P20076Table");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/P20076Table.lua", (config_type)&tower_level_config);
	assert(ret == 0);

	type = sproto_type(sp, "ParameterTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/ParameterTable.lua", (config_type)&parameter_config);
	assert(ret == 0);
	generate_parameters();

	type = sproto_type(sp, "CampTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/CampTable.lua", (config_type)&zhenying_base_config);
	assert(ret == 0);

	type = sproto_type(sp, "GiftTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/GiftTable.lua", (config_type)&friend_gift_config);
	assert(ret == 0);

	lua_close(L);	
	free(buf);
	return (0);
}



