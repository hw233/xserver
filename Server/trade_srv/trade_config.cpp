#include "trade_config.h"
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
#include "sproto.h"
#include "sprotoc_common.h"
#include "game_event.h"
#include "lua_load.h"

std::map<uint64_t, struct ParameterTable *> parameter_config;
std::map<uint64_t, struct ItemsConfigTable *> items_config;
std::map<uint64_t, struct TradingTable*> trade_item_config; //交易物品表
std::map<uint64_t, struct AuctionTable*> auction_config; //拍卖品表

double sg_on_shelf_fee_percent;
double sg_trade_tax_percent;
uint32_t sg_audit_time[2];
uint32_t sg_keep_time;
uint32_t sg_shelf_init_num;
uint32_t sg_shelf_max_num;
uint32_t sg_shelf_enlarge_price;


static void generate_parameters(void)
{
	ParameterTable *config = NULL;
	config = get_config_by_id(161000342, &parameter_config);
	if (config && config->n_parameter1 >= 1)
	{
		sg_on_shelf_fee_percent = config->parameter1[0];
	}
	config = get_config_by_id(161000343, &parameter_config);
	if (config && config->n_parameter1 >= 1)
	{
		sg_trade_tax_percent = config->parameter1[0];
	}
	config = get_config_by_id(161000344, &parameter_config);
	if (config && config->n_parameter1 >= 2)
	{
		sg_audit_time[0] = config->parameter1[0];
		sg_audit_time[1] = config->parameter1[1];
	}
	config = get_config_by_id(161000345, &parameter_config);
	if (config && config->n_parameter1 >= 1)
	{
		sg_keep_time = config->parameter1[0];
	}
	config = get_config_by_id(161000346, &parameter_config);
	if (config && config->n_parameter1 >= 2)
	{
		sg_shelf_init_num = config->parameter1[0];
		sg_shelf_max_num = config->parameter1[1];
	}
	config = get_config_by_id(161000347, &parameter_config);
	if (config && config->n_parameter1 >= 1)
	{
		sg_shelf_enlarge_price = config->parameter1[0];
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

	type = sproto_type(sp, "ItemsConfigTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/ItemsConfigTable.lua", (config_type)&items_config);
	assert(ret == 0);

	type = sproto_type(sp, "TradingTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/TradingTable.lua", (config_type)&trade_item_config);
	assert(ret == 0);

	type = sproto_type(sp, "AuctionTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/AuctionTable.lua", (config_type)&auction_config);
	assert(ret == 0);


	lua_close(L);	
	sproto_release(sp);
	free(buf);

	return (0);
}

int get_item_config_type(uint32_t item_id)
{
	ItemsConfigTable *config = get_config_by_id(item_id, &items_config);
	if (config)
	{
		return config->ItemType;
	}

	return 0;
}


