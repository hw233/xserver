#include "login_config.h"
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

std::map<uint64_t, struct SceneResTable *> scene_res_config; //阻挡，寻路数据
std::map<uint64_t, struct ActorFashionTable *> fashion_config; //时装配置
std::map<uint64_t, struct ParameterTable *> parameter_config; //参数配置
std::map<uint64_t, struct ActorTable *> actor_config;
std::map<uint64_t, struct ServerResTable *> server_res_config; //服务器配置表

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
	type = sproto_type(sp, "SceneResTable");
	assert(type);	
	ret = traverse_main_table(L, type, "../lua_data/SceneResTable.lua", (config_type)&scene_res_config);
	assert(ret == 0);	

	type = sproto_type(sp, "ActorTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/ActorTable.lua", (config_type)&actor_config);
	assert(ret == 0);
	
	type = sproto_type(sp, "ActorFashionTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/ActorFashionTable.lua", (config_type)&fashion_config);
	assert(ret == 0);
	
	type = sproto_type(sp, "ParameterTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/ParameterTable.lua", (config_type)&parameter_config);
	assert(ret == 0);

	type = sproto_type(sp, "ServerResTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/ServerResTable.lua", (config_type)&server_res_config);
	assert(ret == 0);
	
	lua_close(L);	
	free(buf);
	return (0);
}






